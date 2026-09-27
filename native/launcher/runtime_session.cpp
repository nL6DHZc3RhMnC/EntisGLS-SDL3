#include "compatibility/sdk/legacy/gls.h"
#include "launcher/runtime_session.h"
#if defined(ENTISGLS_BUILD_DIAGNOSTICS)
#include "../../tests/probes/cotopha/legacy_file_probe.h"
#include "../../tests/probes/cotopha/legacy_core_probe.h"
#include "../../tests/probes/cotopha/legacy_media_probe.h"
#include "../../tests/probes/cotopha/legacy_input_probe.h"
#include "../../tests/probes/cotopha/legacy_window_probe.h"
#include "../../tests/probes/cotopha/legacy_movie_window_probe.h"
#include "../../tests/probes/cotopha/legacy_setup_probe.h"
#include "../../tests/probes/cotopha/legacy_volume_envelope_probe.h"
#include "../../tests/probes/cotopha/legacy_resource_state_probe.h"
#include "../../tests/probes/cotopha/legacy_atomic_save_probe.h"
#include "../../tests/probes/cotopha/legacy_sprite_state_probe.h"
#include "../../tests/probes/cotopha/legacy_image_export_probe.h"
#include "../../tests/probes/cotopha/legacy_heap_probe.h"
#include "../../tests/probes/cotopha/legacy_thread_state_probe.h"
#include "../../tests/probes/cotopha/legacy_primary_context_probe.h"
#endif
#include "runtime/cotopha_port/legacy_window.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_sprite_dynamic.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_super_raster.h"
#include "runtime/cotopha_port/legacy_super_shading.h"
#include "platform/log.h"
#include "extensions/emote/psb/psb_key_runtime.h"
#include "extensions/emote/psb/psb_key_resolver.h"
#include "launcher/psb_key_settings.h"
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include "platform/sdl/game_font_aliases.h"
#include "platform/sdl/opentype_font.h"
#include "launcher/game_config.h"
#include "compatibility/games/compatibility_profiles.h"
#include "runtime/cotopha_port/legacy_file.h"
#endif
#include <memory>
#include <mutex>
#include <atomic>

namespace {
class GameContext final : public ECSContext {
public:
    std::atomic<bool> cancelled{false};
    ESLError ExecuteInstruction() override {
        if (cancelled.load(std::memory_order_relaxed) || entis::launcher::HasGamePsbKeyError()) return eslErrAbort;
        return ECSContext::ExecuteInstruction();
    }
};
std::mutex activeMutex;
GameContext *activeContext = nullptr;
bool abortRequested = false;
std::string lastError;
int Fail(int code, const std::string& message) {
    lastError = message;
    study::platform::LogWrite(study::platform::LogPriority::Error, "EntisGLS", message.c_str());
    return code;
}
#if defined(STUDYSTEADY_PLATFORM_SDL3)
std::wstring Wide(const std::string& text) {
    SSystem::SString value;
    value.FromUTF8(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    return std::wstring(static_cast<const wchar_t*>(value));
}
#endif

struct RuntimeScope {
    RuntimeScope() {
        ECotophaScript::Initialize(0);
        ECotophaScript::MultithreadReference(true);
    }
    ~RuntimeScope() { ECotophaScript::Release(); }
};

void LogError(const char *stage, ESLError error, const ECSContext *context = nullptr) {
    const char *message = GetESLErrorMsg(error);
    lastError = std::string(stage) + ": " + (message ? message : "unknown runtime error");
    study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Legacy %s failed: %s (ip=0x%08x)",
        stage, message ? message : "unknown error", context ? context->m_ip : 0);
}

struct PublishedContext {
    explicit PublishedContext(GameContext &context) {
        std::lock_guard<std::mutex> guard(activeMutex);
        if (abortRequested) context.cancelled = true;
        activeContext = &context;
    }
    ~PublishedContext() {
        std::lock_guard<std::mutex> guard(activeMutex);
        activeContext = nullptr;
        abortRequested = false;
    }
};
}

std::string LegacyGameLastError() { return lastError; }

int RunLegacyGame(const wchar_t *arguments, const entis::launcher::GameLaunchConfig* launch) {
    lastError.clear();
    RuntimeScope runtime;
    ECSEnvironment environment;
    study::platform::LogWrite(study::platform::LogPriority::Info, "EntisGLS", "Starting traditional Cotopha runtime");
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    if (!launch) return Fail(1, "Missing resolved game configuration");
    if (!study::launcher::ConfigureCompatibility(launch->compatibilityProfile))
        return Fail(1, "Unknown compatibility profile: " + launch->compatibilityProfile);
    if (arguments && *arguments && launch->compatibilityProfile != "study-steady-r18")
        return Fail(2, "These diagnostic probes require the study-steady-r18 compatibility profile");
    SSystem::SMemoryReferenceFile config;
    config.AttachMemory(const_cast<char*>(launch->normalizedXml.data()), launch->normalizedXml.size());
    if (environment.LoadEnvironment(config)) return Fail(1, "Could not load game environment: " + launch->configSource);
    using namespace study::platform::sdl;
    for (const auto& font : launch->openTypeFonts) {
        const auto registered = RegisterOpenTypeFont(Wide(font.path), {},
            [&](const wchar_t* path) { return environment.NewOpenFile(path, SSystem::SFileOpener::shareRead); }, Wide(font.family));
        if (!registered.loaded) return Fail(38, "Cannot load font " + font.path + ": " + registered.error);
    }
    for (const auto& alias : launch->fontAliases)
        if (!RegisterFontAlias(Wide(alias.alias), Wide(alias.source)))
            return Fail(37, "Cannot register font alias " + alias.alias + " -> " + alias.source);
    if (!study::launcher::ApplyCompatibilityFonts(launch->compatibilityProfile))
        return Fail(37, "Compatibility font setup failed: " + launch->compatibilityProfile);
#else
    SSystem::SString external, local;
    if (SSystem::SFile::GetDefaultDirectory(external, SSystem::SFile::DefaultDirectory::AndroidExternalStoragePrivate) ||
        SSystem::SFile::GetDefaultDirectory(local, SSystem::SFile::DefaultDirectory::AndroidLocalFiles))
        return Fail(1, "Cannot resolve per-game PSB configuration directories");
    const auto externalUtf8 = external.ToUTF8(), localUtf8 = local.ToUTF8();
    const auto gameDir = std::filesystem::u8path(reinterpret_cast<const char*>(externalUtf8.GetConstArray())) / "game";
    const auto dataDir = std::filesystem::u8path(reinterpret_cast<const char*>(localUtf8.GetConstArray()));
    entis::launcher::SetGamePsbKeyResolver(std::make_shared<entis::launcher::PsbKeyResolver>(
        gameDir, dataDir, entis::launcher::ReadPsbKeyOverride(dataDir)));
    SSystem::SSmartPointer<SSystem::SFileInterface> config =
        SSystem::SFileOpener::DefaultNewOpenFile(L"assets://cotopha.xml", SSystem::SFileOpener::shareRead);
    if (config == nullptr || environment.LoadEnvironment(*config))
        return Fail(1, "Legacy environment load failed");
#endif
#if defined(ENTISGLS_BUILD_DIAGNOSTICS)
    if (arguments && !std::wcscmp(arguments, L"--self-test")) {
        if (!CheckLegacyFileBridge(environment)) {
            study::platform::LogWrite(study::platform::LogPriority::Error, "StudySteady", "Legacy file bridge verification failed");
            return 2;
        }
        // Each probe owns and releases its temporary context. Keep collecting
        // independent failures so a graphics fixture does not hide a media or
        // persistence regression; the aggregate result still fails the run.
        int failed=0, failureCount=0;
        auto record=[&](const char* name,int code,bool passed){
            if(passed)return;
            if(!failed)failed=code;
            ++failureCount;
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Standalone subsystem failed: %s",name);
        };
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        record("CheckCompatibilityFonts", 37, study::launcher::CheckCompatibilityFonts(launch->compatibilityProfile));
#endif
        record("CheckLegacyCore", 7, CheckLegacyCore(environment));
        record("CheckLegacyHeapState", 29, CheckLegacyHeapState(environment));
        record("CheckLegacyThreadState", 30, CheckLegacyThreadState(environment));
        record("CheckLegacyPrimaryReferenceState", 31, CheckLegacyPrimaryReferenceState(environment));
        record("CheckLegacyContextLoadFailure", 32, CheckLegacyContextLoadFailure(environment));
        record("CheckLegacyMedia", 8, CheckLegacyMedia(environment));
        record("CheckLegacyInput", 9, CheckLegacyInput(environment));
        record("CheckLegacyWindowCommandQueue", 25, CheckLegacyWindowCommandQueue());
        record("CheckLegacySetup", 11, CheckLegacySetup());
        record("CheckLegacyMessage", 12, CheckLegacyMessage());
        record("CheckLegacySpriteDraw", 15, CheckLegacySpriteDraw());
        record("CheckLegacySpriteDynamic", 35, CheckLegacySpriteDynamic());
        record("CheckLegacySuperSprite", 22, CheckLegacySuperSprite(environment));
        record("CheckLegacySuperRaster", 33, CheckLegacySuperRaster());
        record("CheckLegacySuperShading", 34, CheckLegacySuperShading());
        record("CheckLegacyTone", 28, CheckLegacyTone());
        record("CheckLegacyParticleState", 27, CheckLegacyParticleState(environment));
        record("CheckLegacySpriteCallbacks", 18, CheckLegacySpriteCallbacks());
        record("CheckLegacyCompiler", 13, CheckLegacyCompiler(environment));
        record("CheckLegacyMotionOwner", 14, CheckLegacyMotionOwner(environment));
        record("CheckLegacyAudioPlayer", 16, CheckLegacyAudioPlayer(environment));
        record("CheckLegacyMovie", 17, CheckLegacyMovie(environment));
        record("CheckLegacyVolumeEnvelope", 19, CheckLegacyVolumeEnvelope(environment));
        record("CheckLegacyResourceState", 21, CheckLegacyResourceState(environment));
        record("CheckLegacyAtomicSave", 23, CheckLegacyAtomicSave(environment));
        record("CheckLegacySpriteState", 24, CheckLegacySpriteState(environment));
        if(failed){
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy standalone self-tests FAIL: %d subsystems",failureCount);
            return failed;
        }
        study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Legacy standalone self-tests PASS");
        return 0;
    }
    if (arguments && !std::wcscmp(arguments, L"--image-export-probe"))
        return CheckLegacyImageExport(environment) ? 0 : 26;

#else
    if (arguments && (!std::wcscmp(arguments, L"--self-test") ||
        !std::wcscmp(arguments, L"--image-export-probe") || !std::wcscmp(arguments, L"--opening-probe") ||
        !std::wcscmp(arguments, L"--opening-full-probe") || !std::wcscmp(arguments, L"--emote-probe") ||
        !std::wcscmp(arguments, L"--window-probe")))
        return Fail(2, "Diagnostic probes are disabled; build with ENTISGLS_BUILD_DIAGNOSTICS=ON");
#endif

#if defined(STUDYSTEADY_PLATFORM_SDL3)
    auto* file = environment.NewOpenFile(Wide(launch->entryScript).c_str(), SSystem::SFileOpener::shareRead);
    std::unique_ptr<ESLFileObject> script(file ? new LegacyFileAdapter(file, ESLFileObject::modeRead) : nullptr);
    if (!script) return Fail(3, "Entry script not found: " + launch->entryScript);
#else
    std::unique_ptr<ESLFileObject> script(environment.OpenFileObject("script.csx"));
    if (!script) return Fail(3, "Entry script not found: script.csx");
#endif
    ECSExecutionImage image;
    image.AttachCSEnvironment(&environment);
    ESLError error = image.ReadExecution(*script);
    if (error) { LogError("ReadExecution", error); return 4; }
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady",
        "Legacy CSX loaded: image=%u classes=%u strings=%u entry=0x%08x",
        image.m_dwImageSize, static_cast<unsigned>(image.GetClassInfoCount()),
        static_cast<unsigned>(image.m_lstConstStr.GetSize()), image.m_exiHeader.fnEntryPoint);

    GameContext context;
#if defined(ENTISGLS_BUILD_DIAGNOSTICS)
    if (arguments && (!std::wcscmp(arguments, L"--opening-probe") ||
                      !std::wcscmp(arguments, L"--opening-full-probe"))) {
        context.m_pcsxi = &image;
        PublishedContext published(context);
        const bool success = CheckLegacyOpeningWindow(context, environment,
            !std::wcscmp(arguments, L"--opening-full-probe"));
        ShutdownLegacySpriteCallbacksForImage(image);
        context.m_pcsxi = nullptr;
        return success ? 0 : 36;
    }
    if (arguments && !std::wcscmp(arguments, L"--emote-probe")) {
        context.m_pcsxi = &image;
        PublishedContext published(context);
        const bool success = CheckLegacyEmoteWindow(context, environment);
        ShutdownLegacySpriteCallbacksForImage(image);
        context.m_pcsxi = nullptr;
        return success ? 0 : 20;
    }
    if (arguments && !std::wcscmp(arguments, L"--window-probe")) {
        context.m_pcsxi = &image;
        PublishedContext published(context);
        const bool success = CheckLegacyWindow(context);
        context.m_pcsxi = nullptr;
        return success ? 0 : 10;
    }
#endif
    PublishedContext published(context);
    struct CallbackScope {
        ECSExecutionImage& image;
        ~CallbackScope() { ShutdownLegacySpriteCallbacksForImage(image); }
    } callbackScope{image};
    error = context.InitializeContext(&image);
    if (error) { LogError("InitializeContext", error, &context); return 5; }
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Legacy globals and object/naked prologues complete; entering original main");
    {
        ECSObjArray<ECSObject> args;
        args.Add(new ECSString(arguments ? arguments : L""));
        if (image.m_exiHeader.fnStaticInitialize != UINT32_MAX) {
            ECSObjArray<ECSObject> noArguments;
            error = context.CallFunction(image.m_exiHeader.fnStaticInitialize, noArguments);
        }
        if (!error) error = context.CallFunction(image.m_exiHeader.fnEntryPoint, args);
        if (error) LogError("main", error, &context);
        else study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Legacy main returned without uncaught error (ip=0x%08x)", context.m_ip);
    }
    ShutdownLegacySpriteCallbacksForImage(image);
    context.ReleaseContext(true);
    return error ? 6 : 0;
}

void AbortLegacyGame() {
    std::lock_guard<std::mutex> guard(activeMutex);
    abortRequested = true;
    if (activeContext) {
        activeContext->cancelled = true;
        activeContext->SetStatus(ECSSakura2Processor::Context::xsInterrupt);
        activeContext->AbortWatingEvent();
    }
}
