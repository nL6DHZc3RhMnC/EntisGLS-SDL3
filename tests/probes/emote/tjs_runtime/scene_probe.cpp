#include "../../../fixtures/emote/probe_asset.h"
#include "Player.h"
#include "PlayerInternal.h"
#include "ResourceManager.h"
#include "MotionBezierPatch.h"
#include "extensions/emote/tjs_runtime/texture_bridge.h"
#include "ncbind.hpp"
#include "EmoteEngine.h"
#include <iostream>
#include <fstream>
#ifdef MOTION_SCENE_GLES
#include "../../../fixtures/emote/egl_probe_context.h"
#include "extensions/emote/tjs_runtime/gles_render_manager.h"
#include "extensions/emote/tjs_runtime/gles_scene_bridge.h"
#endif
#define NCB_MODULE_NAME TJS_W("motion_scene_probe.dll")
#include "MotionGeometryRegistration.inc"
using motion::Player;
using motion::ResourceManager;
NCB_REGISTER_CLASS(ResourceManager) {
    NCB_CONSTRUCTOR(());NCB_METHOD(requireLayerId);NCB_METHOD(releaseLayerId);
    NCB_METHOD(random);NCB_METHOD(findMotion);NCB_METHOD(isExistMotion);NCB_METHOD(findSource);
}
NCB_REGISTER_CLASS(Player) {NCB_CONSTRUCTOR((tTJSVariant));}
struct SceneCounts {size_t players=0,nodes=0,sources=0,draws=0;};
void countScene(const Player &player,SceneCounts &counts,int depth=0) {
    if(depth>64)throw std::runtime_error("Player hierarchy exceeded depth bound");
    ++counts.players;counts.nodes+=player.nodes().size();
    for(const auto &node:player.nodes()) {
        counts.sources+=node.source.valid;counts.draws+=node.drawFlag;
        if(node.nodeType==3)countScene(*node.getChildPlayer(),counts,depth+1);
    }
}
int main(int argc,char **argv) {
    if(argc!=
#ifdef MOTION_SCENE_GLES
       4
#else
       3
#endif
       ){std::cerr<<"Usage: motion_scene_probe haz_a.psb header_seed [gles_output.pam]\n";return 2;}
    try {
#ifdef MOTION_SCENE_GLES
        MotionProbeEgl egl;
#endif
        spdlog::stdout_color_mt("tjs2");spdlog::stdout_color_mt("core");spdlog::stdout_color_mt("plugin");
        auto *engine=new tTJS();motionSetTjsEngine(engine);ncbAutoRegister::AllRegist();
        if(!ncbAutoRegister::LoadModule(TJS_W("motion_scene_probe.dll")))throw std::runtime_error("NCB registration failed");
        motion::internal::initializeBezierPatchRuntime_guess();
#ifdef MOTION_SCENE_GLES
        studysteady::motion::GlesRenderManager renderer;
#else
        studysteady::motion::TextureBridgeManager renderer(false);
#endif
        studysteady::motion::useRenderManager(&renderer);
        motionSetPsbHeaderSeed(static_cast<uint32_t>(std::stoull(argv[2])));
        {
            auto *manager=new ResourceManager();auto *dispatch=ncbInstanceAdaptor<ResourceManager>::CreateAdaptor(manager);
            if(!dispatch){delete manager;throw std::runtime_error("manager adaptor failed");}
            tTJSVariant owner(dispatch,dispatch);dispatch->Release();
            const auto root=manager->load(ttstr(argv[1]));
            const auto base=motion::detail::motionPropGet(motion::detail::motionPropGet(root,TJS_W("metadata")),TJS_W("base"));
            motion::EmoteEngine emote(owner);auto &player=emote.player();
            player.setProject(tTJSVariant(TVPGetPlacedPath(ttstr(argv[1]))));
            player.setChara(motion::detail::motionPropGetString(base,TJS_W("chara")));
            player.playMotion_guess(motion::PlayFlagForce,motion::detail::motionPropGetString(base,TJS_W("motion")));
            emote.applyMetadata_guess(motion::detail::motionPropGet(root,TJS_W("metadata")));
            for(int frame=0;frame<3;++frame) {
                emote.progress(frame?1.0:0.0);
                player.calcBoundsForDifferentialTest_guess();
#ifdef MOTION_SCENE_GLES
                const auto bounds=player.getBounds();
                const auto l=motion::detail::motionPropGetDouble(bounds,TJS_W("left"));
                const auto t=motion::detail::motionPropGetDouble(bounds,TJS_W("top"));
                const auto r=motion::detail::motionPropGetDouble(bounds,TJS_W("right"));
                const auto b=motion::detail::motionPropGetDouble(bounds,TJS_W("bottom"));
                if(r<=l||b<=t)throw std::runtime_error("invalid scene bounds");
                const double scale=std::min(960/(r-l),960/(b-t));
                player.setDrawAffineTranslateMatrix(scale,0,0,scale,512-(l+r)*scale/2,512-(t+b)*scale/2);
#endif
                SceneCounts counts;countScene(player,counts);
                motion::detail::PreparedRenderItemList main,aux;
                if(!player.prepareRenderItemsForDifferentialTest_guess(main,aux))throw std::runtime_error("no prepared motion content");
                size_t textured=0,blank=0,meshed=0;std::map<int,size_t> blends;
                for(auto *item:main){if(item->hasOwnSource && item->sourceState){textured+=item->sourceState->texture!=nullptr;blank+=item->sourceState->blank;}meshed+=item->meshType!=0;++blends[item->blendMode];}
                std::cout<<"frame="<<frame<<" real_players="<<counts.players<<" nodes="<<counts.nodes<<" valid_sources="<<counts.sources<<" draw_nodes="<<counts.draws<<" main_items="<<main.size()<<" aux_items="<<aux.size()<<'\n';
                std::cout<<"texture_items="<<textured<<" blank_items="<<blank<<" mesh_items="<<meshed<<" blends=";for(auto [blend,count]:blends)std::cout<<blend<<":"<<count<<" ";std::cout<<'\n';
                if(counts.players<2||counts.sources==0||main.empty())throw std::runtime_error("scene failed to produce actual sources/render items");
#ifdef MOTION_SCENE_GLES
                auto *target=renderer.CreateTexture2D(nullptr,0,1024,1024,TVPTextureFormat::RGBA,0);
                std::unique_ptr<iTVPTexture2D,void(*)(iTVPTexture2D *)> targetOwner(target,[](iTVPTexture2D *t){t->Release();});
                renderer.clearTarget(target);
                studysteady::motion::renderSceneItems(false,target,
                    [target](bool,const tTVPRect &){return motion::D3DTargetTexturePair_guess(target,target);},
                    {0,0,1024,1024},[](motion::detail::PreparedRenderItem &item){
                        if(!item.sourceState->texture)throw std::runtime_error("render source texture missing");return item.sourceState->texture;
                    },main,.5f,.5f);
                const auto pixels=renderer.readTarget(target);size_t visible=0;
                for(size_t i=3;i<pixels.size();i+=4)visible+=pixels[i]!=0;
                if(!visible)throw std::runtime_error("GLES rendered an empty character target");
                if(frame==0){std::ofstream out(argv[3],std::ios::binary);out<<"P7\nWIDTH 1024\nHEIGHT 1024\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";out.write(reinterpret_cast<const char *>(pixels.data()),pixels.size());if(!out)throw std::runtime_error("render image write failed");}
                std::cout<<"actual GLES frame="<<frame<<" alpha_pixels="<<visible<<'\n';
#endif
            }
        }
        unsigned draws;uint64_t bytes;renderer.GetRenderStat(draws,bytes);
        std::cout<<"live texture bytes after ResourceManager release="<<bytes<<'\n';
        if(bytes!=0)throw std::runtime_error("scene texture ownership leaked");
        studysteady::motion::useRenderManager(nullptr);ncbAutoRegister::AllUnregist();engine->Shutdown();engine->Release();motionSetTjsEngine(nullptr);
        std::cout<<"real Player playMotion/frameProgress/child hierarchy/source/prepare: PASS";
#ifdef MOTION_SCENE_GLES
        std::cout<<"; actual GLES rasterization PASS\n";
#else
        std::cout<<"; no character rasterization\n";
#endif
        return 0;
    }catch(const eTJSError &e){std::cerr<<"TJS error: "<<e.GetMessage().AsStdString()<<'\n';return 1;}
     catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
}
