#include "motion_apk_runtime.h"
#include "egl_probe_context.h"
#include <GLES2/gl2.h>
#include <future>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <zlib.h>
#include <set>
struct SharedFrame {StudyMotionFrame frame{};std::vector<uint8_t> pixels;std::string error;};
int main(int argc,char **argv){
    if(argc!=4&&argc!=5){std::cerr<<"Usage: motion_runtime_gles_probe haz_a.psb seed output.pam [parent_es_version]\n";return 2;}
    try{
        const int version=argc==5?std::stoi(argv[4]):2;if(version!=2&&version!=3)throw std::runtime_error("parent ES version must be 2 or 3");
        MotionProbeEgl parent(version);
        std::cout<<"parent EGL client version="<<version<<'\n';
        const auto display=reinterpret_cast<uintptr_t>(eglGetCurrentDisplay());
        const auto context=reinterpret_cast<uintptr_t>(eglGetCurrentContext());
        std::promise<SharedFrame> ready;auto result=ready.get_future();
        std::promise<void> consumed;auto release=consumed.get_future();
        std::promise<std::string> closed;auto closing=closed.get_future();
        std::thread worker([&]{
            StudyMotionRuntime *runtime=nullptr;char error[1024]={};bool announced=false;
            try{
                runtime=study_motion_create(error,sizeof(error));if(!runtime)throw std::runtime_error(error);
                auto require=[&](int success){if(!success)throw std::runtime_error(study_motion_last_error(runtime));};
                require(study_motion_initialize_gles(runtime,display,context));
                StudyMotionStats stats{};require(study_motion_stats(runtime,&stats));
                if(stats.capabilities!=31)throw std::runtime_error("GLES capability missing");
                const auto project=study_motion_load_project(runtime,argv[1],static_cast<uint32_t>(std::stoull(argv[2])));if(!project)require(0);
                {
                    // Reproduce Save16: the first draw may precede every timer
                    // callback. Root setters must publish without elapsed time.
                    const auto cold=study_motion_create_player(runtime,project);if(!cold)require(0);
                    require(study_motion_set_scale(runtime,cold,.95,0,0));
                    require(study_motion_set_coord(runtime,cold,0,765,0,0));
                    StudyMotionTransform before{},after{};require(study_motion_get_transform(runtime,cold,&before));
                    if(before.controller_y!=765||before.player_y!=0||before.frame!=0)
                        throw std::runtime_error("cold frame fixture did not separate pending and published transforms");
                    const double stage[]={1,0,0,1,960,690};StudyMotionFrame coldFrame{};
                    require(study_motion_render_player(runtime,cold,1920,1380,stage,&coldFrame));
                    require(study_motion_get_transform(runtime,cold,&after));
                    if(after.player_y!=765||after.player_x!=0||after.player_scale_x!=double(float(.95))||after.frame!=0)
                        throw std::runtime_error("render before first timer lost saved root transform or advanced time");
                    std::vector<uint8_t> first(1920*1380*4),second(first.size());
                    require(study_motion_read_pixels(runtime,cold,first.data(),first.size()));
                    require(study_motion_render_player(runtime,cold,1920,1380,stage,&coldFrame));
                    require(study_motion_read_pixels(runtime,cold,second.data(),second.size()));
                    if(first!=second)throw std::runtime_error("repeated zero-time render changed pixels");
                    size_t alpha=0;for(size_t i=3;i<first.size();i+=4)alpha+=first[i]!=0;
                    if(!alpha)throw std::runtime_error("cold actor first frame is empty");
                    require(study_motion_destroy_player(runtime,cold));
                    std::cout<<"cold render publishes saved root without timer: PASS frame=0 coordY=765 alpha="<<alpha<<'\n';
                }
                const auto actor=study_motion_create_player(runtime,project);if(!actor)require(0);
                double l,t,r,b;require(study_motion_player_bounds(runtime,actor,&l,&t,&r,&b));
                if(r<=l||b<=t)throw std::runtime_error("empty bounds");
                const double scale=std::min(960/(r-l),960/(b-t));
                const double matrix[]={scale,0,0,scale,512-(l+r)*scale/2,512-(t+b)*scale/2};
                SharedFrame output;require(study_motion_render_player(runtime,actor,1024,1024,matrix,&output.frame));
                output.pixels.resize(1024*1024*4);require(study_motion_read_pixels(runtime,actor,output.pixels.data(),output.pixels.size()));
                const auto firstTexture=output.frame.texture;ready.set_value(std::move(output));announced=true;
                release.wait();
                unsigned animations=0,draws=0;std::set<uLong> checksums;std::string lastTimeline,firstTimeline;
                for(uint32_t index=0;index<256&&animations<3;++index){
                    char label[512];double duration;int looping;
                    if(!study_motion_timeline_info(runtime,actor,0,index,label,sizeof(label),&duration,&looping))break;
                    if(!label[0]||label[0]=='-')continue;
                    require(study_motion_play_timeline(runtime,actor,label,1));
                    for(unsigned sample=0;sample<3;++sample){
                        require(study_motion_progress_player(runtime,actor,10));StudyMotionFrame frame{};
                        require(study_motion_render_player(runtime,actor,1024,1024,matrix,&frame));
                        std::vector<uint8_t> pixels(1024*1024*4);require(study_motion_read_pixels(runtime,actor,pixels.data(),pixels.size()));
                        size_t alpha=0;for(size_t p=3;p<pixels.size();p+=4)alpha+=pixels[p]!=0;
                        if(!alpha)throw std::runtime_error("active timeline rendered empty texture");
                        checksums.insert(crc32(0,pixels.data(),pixels.size()));++draws;
                    }
                    ++animations;
                    lastTimeline=label;
                    if(firstTimeline.empty())firstTimeline=label;
                }
                if(animations!=3||checksums.size()<2)throw std::runtime_error("timeline render did not exercise changing output");
                std::cout<<"actual timelines="<<animations<<" rendered_frames="<<draws<<" unique_images="<<checksums.size()<<": PASS\n";
                auto roundtrip=[&](const std::string &label,bool requireActive){
                    double savedCursor;int savedPlaying;
                    require(study_motion_timeline_position(runtime,actor,label.c_str(),&savedCursor));require(study_motion_is_timeline_playing(runtime,actor,label.c_str(),&savedPlaying));
                    std::cout<<"saving timeline='"<<label<<"' cursor="<<savedCursor<<" active="<<savedPlaying<<" texture="<<firstTexture<<'\n';
                    if(requireActive&&!savedPlaying)throw std::runtime_error("GPU state regression timeline is inactive");
                    void *saved=nullptr;size_t savedSize=0;require(study_motion_save_state(runtime,actor,&saved,&savedSize));
                    std::vector<uint8_t> snapshot(static_cast<uint8_t *>(saved),static_cast<uint8_t *>(saved)+savedSize);study_motion_free_buffer(saved);
                    require(study_motion_progress_player(runtime,actor,20));require(study_motion_restore_state(runtime,actor,snapshot.data(),snapshot.size()));
                    double restoredCursor;int restoredPlaying;require(study_motion_timeline_position(runtime,actor,label.c_str(),&restoredCursor));require(study_motion_is_timeline_playing(runtime,actor,label.c_str(),&restoredPlaying));
                    std::cout<<"restored timeline='"<<label<<"' cursor="<<restoredCursor<<" prior_cursor="<<savedCursor<<" active="<<restoredPlaying<<" expected_active="<<savedPlaying<<'\n';
                    if(restoredPlaying!=savedPlaying||(savedPlaying&&restoredCursor!=savedCursor))throw std::runtime_error("GPU actor active timeline save/load mismatch");
                };
                // Retain the previously failing case with full diagnostics.
                // Native serializeTimelineState saves active labels only;
                // an inactive label's stale private cursor is not serialized.
                roundtrip(lastTimeline,false);
                require(study_motion_play_timeline(runtime,actor,firstTimeline.c_str(),1));require(study_motion_progress_player(runtime,actor,5));
                roundtrip(firstTimeline,true);
                StudyMotionFrame restoredFrame{};require(study_motion_render_player(runtime,actor,1024,1024,matrix,&restoredFrame));
                std::cout<<"restored output texture="<<restoredFrame.texture<<" expected="<<firstTexture<<'\n';
                if(restoredFrame.texture!=firstTexture)throw std::runtime_error("save/load invalidated borrowed output texture");
                std::cout<<"structured state restore retains timeline and borrowed GPU texture: PASS\n";
                require(study_motion_destroy_player(runtime,actor));require(study_motion_unload_project(runtime,project));
                if(!study_motion_destroy(runtime,error,sizeof(error)))throw std::runtime_error(error);runtime=nullptr;
                closed.set_value("");
            }catch(const std::exception &e){
                if(!announced){SharedFrame failure;failure.error=e.what();ready.set_value(std::move(failure));}
                if(runtime)study_motion_destroy(runtime,error,sizeof(error));
                closed.set_value(e.what());
            }
        });
        const auto output=result.get();std::string failure;
        try{
            if(!output.error.empty())throw std::runtime_error(output.error);
            if(!output.frame.texture||output.frame.width!=1024||output.frame.texture_origin_bottom_left!=1||output.frame.premultiplied_alpha!=1)throw std::runtime_error("invalid exported texture metadata");
            GLuint fbo;glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,output.frame.texture,0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){glDeleteFramebuffers(1,&fbo);throw std::runtime_error("worker texture unavailable in parent EGL context");}
            std::vector<uint8_t> parentPixels(output.pixels.size());glReadPixels(0,0,1024,1024,GL_RGBA,GL_UNSIGNED_BYTE,parentPixels.data());
            const auto glError=glGetError();glDeleteFramebuffers(1,&fbo);if(glError)throw std::runtime_error("parent shared-texture readback failed");
            for(size_t y=0;y<1024;++y)if(std::memcmp(parentPixels.data()+y*4096,output.pixels.data()+(1023-y)*4096,4096))throw std::runtime_error("worker/parent shared texture pixels differ");
            size_t alpha=0;for(size_t i=3;i<output.pixels.size();i+=4)alpha+=output.pixels[i]!=0;
            if(!alpha)throw std::runtime_error("opaque API rendered empty target");
            std::ofstream image(argv[3],std::ios::binary);image<<"P7\nWIDTH 1024\nHEIGHT 1024\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";image.write(reinterpret_cast<const char *>(output.pixels.data()),output.pixels.size());if(!image)throw std::runtime_error("write failed");
            std::cout<<"opaque API worker/shared parent EGL texture: PASS alpha_pixels="<<alpha<<" crc32="<<std::hex<<crc32(0,output.pixels.data(),output.pixels.size())<<std::dec<<" capabilities=31\n";
        }catch(const std::exception &e){failure=e.what();}
        consumed.set_value();worker.join();const auto closeError=closing.get();
        if(!failure.empty())throw std::runtime_error(failure);if(!closeError.empty())throw std::runtime_error(closeError);
        if(!glGetString(GL_VERSION))throw std::runtime_error("parent EGL context invalidated by worker shutdown");
        std::cout<<"real EmoteEngine metadata/selector/render/readback and worker shutdown: PASS\n";return 0;
    }catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
}
