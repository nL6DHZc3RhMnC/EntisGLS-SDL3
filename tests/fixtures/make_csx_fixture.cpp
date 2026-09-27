// Generates an original, asset-free traditional Cotopha program. This is a
// real SDK execution image: Enter an empty main scope, then return normally.
// No bytes are read from a commercial game's script or archives.
#include "compatibility/sdk/legacy/gls.h"
#include "platform/sdl/system.h"
#include <SDL3/SDL.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <cstdio>

namespace {
class FixtureImage final : public ECSExecutionImage {
public:
    FixtureImage() {
        // Enter (4), empty inline name (uint32), zero arguments (uint32),
        // ExReturn (18), no return value (0). These are SDK instruction codes.
        const BYTE code[] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0};
        std::memset(&m_exiHeader, 0, sizeof(m_exiHeader));
        m_exiHeader.nVersion = 1;
        m_exiHeader.nIntBase = 64;
        m_exiHeader.nStackSize = 4096;
        m_exiHeader.nHeapSize = 4096;
        m_exiHeader.fnEntryPoint = 0;
        m_exiHeader.fnStaticInitialize = UINT32_MAX;
        m_exiHeader.fnResumePrepare = UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)), code, sizeof(code));
        m_bufImage.Flush(sizeof(code));
        m_pImage = static_cast<BYTE*>(m_bufImage.ModifyBuffer(0, sizeof(code)));
        m_dwImageSize = sizeof(code);
    }
};

void Check(ESLError error, const char* stage) {
    if (error) throw std::runtime_error(std::string(stage) + ": " + GetESLErrorMsg(error));
}

struct ScriptRuntime {
    ScriptRuntime() { ECotophaScript::Initialize(0); ECotophaScript::MultithreadReference(true); }
    ~ScriptRuntime() { ECotophaScript::Release(); }
};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: make_csx_fixture <output-directory>\n");
        return 2;
    }
    if (!SDL_Init(0)) return 2;
    bool initialized = false;
    int result = 0;
    try {
        const auto output = std::filesystem::absolute(argv[1]);
        std::filesystem::create_directories(output / "assets");
        std::filesystem::create_directories(output / "local/savedata");
        std::filesystem::create_directories(output / "storage");
        study::platform::sdl::SystemPaths paths;
        paths.assetsRoot = (output / "assets").string();
        paths.storageRoot = (output / "storage").string();
        paths.localRoot = (output / "local").string();
        paths.gameRoot = output.string();
        if (!study::platform::sdl::ConfigureSystemPaths(paths))
            throw std::runtime_error("cannot configure isolated fixture paths");
        SakuraGL::Initialize();
        initialized = true;
        {
            ScriptRuntime runtime;
            ECSEnvironment environment;
            if (environment.InitEnvironment()) throw std::runtime_error("cannot initialize fixture environment");
            FixtureImage image;
            image.AttachCSEnvironment(&environment);
            EMemoryFile bytes;
            Check(bytes.Create(4096), "Create fixture buffer");
            Check(image.WriteExecution(bytes), "WriteExecution");
            // Independently read the serialized EMC records back and execute
            // them, rather than accepting the in-memory source as validation.
            bytes.Seek(0, ESLFileObject::FromBegin);
            ECSExecutionImage readback;
            readback.AttachCSEnvironment(&environment);
            Check(readback.ReadExecution(bytes), "ReadExecution");
            if (readback.m_dwImageSize != 11 || readback.m_exiHeader.fnEntryPoint != 0)
                throw std::runtime_error("fixture execution image changed during serialization");
            ECSContext context;
            Check(context.InitializeContext(&readback), "InitializeContext");
            ECSObjArray<ECSObject> arguments;
            arguments.Add(new ECSString(L"fixture"));
            const auto executeError = context.CallFunction(readback.m_exiHeader.fnEntryPoint, arguments);
            context.ReleaseContext(true);
            Check(executeError, "Execute original empty main");

            std::ofstream script(output / "adventure.csx", std::ios::binary | std::ios::trunc);
            script.write(static_cast<const char*>(bytes.GetBuffer()), static_cast<std::streamsize>(bytes.GetLength()));
            script.close();
            if (!script) throw std::runtime_error("cannot write adventure.csx");
            std::ofstream config(output / "cotopha.xml", std::ios::binary | std::ios::trunc);
            config << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      "<script src=\"adventure.csx\">\n"
                      "  <save_dir path=\"local://savedata\"/>\n"
                      "  <file path=\"$(CURRENT)\"/>\n"
                      "  <display caption=\"Generic EntisGLS fixture\" width=\"320\" height=\"180\"/>\n"
                      "</script>\n";
            config.close();
            if (!config) throw std::runtime_error("cannot write cotopha.xml");
            std::printf("CSX fixture PASS: %lu serialized bytes, 11 instruction bytes, ReadExecution and main returned successfully; %s\n",
                        bytes.GetLength(), (output / "adventure.csx").string().c_str());
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "CSX fixture FAIL: %s\n", error.what());
        result = 1;
    }
    if (initialized) SakuraGL::Finalize();
    SDL_Quit();
    return result;
}
