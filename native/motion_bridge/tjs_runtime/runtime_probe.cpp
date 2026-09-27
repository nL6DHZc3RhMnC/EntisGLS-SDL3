#include "motion_apk_runtime.h"
#include "tjs.h"
#include "tjsBinarySerializer.h"
#include <memory>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
// Inspect snapshots using the actual TJS structured-binary reader. This is a
// read-only probe stream; the production decoder performs its own validation.
class SnapshotStream final:public tTJSBinaryStream {
    const uint8_t* bytes_;size_t length_,position_=8;
public:
    SnapshotStream(const void* bytes,size_t length):bytes_(static_cast<const uint8_t*>(bytes)),length_(length){}
    tjs_uint64 Seek(tjs_int64 offset,tjs_int whence) override {
        const int64_t base=whence==SEEK_SET?0:whence==SEEK_CUR?position_:whence==SEEK_END?length_:-1;
        if(base<0||offset< -base||offset>int64_t(length_)-base)throw std::runtime_error("probe snapshot seek");
        return position_=base+offset;
    }
    tjs_uint Read(void* out,tjs_uint count) override {const auto n=std::min<size_t>(count,length_-position_);std::memcpy(out,bytes_+position_,n);position_+=n;return n;}
    tjs_uint Write(const void*,tjs_uint) override {throw std::runtime_error("probe snapshot is read-only");}
    tjs_uint64 GetSize() override {return length_;}
};
struct RootTransform {double base,user,scale,x,y;};
RootTransform rootTransform(StudyMotionRuntime* runtime,uint64_t actor) {
    void* bytes=nullptr;size_t length=0;
    if(!study_motion_save_state(runtime,actor,&bytes,&length))throw std::runtime_error(study_motion_last_error(runtime));
    std::unique_ptr<void,decltype(&study_motion_free_buffer)> buffer(bytes,study_motion_free_buffer);
    SnapshotStream stream(bytes,length);tTJSBinarySerializer reader;
    std::unique_ptr<tTJSVariant> root(reader.Read(&stream));if(!root)throw std::runtime_error("probe snapshot root");
    auto property=[](const tTJSVariant& parent,const tjs_char* key){tTJSVariant value;auto* object=parent.AsObjectNoAddRef();if(TJS_FAILED(object->PropGet(0,key,nullptr,&value,object)))throw std::runtime_error("probe snapshot field");return value;};
    auto item=[](const tTJSVariant& parent,int index){tTJSVariant value;auto* object=parent.AsObjectNoAddRef();if(TJS_FAILED(object->PropGetByNum(0,index,&value,object)))throw std::runtime_error("probe snapshot channel");return value.AsReal();};
    const auto base=property(property(*root,TJS_W("state")),TJS_W("base"));
    const auto coord=property(property(base,TJS_W("coord")),TJS_W("frame"));
    return {property(*root,TJS_W("baseScale")).AsReal(),property(*root,TJS_W("userScale")).AsReal(),
        item(property(property(base,TJS_W("scale")),TJS_W("frame")),0),item(coord,0),item(coord,1)};
}
struct ReaderState {const char *path;int reads=0,releases=0,queries=0;};
int readArchive(void *user,const char *path,void **bytes,size_t *size,char *,size_t) {
    auto &state=*static_cast<ReaderState *>(user);
    if(std::strcmp(path,"archive/haz_a.psb"))return 0;
    std::ifstream input(state.path,std::ios::binary|std::ios::ate);if(!input)return -1;
    *size=static_cast<size_t>(input.tellg());
    if(!bytes){++state.queries;return 1;}
    *bytes=std::malloc(*size);if(!*bytes)return -1;
    ++state.reads;input.seekg(0);input.read(static_cast<char *>(*bytes),*size);
    return input?1:-1;
}
void releaseArchive(void *user,void *bytes,size_t) {
    ++static_cast<ReaderState *>(user)->releases;std::free(bytes);
}
int main(int argc,char **argv) {
    if(argc!=3){std::cerr<<"Usage: motion_runtime_probe haz_a.psb header_seed\n";return 2;}
    StudyMotionRuntime *r=nullptr;char error[512];
    std::vector<uint8_t> crossOwnerSnapshot;std::string savedTimeline;double savedFrame=0;
    try {
        for(int cycle=0;cycle<3;++cycle) {
            std::cerr<<"runtime cycle="<<cycle<<'\n';
            r=study_motion_create(error,sizeof(error));if(!r)throw std::runtime_error(error);
            if(study_motion_create(error,sizeof(error)))throw std::runtime_error("duplicate runtime accepted");
            StudyMotionStats stats{};
            if(!study_motion_stats(r,&stats)||stats.capabilities!=15||stats.project_count!=0)
                throw std::runtime_error("runtime capabilities invalid");
            const auto id=study_motion_load_project(r,argv[1],static_cast<uint32_t>(std::stoull(argv[2])));
            if(!id)throw std::runtime_error(study_motion_last_error(r));
            char chara[128],motion[128];
            if(!study_motion_project_base(r,id,chara,sizeof(chara),motion,sizeof(motion)))throw std::runtime_error(study_motion_last_error(r));
            if(std::string(chara)!="all_parts"||std::string(motion)!="タイムライン構造")throw std::runtime_error("unexpected base metadata");
            const auto actor=study_motion_create_player(r,id);if(!actor)throw std::runtime_error(study_motion_last_error(r));
            double l,t,right,b;
            if(!study_motion_player_bounds(r,actor,&l,&t,&right,&b)||right<=l||b<=t)throw std::runtime_error("real EmoteEngine bounds invalid");
            if(!study_motion_set_coord(r,actor,20,-10,0,0)||!study_motion_set_scale(r,actor,2,0,0)||!study_motion_progress_player(r,actor,0))throw std::runtime_error(study_motion_last_error(r));
            double ll,tt,rr,bb;
            if(!study_motion_player_bounds(r,actor,&ll,&tt,&rr,&bb))throw std::runtime_error(study_motion_last_error(r));
            std::cerr<<"root bounds before="<<l<<","<<t<<","<<right<<","<<b<<" after="<<ll<<","<<tt<<","<<rr<<","<<bb<<'\n';
            // Node AABBs are quantized to pixel boundaries before merging;
            // doubling pre-rounded bounds can differ by one output pixel.
            if(std::fabs(ll-(2*l+20))>1.01||std::fabs(tt-(2*t-10))>1.01||std::fabs(rr-(2*right+20))>1.01||std::fabs(bb-(2*b-10))>1.01)throw std::runtime_error("actual root scale/coordinate controllers did not transform geometry");
            int playing=-1;if(!study_motion_stop_timeline(r,actor,"")||!study_motion_is_timeline_playing(r,actor,"",&playing)||playing)throw std::runtime_error("timeline stop/query mismatch");
            if(!study_motion_set_coord(r,actor,0,0,0,0)||!study_motion_set_scale(r,actor,1,0,0)||!study_motion_progress_player(r,actor,0))throw std::runtime_error(study_motion_last_error(r));
            if(cycle==0){
                char timeline[512];double duration;int looping;
                bool selected=false;
                for(uint32_t index=0;index<256;++index){
                    if(!study_motion_timeline_info(r,actor,0,index,timeline,sizeof(timeline),&duration,&looping))break;
                    // Metadata includes decorative separators as zero-keyframe
                    // timelines. Exercise a real named animation instead.
                    if(timeline[0]&&timeline[0]!='-'){
                        if(!study_motion_play_timeline(r,actor,timeline,1)||!study_motion_progress_player(r,actor,5)||!study_motion_is_timeline_playing(r,actor,timeline,&playing))throw std::runtime_error(study_motion_last_error(r));
                        if(playing){selected=true;break;}
                    }
                }
                if(!selected)throw std::runtime_error("no actual main timeline found");
                std::cerr<<"actual timeline="<<timeline<<" native_duration="<<duration<<" looping="<<looping<<'\n';
                if(!study_motion_set_coord(r,actor,20,-10,0,0)||!study_motion_set_scale(r,actor,2,0,0)||!study_motion_progress_player(r,actor,0))throw std::runtime_error(study_motion_last_error(r));
                const auto clone=study_motion_clone_player(r,actor);if(!clone)throw std::runtime_error(study_motion_last_error(r));
                const auto sourceRoot=rootTransform(r,actor),initialCloneRoot=rootTransform(r,clone);
                if(sourceRoot.base!=initialCloneRoot.base||sourceRoot.user!=initialCloneRoot.user||
                   sourceRoot.scale!=initialCloneRoot.scale||sourceRoot.x!=initialCloneRoot.x||sourceRoot.y!=initialCloneRoot.y)
                    throw std::runtime_error("clone did not preserve actual root controllers and scale envelope");
                int originalPlaying=-1,clonePlaying=-2;
                if(!study_motion_is_timeline_playing(r,actor,timeline,&originalPlaying)||!study_motion_is_timeline_playing(r,clone,timeline,&clonePlaying)||originalPlaying!=clonePlaying)throw std::runtime_error("real Engine clone lost timeline state");
                double originalFrame=-1,cloneFrame=-2;
                if(!study_motion_timeline_position(r,actor,timeline,&originalFrame)||!study_motion_timeline_position(r,clone,timeline,&cloneFrame)||originalFrame<=0||originalFrame!=cloneFrame)throw std::runtime_error("clone timeline currentTime mismatch");
                std::cerr<<"cloned timeline snapshot frame="<<cloneFrame<<'\n';
                double originalBounds[4];if(!study_motion_player_bounds(r,actor,&originalBounds[0],&originalBounds[1],&originalBounds[2],&originalBounds[3]))throw std::runtime_error(study_motion_last_error(r));
                if(!study_motion_progress_player(r,clone,1)||!study_motion_player_bounds(r,clone,&l,&t,&right,&b))throw std::runtime_error(study_motion_last_error(r));
                if(!study_motion_set_scale(r,clone,3,0,0)||!study_motion_progress_player(r,clone,0)||!study_motion_player_bounds(r,clone,&ll,&tt,&rr,&bb))throw std::runtime_error(study_motion_last_error(r));
                if(!study_motion_player_bounds(r,actor,&ll,&tt,&rr,&bb)||ll!=originalBounds[0]||tt!=originalBounds[1]||rr!=originalBounds[2]||bb!=originalBounds[3])throw std::runtime_error("cloned actor mutation changed original geometry");
                // Physics chain point caches are not part of native engine
                // serialization. Scale re-evaluation can change a model AABB;
                // validate the real published root controller instead.
                const auto cloneRoot=rootTransform(r,clone);
                if(cloneRoot.user!=3||cloneRoot.scale!=cloneRoot.base*3||cloneRoot.x!=20||cloneRoot.y!=-10)
                    throw std::runtime_error("cloned actor root controller/scale envelope mismatch");
                std::cerr<<"clone root current scale="<<cloneRoot.scale<<" base="<<cloneRoot.base<<" user="<<cloneRoot.user<<": PASS\n";
                void *saved=nullptr;size_t savedSize=0;
                if(!study_motion_save_state(r,actor,&saved,&savedSize))throw std::runtime_error(study_motion_last_error(r));
                std::vector<uint8_t> snapshot(static_cast<uint8_t *>(saved),static_cast<uint8_t *>(saved)+savedSize);study_motion_free_buffer(saved);
                std::cerr<<"structured binary state bytes="<<snapshot.size()<<'\n';
                if(!study_motion_set_scale(r,actor,.5,0,0)||!study_motion_progress_player(r,actor,20)||!study_motion_restore_state(r,actor,snapshot.data(),snapshot.size()))throw std::runtime_error(study_motion_last_error(r));
                double restoredFrame=-1;if(!study_motion_timeline_position(r,actor,timeline,&restoredFrame)||restoredFrame!=originalFrame)throw std::runtime_error("persistent timeline cursor restore mismatch");
                for(const size_t truncated:{size_t(0),size_t(8),snapshot.size()/2,snapshot.size()-1})if(study_motion_restore_state(r,actor,snapshot.data(),truncated))throw std::runtime_error("truncated structured state accepted");
                auto malformed=snapshot;malformed.push_back(0);if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("trailing structured state accepted");
                const std::vector<std::vector<uint8_t>> invalidBodies={
                    {0xdf,255,255,255,255}, // map count overflow / allocation bomb
                    {0xc6,255,255,255,255}, // string length overflow
                    {0x82,0xa1,'x',0,0,0xa1,'x',0,0}, // duplicate map key
                    {0xa1,0,0xd8} // unpaired UTF-16 surrogate
                };
                for(const auto &body:invalidBodies){malformed.assign(snapshot.begin(),snapshot.begin()+8);malformed.insert(malformed.end(),body.begin(),body.end());if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("invalid binary tree accepted");}
                malformed.assign(snapshot.begin(),snapshot.begin()+8);malformed.insert(malformed.end(),34,0x91);malformed.push_back(0);
                if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("excessive binary nesting accepted");
                const std::vector<uint8_t> phaseKey={0xa5,'p',0,'h',0,'a',0,'s',0,'e',0};
                malformed=snapshot;auto phase=std::search(malformed.begin(),malformed.end(),phaseKey.begin(),phaseKey.end());if(phase==malformed.end()||*(phase+phaseKey.size())>127)throw std::runtime_error("snapshot phase regression target absent");
                *(phase+phaseKey.size())=127;if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("invalid controller phase accepted");
                malformed=snapshot;phase=std::search(malformed.begin(),malformed.end(),phaseKey.begin(),phaseKey.end());const auto offset=phase-malformed.begin()+phaseKey.size();
                malformed.erase(malformed.begin()+offset);const uint8_t nan[]={0xcb,0,0,0,0,0,0,0xf8,0x7f};malformed.insert(malformed.begin()+offset,std::begin(nan),std::end(nan));
                if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("NaN controller phase accepted");
                // Change a real timeline label without changing binary lengths.
                const std::vector<uint8_t> labelBytes={0x55,0x81,0x07,0x52,0xff,0x66,0x41,0}; // 腕切替A in UTF-16LE
                malformed=snapshot;const auto where=std::search(malformed.begin(),malformed.end(),labelBytes.begin(),labelBytes.end());
                if(where==malformed.end())throw std::runtime_error("snapshot timeline test label missing");*(where+6)=0x5a;
                if(study_motion_restore_state(r,actor,malformed.data(),malformed.size()))throw std::runtime_error("unknown timeline label accepted");
                if(!study_motion_timeline_position(r,actor,timeline,&restoredFrame)||restoredFrame!=originalFrame)throw std::runtime_error("failed restore mutated existing actor");
                std::cerr<<"structured binary roundtrip, truncation/trailing/label rejection and failure atomicity: PASS\n";
                crossOwnerSnapshot=snapshot;savedTimeline=timeline;savedFrame=originalFrame;
                for(int frame=0;frame<120;++frame)if(!study_motion_progress_player(r,clone,1))throw std::runtime_error(study_motion_last_error(r));
                if(!study_motion_player_bounds(r,clone,&ll,&tt,&rr,&bb)||!std::isfinite(ll)||!std::isfinite(tt)||!std::isfinite(rr)||!std::isfinite(bb)||rr<=ll||bb<=tt)throw std::runtime_error("cloned timeline produced invalid geometry");
                if(!study_motion_destroy_player(r,clone)||!study_motion_stop_timeline(r,actor,""))throw std::runtime_error(study_motion_last_error(r));
            }
            if(study_motion_unload_project(r,id))throw std::runtime_error("unloaded project with live player");
            if(!study_motion_set_variable(r,actor,"bridge_probe",2.5,1,0)||!study_motion_progress_player(r,actor,1))throw std::runtime_error(study_motion_last_error(r));
            double variable=0;if(!study_motion_get_variable(r,actor,"bridge_probe",&variable)||variable!=2.5)throw std::runtime_error("actual EmoteEngine variable binding failed");
            if(cycle==1){double frame=-1;
                if(crossOwnerSnapshot.empty())throw std::runtime_error("new owner snapshot is empty");
                if(!study_motion_restore_state(r,actor,crossOwnerSnapshot.data(),crossOwnerSnapshot.size()))throw std::runtime_error(std::string("new owner restore failed: ")+study_motion_last_error(r));
                if(!study_motion_timeline_position(r,actor,savedTimeline.c_str(),&frame))throw std::runtime_error(std::string("new owner timeline query failed: ")+study_motion_last_error(r));
                std::cerr<<"new owner restored timeline="<<savedTimeline<<" cursor="<<frame<<" expected="<<savedFrame<<'\n';
                if(frame!=savedFrame)throw std::runtime_error("new owner timeline cursor mismatch");
                if(!study_motion_progress_player(r,actor,1))throw std::runtime_error(std::string("new owner progress failed: ")+study_motion_last_error(r));
                std::cerr<<"snapshot restored after full owner destruction/recreation: PASS\n";
            }
            if(!study_motion_destroy_player(r,actor))throw std::runtime_error(study_motion_last_error(r));
            if(study_motion_load_project(r,argv[1],static_cast<uint32_t>(std::stoull(argv[2])))!=id)throw std::runtime_error("duplicate project changed identity");
            if(study_motion_project_base(r,id,chara,1,motion,sizeof(motion)))throw std::runtime_error("short identifier buffer accepted");
            int wrongThread=1;
            std::thread worker([&]{StudyMotionStats other{};wrongThread=study_motion_stats(r,&other);});worker.join();
            if(wrongThread)throw std::runtime_error("wrong thread accepted");
            if(!study_motion_unload_project(r,id)||study_motion_unload_project(r,id))throw std::runtime_error("project unload identity mismatch");
            ReaderState state{argv[1]};
            if(!study_motion_set_reader(r,readArchive,releaseArchive,&state))throw std::runtime_error(study_motion_last_error(r));
            const auto archiveId=study_motion_load_project(r,"archive/haz_a.psb",static_cast<uint32_t>(std::stoull(argv[2])));
            if(!archiveId)throw std::runtime_error(study_motion_last_error(r));
            if(state.reads!=1 || state.releases!=1 || state.queries!=1)throw std::runtime_error("archive callback ownership mismatch");
            if(study_motion_set_reader(r,nullptr,nullptr,nullptr))throw std::runtime_error("reader replaced with live project");
            if(!study_motion_destroy(r,error,sizeof(error)))throw std::runtime_error(error);r=nullptr;
        }
        std::cout<<"opaque runtime real TJS+NCB+ResourceManager, PSB base=all_parts/タイムライン構造: PASS\n";
        std::cout<<"create/load/unload/destroy cycles=3; duplicate-owner, wrong-thread, small-buffer rejection: PASS\n";
        std::cout<<"actual EmoteEngine position/scale/variable controllers and timeline query: PASS\n";
        std::cout<<"real Engine serialize/unserialize clone, active timeline and 120 frames: PASS\n";
        std::cout<<"capabilities=15; GLES renderer capability is absent\n";return 0;
    }catch(const std::exception &e){std::cerr<<e.what()<<'\n';if(r)study_motion_destroy(r,error,sizeof(error));return 1;}
}
