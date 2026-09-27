#include "tjs_host.h"
#include "tjsUtils.h"
#include "psbfile/PSBRawFile.h"
#include "psb_header.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc,char **argv) {
    if(argc != 3) { std::cerr << "Usage: motion_psb_tjs_probe file.psb header_seed\n"; return 2; }
    try {
        std::cerr << "PSB/TJS: startup\n";
        spdlog::stdout_color_mt("tjs2"); spdlog::stdout_color_mt("core");
        auto *engine = new tTJS(); motionSetTjsEngine(engine);
        {
            std::ifstream input(argv[1],std::ios::binary|std::ios::ate);
            if(!input) throw std::runtime_error("Cannot open PSB");
            const auto length = input.tellg();
            if(length<56 || length>256*1024*1024) throw std::runtime_error("PSB size out of range");
            std::vector<uint8_t> bytes(static_cast<size_t>(length));
            input.seekg(0);input.read(reinterpret_cast<char *>(bytes.data()),bytes.size());
            if(!input) throw std::runtime_error("PSB read failed");
            studysteady::decodePsbHeader(bytes,static_cast<uint32_t>(std::stoull(argv[2])));
            auto *data = static_cast<uint8_t *>(TJSAlignedAlloc(bytes.size(), 4));
            std::memcpy(data,bytes.data(),bytes.size());
            PSB::PSBFile file;
            if(!file.Adopt(data,bytes.size())) { TJSAlignedDealloc(data); throw std::runtime_error("PSB adoption failed"); }
            std::cerr << "PSB/TJS: Adopt/root ready\n";
            auto *dispatch = file.GetRootDispatch();
            tTJSVariant psb(dispatch,dispatch);dispatch->Release();
            auto *global = engine->GetGlobal();
            global->PropSet(TJS_MEMBERENSURE,TJS_W("psb"),nullptr,&psb,global);global->Release();
            tTJSVariant result;
            std::cerr << "PSB/TJS: evaluate metadata\n";
            engine->EvalExpression(TJS_W("psb.spec + ':' + psb.source.tex.texture.width + ':' + psb.metadata.base.chara"),&result);
            const ttstr summary(result);
            std::cout << "PSB real TJS dispatch=" << summary.AsStdString() << '\n';
            if(summary != TJS_W("win:4096:all_parts")) throw std::runtime_error("Unexpected haz_a metadata");
            std::cerr << "PSB/TJS: evaluate pixels\n";
            engine->EvalExpression(TJS_W("psb.source.tex.texture.pixel"),&result);
            if(result.Type()!=tvtOctet || result.AsOctetNoAddRef()->GetLength()!=67108864) throw std::runtime_error("PSB resource-to-Octet failed");
            std::cout << "PSB resource Octet bytes=" << result.AsOctetNoAddRef()->GetLength() << ": PASS\n";
            engine->EvalExpression(TJS_W("psb.object.body_parts.motion['下半身変形基礎'].layer[0].children[0].children[0].meshCombinator.combinatorList[0].rawMeshList"),&result);
            if(result.Type()!=tvtOctet || result.AsOctetNoAddRef()->GetLength()!=384) throw std::runtime_error("PSB v4 extra resource-to-Octet failed");
            std::cout << "PSB v4 mesh resource Octet bytes=" << result.AsOctetNoAddRef()->GetLength() << ": PASS\n";
            result.Clear();psb.Clear();
            engine->ExecScript(TJS_W("delete psb;"),nullptr);
        }
        engine->Shutdown();engine->Release();motionSetTjsEngine(nullptr);
        return 0;
    } catch(const eTJSError &e) { std::cerr << "TJS error: " << e.GetMessage().AsStdString() << '\n';return 1; }
      catch(const std::exception &e) { std::cerr << e.what() << '\n';return 1; }
}
