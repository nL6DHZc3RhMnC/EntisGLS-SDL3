#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
#include "extensions/emote/psb/psb_header.h"
#include "../../../fixtures/psb_key_argument.h"
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_init.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
struct State {
    std::vector<uint8_t> bytes;
    uint32_t key=0;
    int status=1,calls=0,reads=0,releases=0;
    bool wrongBytes=false,wrongThread=false;
    std::thread::id owner=std::this_thread::get_id();
};
void check(bool condition,const char *message) {
    if(!condition)throw std::runtime_error(message);
}
int read(void *user,const char *path,void **bytes,size_t *size,char *,size_t) {
    auto &s=*static_cast<State *>(user);
    if(std::strcmp(path,"archive/test.psb"))return 0;
    *size=s.bytes.size();
    if(!bytes)return 1;
    *bytes=std::malloc(*size);if(!*bytes)return -1;
    std::memcpy(*bytes,s.bytes.data(),*size);++s.reads;return 1;
}
void release(void *user,void *bytes,size_t) {
    ++static_cast<State *>(user)->releases;std::free(bytes);
}
int resolve(void *user,const char *path,const void *bytes,size_t size,uint32_t *key,
            char *error,size_t capacity) {
    auto &s=*static_cast<State *>(user);++s.calls;
    s.wrongThread|=std::this_thread::get_id()!=s.owner;
    s.wrongBytes|=std::strcmp(path,"archive/test.psb") || size!=s.bytes.size()
        || std::memcmp(bytes,s.bytes.data(),size);
    *key=s.key;
    if(s.status!=1 && capacity) {
        constexpr char message[]="test resolver cannot identify this key";
        const auto length=std::min(capacity-1,sizeof(message)-1);
        std::memcpy(error,message,length);error[length]=0;
    }
    return s.status;
}
}

// This integration probe consumes an externally supplied PSB and key. It embeds
// no game's decryption parameter and exercises the real ResourceManager reader.
int main(int argc,char **argv) {
    if(argc!=3) {
        std::cerr<<"Usage: motion_psb_key_resolver_probe file.psb header_seed\n";return 2;
    }
    StudyMotionRuntime *runtime=nullptr;char error[1024]={};
    try {
        const auto key=ParsePsbKeyArgument(argv[2]);
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        check(SDL_Init(0),SDL_GetError());
#endif
        State state;state.key=key;
        std::ifstream input(argv[1],std::ios::binary|std::ios::ate);
        check(bool(input),"cannot open input PSB");
        const auto size=input.tellg();
        check(size>=56 && size<=256*1024*1024,"input PSB size out of range");
        state.bytes.resize(static_cast<size_t>(size));input.seekg(0);
        input.read(reinterpret_cast<char *>(state.bytes.data()),size);
        check(bool(input) && (state.bytes[6]&1),"probe needs an encrypted PSB");

        runtime=study_motion_create(error,sizeof(error));check(runtime,error);
        check(study_motion_set_reader(runtime,read,release,&state),study_motion_last_error(runtime));
        check(study_motion_set_psb_key_resolver(runtime,resolve,&state),study_motion_last_error(runtime));
        const auto load=[&](uint32_t explicitKey) {
            return study_motion_load_project(runtime,"archive/test.psb",explicitKey);
        };
        state.status=0;
        check(!load(key),"failed resolver fell back to valid explicit key");
        check(std::string(study_motion_last_error(runtime)).find("test resolver cannot identify")!=std::string::npos,
              "resolver diagnostic was lost");
        state.status=2;
        check(!load(key),"invalid resolver status was accepted");
        check(std::string(study_motion_last_error(runtime)).find("invalid status")!=std::string::npos,
              "invalid callback status diagnostic was lost");
        state.status=1;state.key=key^1u;
        check(!load(key),"bad resolved key fell back to valid explicit key");
        state.key=key;
        auto project=load(key^1u);check(project,study_motion_last_error(runtime));
        check(state.calls==4 && !state.wrongBytes && !state.wrongThread,
              "resolver did not receive raw bytes on owner thread");
        check(!study_motion_set_psb_key_resolver(runtime,nullptr,nullptr),
              "resolver replacement accepted with a live project");
        check(study_motion_unload_project(runtime,project),study_motion_last_error(runtime));
        int wrongThreadResult=1;
        std::thread worker([&]{wrongThreadResult=study_motion_set_psb_key_resolver(runtime,nullptr,nullptr);});
        worker.join();check(!wrongThreadResult,"resolver replacement accepted on wrong thread");
        const int callsBefore=state.calls;
        project=load(key^1u);check(project,study_motion_last_error(runtime));
        check(state.calls==callsBefore+1,"wrong-thread replacement changed resolver");
        check(study_motion_unload_project(runtime,project),study_motion_last_error(runtime));

        studysteady::decodePsbHeader(state.bytes,key);state.status=0;
        const int plainCalls=state.calls;
        project=load(0);check(project,study_motion_last_error(runtime));
        check(state.calls==plainCalls,"unencrypted PSB called resolver");
        check(study_motion_unload_project(runtime,project),study_motion_last_error(runtime));
        state.bytes[40]^=1;
        check(!load(0),"unencrypted PSB skipped checksum validation");
        check(state.calls==plainCalls,"corrupt unencrypted PSB called resolver");
        check(state.reads==state.releases,"storage callback allocation leaked");
        check(study_motion_destroy(runtime,error,sizeof(error)),error);runtime=nullptr;

        runtime=study_motion_create(error,sizeof(error));check(runtime,error);
        project=study_motion_load_project(runtime,argv[1],key);
        check(project,study_motion_last_error(runtime));
        check(state.calls==plainCalls,"destroyed owner retained resolver");
        check(study_motion_destroy(runtime,error,sizeof(error)),error);runtime=nullptr;
        std::cout<<"PSB resolver: raw callback, precedence, failure diagnostics, thread/lifetime, unencrypted validation: PASS\n";
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        SDL_Quit();
#endif
        return 0;
    } catch(const std::exception &e) {
        std::cerr<<e.what()<<'\n';if(runtime)study_motion_destroy(runtime,error,sizeof(error));
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        SDL_Quit();
#endif
        return 1;
    }
}
