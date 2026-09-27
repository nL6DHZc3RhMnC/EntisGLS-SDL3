#include "legacy_compat/gls.h"
#include "legacy_emote.h"
#include "legacy_motion_graphics.h"
#include "legacy_sprite_callbacks.h"
#include "legacy_audio_player.h"
#include "legacy_save_io.h"
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include "motion_bridge/tjs_runtime/motion_apk_runtime.h"
#include "platform/log.h"
#include "launcher/psb_key_resolver.h"
#include "launcher/psb_key_runtime.h"
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include "platform/sdl/main_thread.h"
#endif
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <deque>
#include <future>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <map>
#include <cstdio>
#include <vector>
#include <cmath>
#include <algorithm>

namespace {
struct MotionGraphicsGuard {
    MotionGraphicsGuard() { SSystem::Lock(); }
    ~MotionGraphicsGuard() { SSystem::Unlock(); }
};
std::string MotionUTF8(const wchar_t* text) {
    if (!text || !*text) return {};
    const auto bytes = SSystem::SString(text).ToUTF8();
    return {reinterpret_cast<const char*>(bytes.GetConstArray()), bytes.GetLength()};
}
std::wstring MotionWide(const std::string& text) {
    if (text.empty()) return {};
    SSystem::SString wide;
    wide.FromUTF8(reinterpret_cast<const uint8_t*>(text.c_str()));
    return static_cast<const wchar_t*>(wide);
}
const wchar_t* motionMethods[] = {L"LoadPlayer", L"ReleasePlayer", L"SetScreenSize", L"SetScale",
    L"SetCoord", L"PlayTimeline", L"PostTimeline", L"IsPlayingTimeline", L"AttachVoiceSync"};
#if defined(STUDYSTEADY_PLATFORM_SDL3)
template<class Function> auto RunMotionOnMain(Function&& function) -> std::invoke_result_t<Function> {
    if (SDL_IsMainThread()) return std::forward<Function>(function)();
    // LegacyNativeWindow explicitly assigns g_mutexGlobal as its UI mutex.
    // Releasing this count therefore covers both script and WindowUI ownership,
    // without keeping a raw SDK window pointer alive across logical window close.
    const auto systemLocks = SSystem::UnlockAll();
    struct RelockCaller {
        atomic_int_t systemLocks;
        ~RelockCaller() { SSystem::Relock(systemLocks); }
    } relock{systemLocks};
    std::packaged_task<std::invoke_result_t<Function>()> task(std::forward<Function>(function));
    auto result = task.get_future();
    for (;;) {
        const bool completed = study::platform::sdl::RunOnMainThreadSync([&] {
            // Other workers can be waiting on unrelated queued SDL requests.
            // Never block the dispatcher on locks held by one of those workers.
            if (SSystem::Lock(0) != SSystem::errSuccess) return false;
            struct SystemGuard { ~SystemGuard() { SSystem::Unlock(); } } systemGuard;
            auto* currentWindow = SakuraGL::SGLAbstractWindow::GetDefaultWindow();
            if (currentWindow && currentWindow->Lock(0) != SSystem::errSuccess) return false;
            struct WindowGuard {
                SakuraGL::SGLAbstractWindow* window;
                ~WindowGuard() { if (window) window->Unlock(); }
            } windowGuard{currentWindow};
            task();
            return true;
        });
        if (completed) return result.get();
        // Delay only this requesting worker; SDL's main loop remains responsive.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
#endif
}

// Cotopha threads may request projects concurrently. TJS/NCB values remain on
// one owner: the SDL main thread, or the original Android EGL worker. This also
// applies to initialization, callbacks and destruction.
class LegacyMotionService {
public:
    explicit LegacyMotionService(ECSEnvironment& environment) : environment_(environment), psbResolver_(entis::launcher::GetGamePsbKeyResolver()) {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        runtime_ = RunMotionOnMain([this] {
            char error[512] = {};
            auto* runtime = study_motion_create(error, sizeof(error));
            if (!runtime) throw std::runtime_error(error);
            if (!study_motion_set_reader(runtime, Read, Free, &environment_) ||
                !study_motion_set_psb_key_resolver(runtime, ResolvePsbKey, this)) {
                const std::string message = study_motion_last_error(runtime);
                study_motion_destroy(runtime, error, sizeof(error));
                throw std::runtime_error(message);
            }
            return runtime;
        });
#else
        std::promise<std::string> ready;
        auto result = ready.get_future();
        thread_ = std::thread([this, ready = std::move(ready)]() mutable {
            char error[512] = {};
            StudyMotionRuntime* runtime = study_motion_create(error, sizeof(error));
            if (!runtime) { ready.set_value(error); return; }
            if (!study_motion_set_reader(runtime, Read, Free, &environment_) ||
                !study_motion_set_psb_key_resolver(runtime, ResolvePsbKey, this)) {
                const std::string message = study_motion_last_error(runtime);
                study_motion_destroy(runtime, error, sizeof(error));
                ready.set_value(message); return;
            }
            ready.set_value({});
            for (;;) {
                std::function<void(StudyMotionRuntime*)> task;
                {
                    std::unique_lock<std::mutex> guard(mutex_);
                    changed_.wait(guard, [this] { return stopping_ || !queue_.empty(); });
                    if (queue_.empty() && stopping_) break;
                    task = std::move(queue_.front()); queue_.pop_front();
                }
                task(runtime);
            }
            if (!study_motion_destroy(runtime, error, sizeof(error)))
                study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion owner release: %s", error);
        });
        const auto error = result.get();
        if (!error.empty()) { thread_.join(); throw std::runtime_error(error); }
#endif
    }
    ~LegacyMotionService() {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        try {
            RunMotionOnMain([this] {
                char error[512] = {};
                if (!study_motion_destroy(runtime_, error, sizeof(error))) throw std::runtime_error(error);
                runtime_ = nullptr;
            });
        } catch (const std::exception& error) {
            study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion owner release: %s", error.what());
        }
#else
        { std::lock_guard<std::mutex> guard(mutex_); stopping_ = true; }
        changed_.notify_one();
        thread_.join();
#endif
    }
    template<class Fn> auto Call(Fn fn) -> decltype(fn(static_cast<StudyMotionRuntime*>(nullptr))) {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        return RunMotionOnMain([&] { return fn(runtime_); });
#else
        using Result = decltype(fn(static_cast<StudyMotionRuntime*>(nullptr)));
        auto task = std::make_shared<std::packaged_task<Result(StudyMotionRuntime*)>>(std::move(fn));
        auto result = task->get_future();
        { std::lock_guard<std::mutex> guard(mutex_); queue_.emplace_back([task](StudyMotionRuntime* r) { (*task)(r); }); }
        changed_.notify_one();
        return result.get();
#endif
    }
    static int ResolvePsbKey(void* user, const char* path, const void* bytes, size_t size,
                             uint32_t* key, char* error, size_t capacity) {
        const auto started = std::chrono::steady_clock::now();
        auto& owner = *static_cast<LegacyMotionService*>(user);
        try {
            if (!owner.psbResolver_)
                throw std::runtime_error("No game PSB key resolver is configured. Supply the original E-mote driver or a per-game PSB key.");
            *key = owner.psbResolver_->Resolve(static_cast<const uint8_t*>(bytes), size);
            study::platform::LogPrint(study::platform::LogPriority::Info, "EntisGLS",
                "PSB key verified for %s; source=%s resolve_ms=%.1f", path,
                owner.psbResolver_->LastSource().c_str(),
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count());
            if (!owner.psbResolver_->LastWarning().empty())
                study::platform::LogWrite(study::platform::LogPriority::Warn, "EntisGLS", owner.psbResolver_->LastWarning().c_str());
            return 1;
        } catch (const std::exception& failure) {
            const std::string message = std::string("PSB ") + (path ? path : "") + ": " + failure.what();
            study::platform::LogPrint(study::platform::LogPriority::Error, "EntisGLS",
                "PSB key resolution failed: resolve_ms=%.1f; %s",
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count(),
                message.c_str());
            entis::launcher::ReportGamePsbKeyError(message);
            if (error && capacity) { std::strncpy(error, message.c_str(), capacity - 1); error[capacity - 1] = 0; }
            return 0;
        }
    }
    uint32_t Capabilities() {
        return Call([](StudyMotionRuntime* r) { StudyMotionStats stats{};
            return study_motion_stats(r, &stats) ? stats.capabilities : 0; });
    }
    bool InitializeGLES(const LegacyMotionGLParent& parent) {
        return Call([this, parent](StudyMotionRuntime* r) {
            if (glParent_.context) return glParent_.context == parent.context && glParent_.display == parent.display;
            if (!study_motion_initialize_gles(r, parent.display, parent.context)) {
                study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion initialize GLES: %s", study_motion_last_error(r));
                return false;
            }
            glParent_ = parent;
            return true;
        });
    }
    uint64_t Load(const wchar_t* name) {
        const auto utf8 = SSystem::SString(name).ToUTF8();
        const std::string path(reinterpret_cast<const char*>(utf8.GetConstArray()), utf8.GetLength());
        return Call([this, path](StudyMotionRuntime* r) {
            // The callback resolves every encrypted load; this value is unused
            // for encrypted input and is not a fallback for missing metadata.
            const auto id = study_motion_load_project(r, path.c_str(), 0);
            if (!id) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion project: %s", study_motion_last_error(r));
            else ++projectReferences_[id];
            return id;
        });
    }
    bool Base(uint64_t id, std::string& chara, std::string& motion) {
        return Call([&](StudyMotionRuntime* r) { char c[256], m[256];
            if (!study_motion_project_base(r, id, c, sizeof(c), m, sizeof(m))) return false;
            chara = c; motion = m; return true; });
    }
    void Unload(uint64_t id) {
        if (id) Call([this, id](StudyMotionRuntime* r) {
            auto found = projectReferences_.find(id);
            if (found == projectReferences_.end()) return 0;
            if (--found->second) return 1;
            projectReferences_.erase(found);
            return study_motion_unload_project(r, id);
        });
    }
    bool Retain(uint64_t id) {
        return Call([this, id](StudyMotionRuntime*) {
            auto found = projectReferences_.find(id);
            if (found == projectReferences_.end()) return false;
            ++found->second;
            return true;
        });
    }
    static std::shared_ptr<LegacyMotionService> Acquire(ECSEnvironment& environment) {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        // The weak owner and its initialization are serialized by the SDL main
        // thread. Do not hold a worker mutex while waiting for a main callback.
        return RunMotionOnMain([&] {
            static std::weak_ptr<LegacyMotionService> owner;
            auto service = owner.lock();
            if (!service) { service = std::make_shared<LegacyMotionService>(environment); owner = service; }
            else if (&service->environment_ != &environment) throw std::runtime_error("Motion owner belongs to another game environment");
            return service;
        });
#else
        static std::mutex mutex;
        static std::weak_ptr<LegacyMotionService> owner;
        std::lock_guard<std::mutex> guard(mutex);
        auto service = owner.lock();
        if (!service) { service = std::make_shared<LegacyMotionService>(environment); owner = service; }
        else if (&service->environment_ != &environment) throw std::runtime_error("Motion owner belongs to another game environment");
        return service;
#endif
    }
private:
    static int Read(void* user, const char* path, void** bytes, size_t* length, char* error, size_t capacity) {
        try {
            SSystem::SString wide;
            wide.FromUTF8(reinterpret_cast<const uint8_t*>(path));
            const EString fileName(static_cast<const wchar_t*>(wide));
            std::unique_ptr<ESLFileObject> file(static_cast<ECSEnvironment*>(user)->OpenFileObject(fileName));
            if (!file) return 0;
            const auto size = file->GetLength();
            if (size < 56 || size > 256 * 1024 * 1024) throw std::runtime_error("NOA PSB size outside supported range");
            *length = static_cast<size_t>(size);
            if (!bytes) return 1;
            *bytes = std::malloc(*length);
            if (!*bytes) throw std::bad_alloc();
            if (file->Read(*bytes, *length) != *length) throw std::runtime_error("NOA PSB read truncated");
            return 1;
        } catch (const std::exception& e) { if (error && capacity) std::snprintf(error, capacity, "%s", e.what()); }
        catch (...) { if (error && capacity) std::snprintf(error, capacity, "NOA reader failure"); }
        return -1;
    }
    static void Free(void*, void* bytes, size_t) { std::free(bytes); }
    ECSEnvironment& environment_;
    std::shared_ptr<entis::launcher::PsbKeyResolver> psbResolver_;
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    StudyMotionRuntime* runtime_ = nullptr;
#else
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable changed_;
    std::deque<std::function<void(StudyMotionRuntime*)>> queue_;
    bool stopping_ = false;
#endif
    std::map<uint64_t, unsigned> projectReferences_;
    LegacyMotionGLParent glParent_;
};

IMPLEMENT_CLASS_INFO(ECSEmoteDevice, ECSObject)
ECSEmoteDevice::ECSEmoteDevice(ECSEnvironment& environment)
    : environment_(environment), service_(LegacyMotionService::Acquire(environment)) { m_vtType = csvtObject; }
ECSEmoteDevice::~ECSEmoteDevice() { window_.SetReference(nullptr, nullptr); }
const wchar_t* ECSEmoteDevice::GetTypeName() const { return L"EmoteDevice"; }
ECSObject* ECSEmoteDevice::GetTypeOf(const wchar_t* name) {
    return !EWideString::Compare(name, L"EmoteDevice") ? this : ECSObject::GetTypeOf(name);
}
ECSObject* ECSEmoteDevice::Duplicate() {
    auto* copy = new ECSEmoteDevice(environment_);
    copy->window_.SetReference(window_.m_pRef, nullptr);
    return copy;
}
ESLError ECSEmoteDevice::Move(ECSContext& context, ECSObject* object) {
    auto* source = ESLTypeCast<ECSEmoteDevice>(ECSObject::GetEntity(object));
    if (!source || &source->environment_ != &environment_) return eslErrInvalidParam;
    service_ = source->service_;
    window_.SetReference(source->window_.m_pRef, &context);
    context.delete_CSObject(object);
    return eslErrSuccess;
}
ESLError ECSEmoteDevice::UnaryOperate(ECSContext&, CSUnaryOperatorType) { return eslErrNotSupported; }
ESLError ECSEmoteDevice::Operate(ECSContext&, CSOperatorType, ECSObject*) { return eslErrNotSupported; }
ESLError ECSEmoteDevice::Compare(ECSContext&, int&, CSCompareType, ECSObject&) { return eslErrNotSupported; }
ESLError ECSEmoteDevice::GetFunction(ECSContext&, int& index, const wchar_t* name) {
    if (!EWideString::Compare(name, L"Initialize")) index = 0;
    else if (!EWideString::Compare(name, L"Release")) index = 1;
    else return ESLErrorMsg("Unknown EmoteDevice method");
    return eslErrSuccess;
}
ESLError ECSEmoteDevice::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    ESLError error = context.VerifyArgumentCount(args, index == 0 ? 2 : 1);
    if (error) return error;
    if (index == 1) { window_.SetReference(nullptr, &context); return context.PushObject(context.new_CSInteger(0)); }
    if (index != 0) return ESLErrorMsg("Invalid EmoteDevice method index");
    auto* window = ESLTypeCast<ECSWindow>(context.GetArgumentObjectAs(args, 1, L"Window"));
    if (!window) return ESLErrorMsg("EmoteDevice.Initialize requires Window");
    window_.SetReference(window, &context);
    LegacyMotionGLParent parent;
    if (!CaptureLegacyMotionGLParent(*window, parent))
        return context.PushObject(context.new_CSInteger(eslErrNotSupported));
    if (!service_->InitializeGLES(parent))
        return context.PushObject(context.new_CSInteger(eslErrGeneral));
    const auto capabilities = service_->Capabilities();
    // A usable CPU owner is not a usable renderer. Preserve the API's error
    // return even if the original game elects to ignore it.
    if (!(capabilities & STUDY_MOTION_GLES_RENDERER)) {
        study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "EmoteDevice.Initialize: real CPU owner ready (caps=%u), renderer unavailable; returning unsupported", capabilities);
        return context.PushObject(context.new_CSInteger(eslErrNotSupported));
    }
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "EmoteDevice.Initialize: shared GL renderer ready (caps=%u)", capabilities);
    return context.PushObject(context.new_CSInteger(0));
}
ECSObject* ECSEmoteDevice::GetVariableAt(int index) { return index == -1 ? &window_ : ECSObject::GetVariableAt(index); }
void ECSEmoteDevice::IndexAllMember() {
    ECSObject::IndexAllMember(); window_.IndexAllMember(); window_.m_pParent = this; window_.m_nIndex = -1;
}
void ECSEmoteDevice::CleanupAllReference(ECSContext& context) { window_.CleanupAllReference(context); }
ESLError ECSEmoteDevice::CommitAllReference(ECSContext& context) { return window_.CommitAllReference(context); }
ESLError ECSEmoteDevice::Save(ESLFileObject&, ECSContext&) { return ESLErrorMsg("EmoteDevice has no serialized object body; save its persistent device reference"); }
ESLError ECSEmoteDevice::Load(ESLFileObject&, ECSContext&) { return ESLErrorMsg("EmoteDevice has no serialized object body; restore its persistent device reference"); }

IMPLEMENT_CLASS_INFO(ECSEmoteSprite, ECSSprite)
class ECSEmoteSprite::MotionNative final : public LegacyCallbackSprite<SakuraGL::SGLSpriteFormed> {
public:
    explicit MotionNative(ECSEmoteSprite& owner) : owner_(owner) {}
    void AdvanceTime(uint32_t milliseconds) override {
        owner_.AdvanceMotion(milliseconds);
        LegacyCallbackSprite::AdvanceTime(milliseconds);
    }
    void PrepareDrawFrame() override {
        owner_.DrawMotion();
        LegacyCallbackSprite::PrepareDrawFrame();
    }
private:
    ECSEmoteSprite& owner_;
};
ECSEmoteSprite::ECSEmoteSprite() : ECSSprite(new MotionNative(*this)) {}
ECSEmoteSprite::~ECSEmoteSprite() { Release(); }
const wchar_t* ECSEmoteSprite::GetTypeName() const { return L"EmoteSprite"; }
ECSObject* ECSEmoteSprite::GetTypeOf(const wchar_t* name) {
    return !EWideString::Compare(name, L"EmoteSprite") ? this : ECSSprite::GetTypeOf(name);
}
ECSObject* ECSEmoteSprite::Duplicate() {
    MotionGraphicsGuard guard;
    auto* copy = new ECSEmoteSprite;
    copy->width_ = width_; copy->height_ = height_;
    copy->playerPath_ = playerPath_;
    copy->scale_ = scale_; copy->coordX_ = coordX_; copy->coordY_ = coordY_;
    if (copy->CopySprite(*this)) { delete copy; return nullptr; }
    if (player_) {
        const auto actor = service_->Call([this](StudyMotionRuntime* r) {
            return study_motion_clone_player(r, player_);
        });
        if (!actor) { delete copy; return nullptr; }
        if (!service_->Retain(project_)) {
            service_->Call([actor](StudyMotionRuntime* r) { return study_motion_destroy_player(r, actor); });
            delete copy; return nullptr;
        }
        copy->service_ = service_; copy->project_ = project_; copy->player_ = actor;
        copy->device_.SetReference(device_.m_pRef, nullptr);
        copy->voice_.SetReference(voice_.m_pRef, nullptr);
        copy->voiceVariable_ = voiceVariable_; copy->voiceCurve_ = voiceCurve_;
        copy->voiceSamplesPerFrame_ = voiceSamplesPerFrame_; copy->voiceGain_ = voiceGain_;
        copy->voiceWasPlaying_ = voiceWasPlaying_;
        copy->timeline_ = timeline_; copy->timelineQueue_ = timelineQueue_;
    }
    return copy;
}
ESLError ECSEmoteSprite::Move(ECSContext& context, ECSObject* object) {
    auto* source = ESLTypeCast<ECSEmoteSprite>(ECSObject::GetEntity(object));
    if (!source) return ESLErrorMsg("EmoteSprite assignment requires an EmoteSprite");
    if (source == this) { context.delete_CSObject(object); return eslErrSuccess; }
    MotionGraphicsGuard guard;
    std::unique_ptr<ECSEmoteSprite> copy(static_cast<ECSEmoteSprite*>(source->Duplicate()));
    if (!copy) return ESLErrorMsg("EmoteSprite actor-state duplication failed");
    Release();
    const auto error = CopySprite(*copy);
    if (error) return error;
    width_ = copy->width_; height_ = copy->height_;
    playerPath_ = copy->playerPath_;
    scale_ = copy->scale_; coordX_ = copy->coordX_; coordY_ = copy->coordY_;
    service_ = std::move(copy->service_); project_ = copy->project_; player_ = copy->player_;
    copy->project_ = 0; copy->player_ = 0;
    device_.SetReference(copy->device_.m_pRef, &context);
    voice_.SetReference(copy->voice_.m_pRef, &context);
    voiceVariable_ = std::move(copy->voiceVariable_); voiceCurve_ = std::move(copy->voiceCurve_);
    voiceSamplesPerFrame_ = copy->voiceSamplesPerFrame_; voiceGain_ = copy->voiceGain_;
    voiceWasPlaying_ = copy->voiceWasPlaying_;
    timeline_ = std::move(copy->timeline_); timelineQueue_ = std::move(copy->timelineQueue_);
    context.delete_CSObject(object);
    return eslErrSuccess;
}
ESLError ECSEmoteSprite::GetFunction(ECSContext& context, int& index, const wchar_t* name) {
    for (int i = 0; i < 9; ++i) if (!EWideString::Compare(name, motionMethods[i])) {
        index = 4096 + i; return eslErrSuccess;
    }
    return ECSSprite::GetFunction(context, index, name);
}
ESLError ECSEmoteSprite::Release() {
    MotionGraphicsGuard guard;
    ReleaseMotionPlayer();
    restorePlayer_ = false;
    scale_ = 1; coordX_ = coordY_ = 0;
    timeline_.clear(); timelineQueue_.clear();
    voice_.SetReference(nullptr, nullptr);
    voiceCurve_.clear(); voiceVariable_.clear(); voiceWasPlaying_ = false;
    return ECSSprite::Release();
}
void ECSEmoteSprite::ReleaseMotionPlayer() {
    MotionGraphicsGuard guard;
    NativeSprite().AttachImage(nullptr);
    m_image.reset();
    if (service_ && player_) service_->Call([this](StudyMotionRuntime* r) { return study_motion_destroy_player(r, player_); });
    player_ = 0;
    if (service_) service_->Unload(project_);
    project_ = 0; service_.reset();
    playerPath_.clear(); renderFailed_ = false;
    motionTraceFrames_ = 0;
    motionTraceAdvances_ = 0;
    framePixels_.clear();
    device_.SetReference(nullptr, nullptr);
}
ESLError ECSEmoteSprite::OpenPlayer(ECSContext& context, ECSEmoteDevice& device, const wchar_t* path) {
    const auto started = std::chrono::steady_clock::now();
    auto service = device.Service();
    if (!(service->Capabilities() & STUDY_MOTION_GLES_RENDERER)) return eslErrNotSupported;
    uint64_t project = 0;
    {
        const auto locks = SSystem::UnlockAll();
        struct Relock { decltype(locks) count; ~Relock() { SSystem::Relock(count); } } relock{locks};
        project = service->Load(path);
    }
    if (!project) return eslErrGeneral;
    const auto player = service->Call([project](StudyMotionRuntime* r) {
        const auto id = study_motion_create_player(r, project);
        if (!id) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion create Player: %s", study_motion_last_error(r));
        return id;
    });
    if (!player) { service->Unload(project); return eslErrGeneral; }
    MotionGraphicsGuard guard;
    service_ = std::move(service); project_ = project; player_ = player;
    playerPath_ = path;
    scale_ = 1; coordX_ = coordY_ = 0;
    device_.SetReference(&device, &context);
    NativeSprite().NotifyUpdate();
    // Total includes archive reads, PSB key resolution, decoding and Player creation.
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady",
        "EmoteSprite loaded real Player from NOA: %s; load_total_ms=%.1f", MotionUTF8(path).c_str(),
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count());
    TraceMotion("opened");
    return eslErrSuccess;
}
ESLError ECSEmoteSprite::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    if (index < 4096) return ECSSprite::CallFunction(context, index, args);
    const int method = index - 4096;
    if (method < 0 || method >= 9) return ESLErrorMsg("Invalid EmoteSprite method index");
    auto result = [&](INT64 value = 0) { return context.PushObject(context.new_CSInteger(value)); };
    ESLError error = eslErrSuccess;
    auto count = [&](int low, int high = 0) { return context.VerifyArgumentCount(args, low, high); };
    if (method == 0) {
        if ((error = count(3))) return error;
        auto* device = ESLTypeCast<ECSEmoteDevice>(context.GetArgumentObjectAs(args, 1, L"EmoteDevice"));
        ECSWideString path;
        if (!device) return ESLErrorMsg("LoadPlayer requires EmoteDevice");
        if ((error = context.GetArgumentAsStr(path, args, 2, nullptr))) return error;
        ReleaseMotionPlayer();
        // File loading can acquire SDK file/archive locks. Do it before holding
        // the graphics lock needed by the Android renderer.
        return result(OpenPlayer(context, *device, path));
    }
    if (method == 1) { if ((error = count(1))) return error; return result(Release()); }
    MotionGraphicsGuard guard;
    if (method == 2) {
        if ((error = count(3))) return error;
        int width, height;
        if ((error = context.GetArgumentAsInt(width, args, 1, 0)) ||
            (error = context.GetArgumentAsInt(height, args, 2, 0))) return error;
        if (width < 1 || height < 1 || width > 8192 || height > 8192 || uint64_t(width) * height > 33554432)
            return eslErrInvalidParam;
        width_ = width; height_ = height; renderFailed_ = false;
        return result();
    }
    if (method == 3 || method == 4) {
        const int minimum = method == 3 ? 2 : 3;
        if ((error = count(minimum, minimum + 1))) return error;
        double x, y = 0, duration = 0;
        if ((error = context.GetArgumentAsReal(x, args, 1, 0)) ||
            (method == 4 && (error = context.GetArgumentAsReal(y, args, 2, 0))) ||
            (error = context.GetArgumentAsReal(duration, args, minimum, 0))) return error;
        if (!std::isfinite(float(x)) || !std::isfinite(float(y)) || !std::isfinite(duration)) return eslErrInvalidParam;
        if (method == 3) scale_ = float(x);
        else { coordX_ = float(x); coordY_ = float(y); }
        if (!player_ || !service_) return result();
        const bool success = service_->Call([&](StudyMotionRuntime* r) {
            return method == 3 ? study_motion_set_scale(r, player_, x, duration, 0)
                : study_motion_set_coord(r, player_, x, y, duration, 0);
        });
        renderFailed_ = false;
        TraceMotion(method==3?"set-scale":"set-coord");
        return result(success ? 0 : eslErrGeneral);
    }
    if (!player_ || !service_) return ESLErrorMsg("EmoteSprite has no loaded Player");
    if (method >= 5 && method <= 7) {
        if ((error = count(2))) return error;
        ECSWideString value;
        if ((error = context.GetArgumentAsStr(value, args, 1, nullptr))) return error;
        const auto name = MotionUTF8(value);
        if (method == 6) { timelineQueue_.push_back(name); return result(); }
        if (method == 7) {
            int playing = 0;
            if (!service_->Call([&](StudyMotionRuntime* r) { return study_motion_is_timeline_playing(r, player_, name.c_str(), &playing); }))
                return ESLErrorMsg("EmoteSprite timeline query failed");
            return result(playing ? -1 : 0);
        }
        const bool success = service_->Call([&](StudyMotionRuntime* r) {
            return study_motion_stop_timeline(r, player_, "") && study_motion_play_timeline(r, player_, name.c_str(), 1);
        });
        if (success) { timeline_ = name; timelineQueue_.clear(); renderFailed_ = false; }
        return result(success ? 0 : eslErrGeneral);
    }
    if ((error = count(4))) return error;
    auto* resource = ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args, 1, L"Resource"));
    ECSWideString variable;
    double gain = 5;
    if ((error = context.GetArgumentAsStr(variable, args, 2, nullptr)) ||
        (error = context.GetArgumentAsReal(gain, args, 3, 5))) return error;
    if (!std::isfinite(gain)) return eslErrInvalidParam;
    if (resource && resource->GetSound()) {
        // An independent decoder leaves the live playback cursor untouched.
        std::unique_ptr<SakuraGL::SGLAudioPlayerInterface> clone(resource->GetSound()->ClonePlayer());
        if (!clone) return result(eslErrGeneral);
        clone->Stop();
        ECSAudioPlayer::PCM pcm;
        if ((error = ECSAudioPlayer::Decode(*clone, pcm))) return result(error);
        if (pcm.format.format != SakuraGL::formatSoundLinearPCM || pcm.format.bitsPerSample != 16 ||
            pcm.format.channels != 1 || pcm.format.frequency == 0)
            return ESLErrorMsg("Emote voice analysis currently requires mono 16-bit PCM");
        const uint32_t chunk = (pcm.format.frequency + 59) / 60;
        const size_t samples = pcm.data.size() / 2;
        std::vector<double> curve;
        curve.reserve((samples + chunk - 1) / chunk);
        int previous = 0;
        double maximum = 0;
        for (size_t first = 0; first < samples; first += chunk) {
            const auto end = std::min(samples, first + chunk);
            double sum = 0;
            for (size_t i = first; i < end; ++i) {
                const int sample = int16_t(uint16_t(pcm.data[2*i]) | uint16_t(pcm.data[2*i+1]) << 8);
                sum += std::abs(sample - previous); previous = sample;
            }
            const double amplitude = std::pow(sum / (end - first), .25);
            curve.push_back(amplitude); maximum = std::max(maximum, amplitude);
        }
        if (maximum > 0) for (auto& value : curve) value /= maximum;
        voiceCurve_ = std::move(curve); voiceSamplesPerFrame_ = chunk;
        voice_.SetReference(resource, &context); voiceVariable_ = MotionUTF8(variable); voiceGain_ = gain;
        voiceWasPlaying_ = false;
    } else {
        if (ECSObject::GetEntity(args.GetAt(1)) && !resource)
            return ESLErrorMsg("AttachVoiceSync requires a Resource or null");
        voice_.SetReference(nullptr, &context); voiceCurve_.clear(); voiceWasPlaying_ = false;
    }
    return result();
}

ECSObject* ECSEmoteSprite::GetVariableAt(int index) {
    if (index == -11) return &device_;
    if (index == -12) return &voice_;
    return ECSSprite::GetVariableAt(index);
}
void ECSEmoteSprite::IndexAllMember() {
    ECSSprite::IndexAllMember();
    device_.IndexAllMember(); device_.m_pParent = this; device_.m_nIndex = -11;
    voice_.IndexAllMember(); voice_.m_pParent = this; voice_.m_nIndex = -12;
}
void ECSEmoteSprite::CleanupAllReference(ECSContext& context) {
    MotionGraphicsGuard guard;
    voice_.CleanupAllReference(context); device_.CleanupAllReference(context);
    ECSSprite::CleanupAllReference(context);
}
ESLError ECSEmoteSprite::CommitAllReference(ECSContext& context) {
    MotionGraphicsGuard guard;
    auto error = ECSSprite::CommitAllReference(context);
    if (!error) error = device_.CommitAllReference(context);
    if (!error) error = voice_.CommitAllReference(context);
    if (error || !restorePlayer_) return error;
    if (playerPath_.empty()) { restorePlayer_ = false; return eslErrSuccess; }
    auto* device = ESLTypeCast<ECSEmoteDevice>(ECSObject::GetEntity(&device_));
    if (!device) return ESLErrorMsg("Saved EmoteSprite has no initialized EmoteDevice reference");
    ECSReference keepDevice(device);
    const auto path = playerPath_;
    const auto scale = scale_, x = coordX_, y = coordY_;
    if (player_) ReleaseMotionPlayer();
    if ((error = OpenPlayer(context, *device, path.c_str()))) {
        playerPath_ = path;
        device_.SetReference(device, &context);
        return error;
    }
    scale_ = scale; coordX_ = x; coordY_ = y;
    const bool success = service_->Call([&](StudyMotionRuntime* r) {
        // The original Windows record restores the target transform and starts
        // its named timeline again. It does not contain an Engine snapshot.
        return study_motion_set_scale(r, player_, scale, 0, 0) &&
            study_motion_set_coord(r, player_, x, y, 0, 0) &&
            (timeline_.empty() || study_motion_play_timeline(r, player_, timeline_.c_str(), 1)) &&
            study_motion_progress_player(r, player_, 0);
    });
    if (!success) return ESLErrorMsg("Saved EmoteSprite transform or timeline cannot be reconstructed");
    TraceMotion("committed");
    restorePlayer_ = false;
    return eslErrSuccess;
}

ESLError ECSEmoteSprite::Save(ESLFileObject& file, ECSContext& context) {
    MotionGraphicsGuard guard;
    if (restorePlayer_) return ESLErrorMsg("EmoteSprite cannot save before player restoration is committed");
    if (timelineQueue_.size() > 65536) return eslErrInvalidParam;
    TraceMotion("saving");
    // The image is a transient renderer output. Windows serializes the PSB
    // filename below, not that anonymous Resource image. Keep it alive and
    // attached to the native Sprite throughout the base serialization.
    const bool transient = player_ != 0;
    auto rendered = transient ? std::move(m_image) : decltype(m_image){};
    struct RestoreImage {
        decltype(m_image)& image; decltype(m_image)& rendered;
        bool active;
        ~RestoreImage() { if (active) image = std::move(rendered); }
    } restore{m_image, rendered, transient};
    if (const auto error = ECSSprite::Save(file, context)) return error;
    LegacySave::Writer out{file};
    out.Reference(device_, context);
    out.U32(width_); out.U32(height_);
    out.F32(scale_); out.F32(coordX_); out.F32(coordY_);
    out.String(playerPath_.c_str()); out.String(MotionWide(timeline_).c_str());
    out.U32(static_cast<uint32_t>(timelineQueue_.size()));
    for (const auto& name : timelineQueue_) out.String(MotionWide(name).c_str());
    return out.error;
}

ESLError ECSEmoteSprite::Load(ESLFileObject& file, ECSContext& context) {
    MotionGraphicsGuard guard;
    if (const auto error = ECSSprite::Load(file, context)) return error;
    LegacySave::Reader in{file};
    in.Reference(device_, context);
    const auto width = in.U32(), height = in.U32();
    const auto scale = in.F32(), x = in.F32(), y = in.F32();
    const auto path = in.String(), timeline = in.String();
    const auto count = in.Count();
    std::deque<std::string> queue;
    for (uint32_t i = 0; i < count && !in.error; ++i) queue.push_back(MotionUTF8(in.String()));
    if (in.error) return in.error;
    if (!width || !height || width > 8192 || height > 8192 || uint64_t(width) * height > 33554432)
        return eslErrInvalidParam;
    width_ = width; height_ = height;
    scale_ = scale; coordX_ = x; coordY_ = y;
    playerPath_ = path.CharPtr() ? path.CharPtr() : L"";
    timeline_ = MotionUTF8(timeline); timelineQueue_ = std::move(queue);
    restorePlayer_ = true;
    return eslErrSuccess;
}

void ECSEmoteSprite::AdvanceMotion(uint32_t milliseconds) {
    MotionGraphicsGuard guard;
    if (!service_ || !player_ || renderFailed_) return;
    bool updateVoice = false;
    double voiceValue = 0;
    auto* voice = ESLTypeCast<ECSResource>(ECSObject::GetEntity(&voice_));
    if (voice && voice->GetSound() && voice->GetSound()->IsPlaying() && voiceSamplesPerFrame_) {
        const auto frame = voice->GetSound()->GetPosition() / voiceSamplesPerFrame_;
        if (frame < voiceCurve_.size()) { voiceValue = voiceCurve_[frame] * voiceGain_; updateVoice = true; }
        voiceWasPlaying_ = true;
    } else if (voiceWasPlaying_) { voiceWasPlaying_ = false; updateVoice = true; }
    const bool success = service_->Call([&](StudyMotionRuntime* r) {
        if (updateVoice && !study_motion_set_variable(r, player_, voiceVariable_.c_str(), voiceValue, 0, 0)) return false;
        if (!study_motion_progress_player(r, player_, double(milliseconds) * .06)) return false;
        if (!timeline_.empty()) {
            int playing = 0;
            if (!study_motion_is_timeline_playing(r, player_, timeline_.c_str(), &playing)) return false;
            if (!playing) {
                if (timelineQueue_.empty()) timeline_.clear();
                else {
                    const auto next = timelineQueue_.front();
                    if (!study_motion_stop_timeline(r, player_, "") || !study_motion_play_timeline(r, player_, next.c_str(), 1)) return false;
                    timeline_ = next; timelineQueue_.pop_front();
                }
            }
        }
        return true;
    });
    if (!success) {
        renderFailed_ = true;
        study::platform::LogWrite(study::platform::LogPriority::Error, "StudySteady", "EmoteSprite frame progression failed");
    }
    if(motionTraceAdvances_==0||motionTraceAdvances_==30)TraceMotion(motionTraceAdvances_==0?"first-advance":"advance-30");
    if(motionTraceAdvances_<31)++motionTraceAdvances_;
    // This Sprite attaches a rendered image without owning an SDK framebuffer.
    // Refresh() would return immediately and leave its parent's cached scene
    // unchanged. NotifyUpdate schedules real parent composition next frame.
    NativeSprite().NotifyUpdate();
}

void ECSEmoteSprite::DrawMotion() {
    MotionGraphicsGuard guard;
    if (!service_ || !player_ || renderFailed_) return;
    const bool tracePixels=motionTraceFrames_==0||motionTraceFrames_==30;
    if(tracePixels)TraceMotion(motionTraceFrames_==0?"first-draw":"draw-30");
    if(motionTraceFrames_<31)++motionTraceFrames_;
    framePixels_.resize(size_t(width_) * height_ * 4);
    const double transform[] = {1, 0, 0, 1, width_ * .5, height_ * .5};
    const bool success = service_->Call([&](StudyMotionRuntime* r) {
        StudyMotionFrame frame{};
        const bool ok = study_motion_render_player(r, player_, width_, height_, transform, &frame)
            && study_motion_read_pixels(r, player_, framePixels_.data(), framePixels_.size());
        if (!ok) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "EmoteSprite draw: %s", study_motion_last_error(r));
        return ok;
    });
    if (!success) { renderFailed_ = true; return; }
    if(tracePixels) {
        uint32_t hash=2166136261u;size_t alpha=0;
        for(size_t i=0;i<framePixels_.size();++i){hash=(hash^framePixels_[i])*16777619u;if((i&3)==3&&framePixels_[i])++alpha;}
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Emote frame actor=%llu path=%s count=%u rgba_fnv32=%08x alpha=%zu visible=%d parent=%p framebuffer=%p",
            static_cast<unsigned long long>(player_),MotionUTF8(playerPath_.c_str()).c_str(),motionTraceFrames_,hash,alpha,
            NativeSprite().IsVisible(),NativeSprite().GetParent(),NativeSprite().GetFrameBuffer());
    }
    SakuraGL::SGLImageInfo info;
    if (!m_image || m_image->GetImageInfo(info) || info.width != width_ || info.height != height_) {
        auto image = std::make_shared<SakuraGL::SGLImage>();
        if (image->CreateImage(width_, height_, SakuraGL::formatImageABGR, 32)) {
            renderFailed_ = true; return;
        }
        m_image = std::move(image);
        NativeSprite().AttachImage(m_image.get());
    }
    auto* output = m_image->LockBuffer(info, SakuraGL::SGLImageObject::lockWrite);
    if (!output) { renderFailed_ = true; return; }
    for (uint32_t y = 0; y < height_; ++y)
        std::memcpy(output + y * info.pitchLine, framePixels_.data() + size_t(y) * width_ * 4, size_t(width_) * 4);
    m_image->UnlockBuffer(SakuraGL::SGLImageObject::lockWrite);
    NativeSprite().NotifyUpdate();
}

void ECSEmoteSprite::TraceMotion(const char* stage) {
    if(!service_||!player_)return;
    StudyMotionTransform transform{};double bounds[4]{};
    const bool ok=service_->Call([&](StudyMotionRuntime* r){
        return study_motion_get_transform(r,player_,&transform)&&
            study_motion_player_bounds(r,player_,&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
    });
    const auto& native=NativeSprite().GetParameter();
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
        "Emote transform %s actor=%llu path=%s ok=%d viewport=%ux%u target=%.6g,%.6g,%.6g base/user=%.6g,%.6g ctl=%.6g,%.6g,%.6g published=%.6g,%.6g,%.6g,%.6g bounds=%.3f,%.3f,%.3f,%.3f nativeDst=%.3f,%.3f center=%.3f,%.3f zoom=%.6g,%.6g frame=%.3f timeline=%s queue=%zu",
        stage,static_cast<unsigned long long>(player_),MotionUTF8(playerPath_.c_str()).c_str(),ok,width_,height_,
        double(scale_),double(coordX_),double(coordY_),transform.base_scale,transform.user_scale,
        transform.controller_scale,transform.controller_x,transform.controller_y,
        transform.player_scale_x,transform.player_scale_y,transform.player_x,transform.player_y,
        bounds[0],bounds[1],bounds[2],bounds[3],native.vDst.x,native.vDst.y,native.vCenter.x,native.vCenter.y,
        native.vZoom.x,native.vZoom.y,transform.frame,timeline_.c_str(),timelineQueue_.size());
}

bool CheckLegacyMotionOwner(ECSEnvironment& environment) {
    try {
        auto service = LegacyMotionService::Acquire(environment);
        const auto id = service->Load(L"haz_a.psb");
        std::string chara, motion;
        if (!id || !service->Base(id, chara, motion) || chara != "all_parts") return false;
        service->Unload(id);
        study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Motion owner PASS: real TJS/NCB, original NOA PSB, base=%s/%s, owner-thread lifecycle; renderer unavailable", chara.c_str(), motion.c_str());
        return true;
    } catch (const std::exception& e) {
        study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Motion owner probe: %s", e.what());
        return false;
    }
}
