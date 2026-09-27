#include "probe_asset.h"
#include "Player.h"
#include "PlayerInternal.h"
#include "NodeTree.h"
#include "ResourceManager.h"
#include "MotionBezierPatch.h"
#include "ncbind.hpp"
#include "player_mesh_bridge.h"
#include <iostream>
#include <cmath>
#define NCB_MODULE_NAME TJS_W("motion_timeline_probe.dll")
using motion::Player;
using motion::ResourceManager;
NCB_REGISTER_CLASS(ResourceManager) {
    NCB_CONSTRUCTOR(());NCB_METHOD(requireLayerId);NCB_METHOD(releaseLayerId);
    NCB_METHOD(random);NCB_METHOD(findMotion);NCB_METHOD(isExistMotion);
}
NCB_REGISTER_CLASS(Player) {NCB_CONSTRUCTOR((tTJSVariant));}
int main(int argc,char **argv) {
    if(argc!=3) {std::cerr<<"Usage: motion_timeline_probe haz_a.psb header_seed\n";return 2;}
    try {
        spdlog::stdout_color_mt("tjs2");spdlog::stdout_color_mt("core");spdlog::stdout_color_mt("plugin");
        auto *engine=new tTJS();motionSetTjsEngine(engine);ncbAutoRegister::AllRegist();
        if(!ncbAutoRegister::LoadModule(TJS_W("motion_timeline_probe.dll")))throw std::runtime_error("NCB registration failed");
        motion::internal::initializeBezierPatchRuntime_guess();
        motionSetPsbHeaderSeed(static_cast<uint32_t>(std::stoull(argv[2])));
        {
            PSB::PSBFile file;if(!file.LoadStorage(ttstr(argv[1])))throw std::runtime_error("PSB load failed");
            auto *manager=new ResourceManager();
            auto *dispatch=ncbInstanceAdaptor<ResourceManager>::CreateAdaptor(manager);
            if(!dispatch){delete manager;throw std::runtime_error("manager adaptor failed");}
            tTJSVariant owner(dispatch,dispatch);dispatch->Release();
            size_t motions=0,nodes=0,evaluations=0,changes=0,crossfades=0,legacyPatches=0,combinators=0,combinedSamples=0;
            probeMotions(file,[&](const auto &name,const auto &content){
                ++motions;Player player(owner);
                player.setMotionContentForDifferentialTest_guess(content);
                player.parseParameterListForDifferentialTest_guess(motion::detail::motionPropGet(content,TJS_W("parameter")));
                ncbPropAccessor accessor(content);motion::detail::buildNodeTree(player,accessor);
                for(size_t i=1;i<player.nodes().size();++i) {
                    auto &node=player.nodesForBuild()[i];++nodes;
                    combinators+=bool(node.meshCombinator);
                    const int count=motion::detail::motionPropGetCount(node.frameListVariant);
                    if(count<2)throw std::runtime_error(name+": expected two timeline sentinels");
                    for(int slot=0;slot<2;++slot){
                        motion::internal::parseNodeFrame_guess(node.slots[slot],node.frameListVariant,slot);
                        motion::internal::mergeNodeFrameContent_guess(node.slots[slot],node.nodeType,node.frameListVariant);
                    }
                    node.activeSlotIndex=0;
                    // Traverse exact frames and midpoints, then backwards. The
                    // real parameter table owns node.parameterEntry throughout.
                    std::vector<double> times;
                    for(int f=0;f<count-1;++f){
                        const double t=motion::detail::motionPropGetDouble(motion::detail::motionPropGetByNum(node.frameListVariant,f),TJS_W("time"));
                        const double n=motion::detail::motionPropGetDouble(motion::detail::motionPropGetByNum(node.frameListVariant,f+1),TJS_W("time"));
                        times.push_back(t);if(n>t)times.push_back((t+n)/2);
                    }
                    const size_t forwards=times.size();
                    for(size_t j=forwards;j>0;--j)times.push_back(times[j-1]);
                    for(double t:times){
                        if(node.parameterEntry)node.parameterEntry->value=t;
                        motion::internal::seekNodeFrameSelection_guess(node,t,nullptr);
                        const bool changed=player.evaluateTimelineForDifferentialTest_guess(node,t,true);
                        ++evaluations;changes+=changed;crossfades+=node.activeSlot().crossfading;
                        legacyPatches+=!node.meshControlPoints.empty();
                        const auto &a=node.accumulated;
                        if(!std::isfinite(a.posX)||!std::isfinite(a.posY)||!std::isfinite(a.angle)||!std::isfinite(a.scaleX))
                            throw std::runtime_error(name+": nonfinite timeline result");
                        if(!node.meshControlPoints.empty() && node.meshControlPoints.size()!=16)
                            throw std::runtime_error(name+": invalid evaluated legacy Bezier patch");
                    }
                    if(node.meshCombinator) {
                        const auto &axis=node.meshCombinator->axes().front();
                        studysteady::motion::updatePlayerCombinators(player,{{axis.key,axis.begin}});
                        player.evaluateTimelineForDifferentialTest_guess(node,times.front(),true);
                        if(!node.meshCombinator->allNeutral() && node.meshControlPoints.size()!=16)
                            throw std::runtime_error(name+": combined patch did not reach real MotionNode");
                        combinedSamples+=node.meshControlPoints.size()==16;
                        studysteady::motion::updatePlayerCombinators(player,{});
                        player.evaluateTimelineForDifferentialTest_guess(node,times.front(),true);
                        if(!node.meshControlPoints.empty())throw std::runtime_error(name+": combined patch not cleared at neutral");
                    }
                }
            });
            if(motions!=45 || nodes!=379 || crossfades==0)throw std::runtime_error("unexpected timeline coverage");
            std::cout<<"real Player timeline motions="<<motions<<" nodes="<<nodes<<" evaluations="<<evaluations<<" changed="<<changes<<" crossfade_samples="<<crossfades<<" legacy_mesh_samples="<<legacyPatches<<": PASS\n";
            if(combinators!=78 || combinedSamples==0)throw std::runtime_error("combined Player node coverage failed");
            std::cout<<"real MotionNode combinators="<<combinators<<" nonneutral_published="<<combinedSamples<<" neutral clears verified: PASS\n";
            std::cout<<"Original parse/merge/seek/evaluate forward+reverse; no hierarchy/source resolution or full frameProgress/rendering\n";
        }
        ncbAutoRegister::AllUnregist();engine->Shutdown();engine->Release();motionSetTjsEngine(nullptr);return 0;
    }catch(const eTJSError &e){std::cerr<<"TJS error: "<<e.GetMessage().AsStdString()<<'\n';return 1;}
     catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
}
