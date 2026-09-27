#include "ncbind.hpp"
#include "Player.h"
#include "NodeTree.h"
#include "ResourceManager.h"
#include "extensions/emote/tjs_runtime/psb_storage.h"
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#define NCB_MODULE_NAME TJS_W("motion_cpu_probe.dll")
using motion::Player;
using motion::ResourceManager;
NCB_REGISTER_CLASS(ResourceManager) {
    NCB_CONSTRUCTOR(());
    NCB_METHOD(requireLayerId);
    NCB_METHOD(releaseLayerId);
    NCB_METHOD(random);
    NCB_METHOD(findMotion);
    NCB_METHOD(isExistMotion);
}
NCB_REGISTER_CLASS(Player) { NCB_CONSTRUCTOR((tTJSVariant)); }

int main(int argc,char **argv) {
    if(argc!=3 && argc!=4) { std::cerr << "Usage: motion_player_probe haz_a.psb header_seed [cycles]\n";return 2; }
    try {
        const int cycles = argc == 4 ? std::stoi(argv[3]) : 1;
        if(cycles < 1 || cycles > 1000) throw std::runtime_error("Cycles must be 1..1000");
        spdlog::stdout_color_mt("tjs2");spdlog::stdout_color_mt("core");spdlog::stdout_color_mt("plugin");
        auto *engine = new tTJS();motionSetTjsEngine(engine);
        ncbAutoRegister::AllRegist();
        if(!ncbAutoRegister::LoadModule(TJS_W("motion_cpu_probe.dll"))) throw std::runtime_error("CPU registrar load failed");
        motionSetPsbHeaderSeed(static_cast<uint32_t>(std::stoull(argv[2])));
        {
            auto *manager = new ResourceManager();
            auto *dispatch = ncbInstanceAdaptor<ResourceManager>::CreateAdaptor(manager);
            if(!dispatch) { delete manager;throw std::runtime_error("ResourceManager adaptor failed"); }
            tTJSVariant owner(dispatch,dispatch);dispatch->Release();
            tTJSVariant loaded = manager->load(ttstr(argv[1]));
            const ttstr path = TVPGetPlacedPath(ttstr(argv[1]));
            tTJSVariant found = manager->findMotion(tTJSVariant(path),TJS_W("motion/all_parts/全体構造"));
            if(found.Type()!=tvtObject) throw std::runtime_error("Real ResourceManager did not find motion");
            ncbPropAccessor pair(found);
            tTJSVariant content = pair.GetValue(0,ncbTypedefs::Tag<tTJSVariant>(),0);
            ncbPropAccessor motionContent(content);
            for(int cycle = 0; cycle < cycles; ++cycle) {
                Player player(owner);
                player.setProject(tTJSVariant(path));
                player.setMotionContentForDifferentialTest_guess(content);
                player.parseParameterListForDifferentialTest_guess(motionContent.GetValue(TJS_W("parameter"),ncbTypedefs::Tag<tTJSVariant>(),0));
                motion::detail::buildNodeTree(player,motionContent);
                size_t children = 0;
                for(const auto &node:player.nodes()) children += node.nodeType==3;
                if(player.parameterEntryCountForDifferentialTest_guess()!=3 || player.nodes().size()!=26 || children!=13)
                    throw std::runtime_error("Player node/parameter construction failed");
                if(cycle == 0) std::cout << "real Player nodes=" << player.nodes().size() << " child_players=" << children << " parameters=" << player.parameterEntryCountForDifferentialTest_guess() << '\n';
                for(size_t i=0;i<player.parameterEntryCountForDifferentialTest_guess();++i) {
                    const auto &parameter=player.parameterEntryForDifferentialTest_guess(i);
                    if(parameter.value!=30) throw std::runtime_error("Parameter initial normalization failed");
                    if(cycle == 0) std::cout << parameter.id.AsStdString() << "=" << parameter.value << '\n';
                }
            }
            std::cout << "real ResourceManager + Player + NCB node build: PASS (no rendering or timeline progress)\n";
            std::cout << "Player construction/destruction cycles=" << cycles << " child_players_per_cycle=13: PASS\n";
        }
        ncbAutoRegister::AllUnregist();engine->Shutdown();engine->Release();motionSetTjsEngine(nullptr);
        return 0;
    } catch(const eTJSError &e) {std::cerr << "TJS error: " << e.GetMessage().AsStdString() << '\n';return 1;}
      catch(const std::exception &e) {std::cerr << e.what() << '\n';return 1;}
}
