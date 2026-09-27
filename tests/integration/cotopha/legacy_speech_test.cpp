// Synthetic calls only: no commercial script, voice, or configuration data.
#include "compatibility/sdk/legacy/gls.h"
#include "compatibility/sdk/legacy/runtime_support.h"
#include "runtime/cotopha_port/legacy_speech.h"
#include "platform/sdl/system.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <stdexcept>

namespace {
void Require(bool value, const char* detail) {
    if (!value) throw std::runtime_error(detail);
}
std::unique_ptr<ECSObject> Invoke(ECSContext& context, ECSObject& receiver,
                                const wchar_t* name, std::initializer_list<ECSObject*> parameters = {}) {
    ECSObjArray<ECSObject> args;
    args.Add(new ECSReference(&receiver));
    for (auto* argument : parameters) args.Add(argument);
    int index = -1;
    Require(!receiver.GetFunction(context, index, name), "native method must resolve");
    Require(!receiver.CallFunction(context, index, args), "API status must be a return value, not a VM exception");
    std::unique_ptr<ECSObject> result(context.PopObject());
    Require(bool(result), "native method returns a value");
    return result;
}
INT64 Integer(std::unique_ptr<ECSObject> value) {
    INT64 result = 0;
    Require(value && !value->OperateInteger(result), "integer API result");
    return result;
}
double Real(std::unique_ptr<ECSObject> value) {
    double result = 0;
    Require(value && !value->OperateReal(result), "real API result");
    return result;
}
void EmptyConfig(ECSContext& context, ECSObject& generator) {
    auto value = Invoke(context, generator, L"GetConfig");
    auto* text = ESLTypeCast<ECSString>(value.get());
    Require(text && text->m_varStr.IsEmpty(), "missing backend exposes no fabricated configuration");
}
void Run() {
    struct Runtime {
        Runtime() { ECotophaScript::Initialize(0); }
        ~Runtime() { ECotophaScript::Release(); }
    } runtime;
    ECSContext context;
    std::unique_ptr<ECSObject> generator(LegacyCreatePlatformObject(context, L"SpeachVoiceGenerator"));
    std::unique_ptr<ECSObject> player(LegacyCreatePlatformObject(context, L"SpeachVoicePlayer"));
    Require(generator && player, "speech API objects can be constructed without a synthesizer");
    Require(generator->GetTypeOf(L"SpeachVoiceGenerator") == generator.get(), "generator native type");
    auto* resource = ESLTypeCast<ECSResource>(player.get());
    Require(resource && player->GetTypeOf(L"Resource") == player.get(), "player preserves Resource inheritance");
    Require(Integer(Invoke(context, *generator, L"Initialize")) == eslErrNotSupported,
            "initialization honestly reports unavailable backend");
    EmptyConfig(context, *generator);
    Require(Integer(Invoke(context, *generator, L"SetConfig", {new ECSString(L"<voice_speaker_config/>")})) == eslErrNotSupported,
            "configuration is not reported as applied");
    EmptyConfig(context, *generator);
    Require(Integer(Invoke(context, *generator, L"IsSpeakerMute", {new ECSInteger(0)})) != 0,
            "unavailable speaker prevents optional synthesis branch");
    Require(Real(Invoke(context, *generator, L"GetSpeakerVolume", {new ECSInteger(0)})) == 0,
            "unavailable speaker has no output gain");
    Require(Integer(Invoke(context, *generator, L"SetSpeakerMute", {new ECSInteger(0), new ECSInteger(0)})) == eslErrNotSupported,
            "unmuting cannot enable an absent synthesizer");
    Require(Integer(Invoke(context, *generator, L"SetSpeakerVolume", {new ECSInteger(0), new ECSReal(1)})) == eslErrNotSupported,
            "gain change does not pretend to succeed");
    Require(Integer(Invoke(context, *generator, L"IsSpeakerMute", {new ECSInteger(0)})) != 0,
            "unsupported updates preserve unavailable state");
    Require(Integer(Invoke(context, *generator, L"DoConfigDialog", {new ECSReference})) == eslErrNotSupported,
            "missing speech settings dialog reports unsupported");
    Require(Integer(Invoke(context, *player, L"AttachVoiceGenerator", {new ECSReference(generator.get())})) == eslErrNotSupported,
            "attaching unavailable generator does not report success");
    Require(Integer(Invoke(context, *player, L"MakeVoice", {new ECSString(L"Synthetic fixture"), new ECSInteger(0)})) == eslErrNotSupported,
            "voice generation returns unsupported");
    Require(resource->GetSound() == nullptr && !resource->IsPlaying(), "failed generation creates no silent or fake voice");
    Require(Integer(Invoke(context, *player, L"SetVolume", {new ECSReal(0.25), new ECSReal(0.5)})) == eslErrSuccess,
            "inherited Resource volume controls remain functional");
    Require(Integer(Invoke(context, *player, L"IsPlaying")) == 0, "inherited Resource state query remains functional");
    std::unique_ptr<ECSObject> duplicate(player->Duplicate());
    Require(duplicate && duplicate->GetTypeOf(L"SpeachVoicePlayer") == duplicate.get() &&
            duplicate->GetTypeOf(L"Resource") == duplicate.get(), "duplication preserves derived native type");
    Require(Integer(Invoke(context, *generator, L"Release")) == eslErrSuccess, "releasing an empty backend is idempotent");
    int index = -1;
    Require(generator->GetFunction(context, index, L"InventedMethod") != eslErrSuccess && index == -1,
            "unknown methods are rejected");
    Require(!generator->GetFunction(context, index, L"SetSpeakerVolume"), "resolve argument validation target");
    ECSObjArray<ECSObject> invalidArgs;
    invalidArgs.Add(new ECSReference(generator.get()));
    Require(generator->CallFunction(context, index, invalidArgs) != eslErrSuccess,
            "invalid argument count remains a VM error");
}
}

int main() {
    if (!SDL_Init(0)) return 2;
    bool initialized = false;
    bool createdRoot = false;
    std::filesystem::path root;
    int result = 0;
    try {
        namespace fs = std::filesystem;
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        root = fs::temp_directory_path() / ("entis-speech-test-" + unique);
        createdRoot = fs::create_directory(root);
        Require(createdRoot, "cannot create isolated fixture directory");
        for (const char* name : {"assets", "storage", "local", "game"}) fs::create_directory(root / name);
        study::platform::sdl::SystemPaths paths;
        paths.assetsRoot = (root / "assets").string();
        paths.storageRoot = (root / "storage").string();
        paths.localRoot = (root / "local").string();
        paths.gameRoot = (root / "game").string();
        Require(study::platform::sdl::ConfigureSystemPaths(paths), "isolated fixture paths rejected");
        SakuraGL::Initialize();
        initialized = true;
        Run();
        std::puts("Legacy speech API PASS: unavailable backend, strict return codes, optional muted state, Resource inheritance");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Legacy speech API FAIL: %s\n", error.what());
        result = 1;
    }
    if (initialized) SakuraGL::Finalize();
    SDL_Quit();
    if (createdRoot) {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }
    return result;
}
