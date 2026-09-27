#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
#include "../../fixtures/psb_key_argument.h"
#include "RenderManager.h"
#include <GLES2/gl2.h>
#include <zlib.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Standalone probe: reads existing raw NOA entries through the same public
// storage boundary as the APK. Original assets and the running app are untouched.
struct Asset {
    std::string archive,name,sha256,root;
    uint64_t offset=0,size=0;
    uint32_t crc=0;
    unsigned allocations=0,releases=0;
};
static std::string quote(const std::string& text) {
    std::ostringstream out;out<<'"';
    for(unsigned char c:text) {
        if(c=='"'||c=='\\')out<<'\\'<<c;
        else if(c<32)out<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<unsigned(c)<<std::dec;
        else out<<c;
    }
    out<<'"';return out.str();
}
static int readAsset(void* opaque,const char* name,void** bytes,size_t* size,char* error,size_t capacity) {
    auto& asset=*static_cast<Asset*>(opaque);
    try {
        if(asset.name!=name)return 0;
        *size=asset.size;
        if(!bytes)return 1;
        if(asset.size<56||asset.size>256*1024*1024)throw std::runtime_error("asset size outside PSB limit");
        std::ifstream file(asset.root+"/"+asset.archive,std::ios::binary);
        if(!file)throw std::runtime_error("cannot open original NOA");
        file.seekg(asset.offset);if(!file)throw std::runtime_error("cannot seek original NOA entry");
        void* buffer=std::malloc(asset.size);if(!buffer)throw std::bad_alloc();
        file.read(static_cast<char*>(buffer),asset.size);
        if(!file){std::free(buffer);throw std::runtime_error("truncated original NOA entry");}
        const auto crc=crc32(0,static_cast<const Bytef*>(buffer),asset.size);
        if(crc!=asset.crc){std::free(buffer);throw std::runtime_error("original NOA bytes differ from host manifest CRC32");}
        *bytes=buffer;++asset.allocations;return 1;
    } catch(const std::exception& e) {
        if(error&&capacity){std::strncpy(error,e.what(),capacity-1);error[capacity-1]=0;}
        return -1;
    }
}
static void releaseAsset(void* opaque,void* bytes,size_t) {
    ++static_cast<Asset*>(opaque)->releases;std::free(bytes);
}
int main(int argc,char** argv) {
    if(argc!=4){std::cerr<<"usage: motion_asset_batch_probe manifest.tsv original_game_directory header_seed\n";return 2;}
    uint32_t psbKey=0;
    try {psbKey=ParsePsbKeyArgument(argv[3]);}
    catch(const std::exception &error){std::cerr<<error.what()<<'\n';return 2;}
    std::ifstream manifest(argv[1]);if(!manifest){std::cerr<<"cannot read manifest\n";return 2;}
    unsigned passed=0;std::string line;
    while(std::getline(manifest,line)) {
        if(line.empty()||line[0]=='#')continue;
        Asset asset;asset.root=argv[2];std::string crc;
        std::istringstream fields(line);
        if(!(fields>>asset.archive>>asset.name>>asset.offset>>asset.size>>crc>>asset.sha256)) {
            std::cerr<<"malformed manifest line\n";return 2;
        }
        asset.crc=std::stoul(crc,nullptr,16);
        std::cout<<"{\"status\":\"BEGIN\",\"archive\":"<<quote(asset.archive)<<",\"asset\":"<<quote(asset.name)<<"}"<<std::endl;
        StudyMotionRuntime* runtime=nullptr;char error[1024]={};std::string stage="create_owner";
        const auto start=std::chrono::steady_clock::now();
        try {
            runtime=study_motion_create(error,sizeof(error));if(!runtime)throw std::runtime_error(error);
            auto require=[&](bool value){if(!value)throw std::runtime_error(study_motion_last_error(runtime));};
            stage="initialize_gles";require(study_motion_initialize_gles(runtime,0,0));
            if(passed==0)std::cout<<"{\"status\":\"DEVICE\",\"gles\":"<<quote(reinterpret_cast<const char*>(glGetString(GL_VERSION)))
                <<",\"renderer\":"<<quote(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))<<"}"<<std::endl;
            StudyMotionStats stats{};require(study_motion_stats(runtime,&stats));
            if(stats.capabilities!=31)throw std::runtime_error("real GLES capability missing");
            stage="set_noa_reader";require(study_motion_set_reader(runtime,readAsset,releaseAsset,&asset));
            stage="load_project";const auto project=study_motion_load_project(runtime,asset.name.c_str(),psbKey);require(project!=0);
            char chara[1024],motion[1024];stage="metadata_base";
            require(study_motion_project_base(runtime,project,chara,sizeof(chara),motion,sizeof(motion)));
            stage="create_player";const auto actor=study_motion_create_player(runtime,project);require(actor!=0);
            double left,top,right,bottom;stage="bounds";
            require(study_motion_player_bounds(runtime,actor,&left,&top,&right,&bottom));
            if(!std::isfinite(left)||!std::isfinite(top)||!std::isfinite(right)||!std::isfinite(bottom)||right<=left||bottom<=top)
                throw std::runtime_error("invalid initial base bounds");
            const double scale=std::min(480/(right-left),480/(bottom-top));
            const double matrix[]={scale,0,0,scale,256-(left+right)*scale/2,256-(top+bottom)*scale/2};
            uint64_t alpha[2]={};uint32_t images[2]={};GLuint texture=0;
            std::vector<uint8_t> pixels(512*512*4);
            unsigned draws=0;uint64_t textureBytes=0;
            for(unsigned sample=0;sample<2;++sample) {
                stage=sample?"base_progress_1":"base_render_0";
                if(sample)require(study_motion_progress_player(runtime,actor,1));
                StudyMotionFrame frame{};require(study_motion_render_player(runtime,actor,512,512,matrix,&frame));
                if(!frame.texture||frame.width!=512||frame.height!=512||!frame.premultiplied_alpha||!frame.texture_origin_bottom_left)
                    throw std::runtime_error("invalid actual GLES output metadata");
                texture=frame.texture;require(study_motion_read_pixels(runtime,actor,pixels.data(),pixels.size()));
                for(size_t i=3;i<pixels.size();i+=4)alpha[sample]+=pixels[i]!=0;
                images[sample]=crc32(0,pixels.data(),pixels.size());
                if(!alpha[sample])throw std::runtime_error("actual metadata.base rendered no nonzero-alpha pixels");
                if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("GLES error after actual base readback");
            }
            TVPGetRenderManager()->GetRenderStat(draws,textureBytes);
            stage="destroy_player";require(study_motion_destroy_player(runtime,actor));
            if(glIsTexture(texture))throw std::runtime_error("destroyed Player retained output GL texture");
            stage="unload_project";require(study_motion_unload_project(runtime,project));
            require(study_motion_stats(runtime,&stats));unsigned releasedDraws;uint64_t releasedBytes;
            TVPGetRenderManager()->GetRenderStat(releasedDraws,releasedBytes);
            if(stats.project_count||stats.player_count||releasedBytes||asset.allocations!=asset.releases)
                throw std::runtime_error("project/player/texture/storage remained after explicit release");
            stage="destroy_owner";if(!study_motion_destroy(runtime,error,sizeof(error)))throw std::runtime_error(error);runtime=nullptr;
            const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
            ++passed;
            std::cout<<"{\"status\":\"PASS\",\"archive\":"<<quote(asset.archive)<<",\"asset\":"<<quote(asset.name)
                <<",\"bytes\":"<<asset.size<<",\"sha256\":"<<quote(asset.sha256)<<",\"chara\":"<<quote(chara)<<",\"motion\":"<<quote(motion)
                <<",\"bounds\":["<<left<<','<<top<<','<<right<<','<<bottom<<"],\"alpha_pixels\":["<<alpha[0]<<','<<alpha[1]
                <<"],\"frame_crc32\":["<<images[0]<<','<<images[1]<<"],\"draws\":"<<draws<<",\"texture_bytes\":"<<textureBytes
                <<",\"remaining_texture_bytes\":0,\"remaining_projects\":0,\"remaining_players\":0,\"storage_reads\":"<<asset.allocations
                <<",\"storage_releases\":"<<asset.releases<<",\"owner_destroyed\":true,\"elapsed_ms\":"<<elapsed<<"}"<<std::endl;
        } catch(const std::exception& e) {
            const std::string reason=e.what();std::string cleanup;
            if(runtime&&!study_motion_destroy(runtime,error,sizeof(error)))cleanup=error;
            std::cout<<"{\"status\":\"FAIL\",\"archive\":"<<quote(asset.archive)<<",\"asset\":"<<quote(asset.name)
                <<",\"stage\":"<<quote(stage)<<",\"error\":"<<quote(reason)<<",\"cleanup_error\":"<<quote(cleanup)<<"}"<<std::endl;
            return 1;
        }
    }
    std::cout<<"{\"status\":\"SUMMARY\",\"passed\":"<<passed<<",\"scope\":\"metadata.base real Player and two fitted GLES frames; no named-timeline or gameplay coverage\"}"<<std::endl;
    return passed?0:2;
}
