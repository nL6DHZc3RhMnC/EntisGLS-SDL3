#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
#include "../../fixtures/psb_key_argument.h"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <cmath>

int main(int argc,char** argv){
    if(argc!=4){std::fprintf(stderr,"usage: motion_cold_transform_probe nak_d.psb haz_h.psb header_seed\n");return 2;}
    char error[512];StudyMotionRuntime* runtime=nullptr;
    try{
        const auto psbKey=ParsePsbKeyArgument(argv[3]);
        for(int order=0;order<2;++order){
            runtime=study_motion_create(error,sizeof(error));if(!runtime)throw std::runtime_error(error);
            uint64_t actors[2]{};
            auto print=[&](int n,const char* stage){StudyMotionTransform t{};double l,top,r,b;
                if(!study_motion_get_transform(runtime,actors[n],&t)||!study_motion_player_bounds(runtime,actors[n],&l,&top,&r,&b))
                    throw std::runtime_error(study_motion_last_error(runtime));
                std::printf("order=%d actor=%s stage=%s controller=%g,%g,%g published=%g,%g,%g bounds=%g,%g,%g,%g\n",
                    order,n?"haz_h":"nak_d",stage,t.controller_scale,t.controller_x,t.controller_y,t.player_scale_x,t.player_x,t.player_y,l,top,r,b);
                if(std::fabs(t.controller_scale-(n?1.:double(float(.95))))>.00001||t.controller_x!=0||t.controller_y!=(n?744:765)||
                   t.player_scale_x!=t.controller_scale||t.player_x!=0||t.player_y!=t.controller_y)
                    throw std::runtime_error("saved transform was lost by live engine/actor interleaving");
            };
            for(int i=0;i<2;++i){const int n=order?1-i:i;
                const auto project=study_motion_load_project(runtime,argv[n+1],psbKey);
                if(!project||!(actors[n]=study_motion_create_player(runtime,project)))throw std::runtime_error(study_motion_last_error(runtime));
                if(!study_motion_set_scale(runtime,actors[n],n?1.:.95,0,0)||
                   !study_motion_set_coord(runtime,actors[n],0,n?744.:765.,0,0)||
                   !study_motion_progress_player(runtime,actors[n],0))throw std::runtime_error(study_motion_last_error(runtime));
                print(n,"restored");
            }
            print(0,"other-created");print(1,"other-created");
            for(int frame=0;frame<120;++frame){
                for(int n=0;n<2;++n)if(!study_motion_progress_player(runtime,actors[n],1))throw std::runtime_error(study_motion_last_error(runtime));
                if(frame==0||frame==29||frame==119){print(0,"progressed");print(1,"progressed");}
            }
            for(auto actor:actors)study_motion_destroy_player(runtime,actor);
            if(!study_motion_destroy(runtime,error,sizeof(error)))throw std::runtime_error(error);runtime=nullptr;
        }
        std::puts("Cold actor transform PASS: real Save16 values, both load orders and 120 live frames");return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());if(runtime)study_motion_destroy(runtime,error,sizeof(error));return 1;}
}
