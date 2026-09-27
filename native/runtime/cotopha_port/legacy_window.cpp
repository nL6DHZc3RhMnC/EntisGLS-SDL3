#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_window.h"
#include "runtime/cotopha_port/legacy_window_draw.h"
#include "runtime/cotopha_port/legacy_paint_gate.h"
#include <sakuraglx/sprite/sglx_sprite_button.h>
#include "runtime/cotopha_port/legacy_input.h"
#include <algorithm>
#include "platform/log.h"
#include <chrono>
#include <cwchar>
#include <thread>

IMPLEMENT_CLASS_INFO(ECSWindow, ECSSprite)
namespace {
using namespace SakuraGL;
using Clock = std::chrono::steady_clock;
LegacyPaintGate<SGLGenericWindow> paintFreezeGate;
bool IsPaintFrozen(const SGLGenericWindow* window) {
    return paintFreezeGate.IsFrozen(window);
}
constexpr int windowMethodBase = 2048;
const wchar_t *const methods[] = {
    L"CreateDisplay", L"CloseDisplay", L"GetOptionalFuncFlag", L"SetOptionalFuncFlag",
    L"ChangeCooperationLevel", L"ChangeDisplaySize", L"SetChangeDisplayModeFlag",
    L"SetStereoDisplayMode", L"IsSupportedStereoDisplayMode", L"GetDisplaySize",
    L"UpdateWindow", L"ProcessUserInput", L"IsWindowActive", L"InitWindowPosition",
    L"GetNormalWindowPosition", L"GetPhysicalMonitorSize", L"SetExteriorBackgroundFrame",
    L"MessageBox", L"CreateWindow", L"CloseWindow", L"ChangeWindowSize", L"SetLayeredWindow",
    L"SetWindowLayout", L"EnableCommandQueue", L"FlushCommandQueue", L"QueueCommand",
    L"GetCommand", L"CallMouseMove", L"Lock", L"Unlock", L"FreezePaint", L"UnfreezePaint",
    L"SyncTimePaint", L"AsyncTimePaint", L"ShowCursor", L"IsShowCursor", L"Refresh"
};
struct WindowDefaults {
    ECSWideString caption;
    int mode = Window::modeWindow, width = 640, height = 480, depth = 0, frequency = 0;
    explicit WindowDefaults(ECSContext &context) {
        auto *env = context.GetEnvironment();
        if (!env) return;
        SSystem::SString app;
        env->GetApplicationName(app);
        caption = static_cast<const wchar_t *>(app);
        const auto &doc = env->GetXMLDocumnet();
        auto *script = doc.GetElementTagAs(L"script");
        if (!script) script = doc.GetElementTagAs(L"cotopha");
        auto *display = script ? script->GetElementTagAs(L"display") : nullptr;
        if (!display) return;
        const auto displayCaption = display->GetAttrStringAs(L"caption", caption);
        caption = static_cast<const wchar_t *>(displayCaption);
        width = display->GetAttrIntegerAs(L"width", width);
        height = display->GetAttrIntegerAs(L"height", height);
        depth = display->GetAttrIntegerAs(L"depth", depth);
        frequency = display->GetAttrIntegerAs(L"frequency", frequency);
        const auto cooperation = display->GetAttrStringAs(L"CooperationLevel", L"window");
        if (cooperation == L"normal") mode = Window::modeNormal;
        else if (cooperation == L"fullscreen") mode = Window::modeFullScreen;
        else if (cooperation == L"exclusive") mode = Window::modeExclusive;
    }
};
bool ValidSize(int w, int h) { return w > 0 && h > 0 && w <= 16384 && h <= 16384; }
// GLS4 emits overwriteable status after click and again on pointer leave.
// GLS3 emitted non-overwriteable status before click. Reconcile both mouse and
// touch dispatch here; script QueueCommand and unrelated native commands keep
// their original overwrite semantics.
thread_local const ECSWindow* pointerCommandWindow=nullptr;
struct PointerCommandScope {
    const ECSWindow* previous=pointerCommandWindow;
    explicit PointerCommandScope(const ECSWindow* window) { pointerCommandWindow=window; }
    ~PointerCommandScope(){pointerCommandWindow=previous;}
};
}

void StudySteadyDrawAndroidWindow(SakuraGL::SGLGenericWindow* window) {
    // OnDraw is not virtual in the official SDK. The generated JNI entry calls
    // here so the freeze check still precedes OnDraw's acquisition of the UI
    // mutex, just as it did in the previous SDK extension.
    StudySteadyDrawWindow(window);
}

bool StudySteadyDrawWindow(SakuraGL::SGLGenericWindow* window) {
    bool drawn = false;
    paintFreezeGate.Draw(window, [&](SakuraGL::SGLGenericWindow* target) {
        target->OnDraw(); drawn = true;
    });
    return drawn;
}

class ECSWindow::WindowBridge final : public SakuraGL::SGLWindowSprite {
    ECSWindow &owner_;
    struct MouseProxy final : SakuraGL::SGLMouseInterface {
        WindowBridge &bridge;
        explicit MouseProxy(WindowBridge &b) : bridge(b) {}
        bool OnMouseMove(SakuraGL::Window *w, int32_t x, int32_t y, int64_t flags) override {
            PointerCommandScope dispatch(&bridge.owner_);
            bridge.RememberPointer(x,y,flags);
            std::lock_guard<std::recursive_mutex> guard(bridge.owner_.filterMutex_);
            auto *filter = bridge.owner_.inputFilter_;
            if (filter && bridge.owner_.filterMode_ == 0) filter->OnMouseMove(x, y);
            bool handled = bridge.originalMouse_->OnMouseMove(w, x, y, flags);
            if (filter && bridge.owner_.filterMode_ == 1) filter->OnMouseMove(x, y);
            return handled;
        }
        void OnMouseLeave(SakuraGL::Window *w, int64_t flags) override {
            PointerCommandScope dispatch(&bridge.owner_);
            bridge.EndPointer(flags);
            bridge.originalMouse_->OnMouseLeave(w, flags);
        }
        bool OnMouseWheel(SakuraGL::Window *w, int32_t delta, int32_t x, int32_t y, int64_t flags) override {
            PointerCommandScope dispatch(&bridge.owner_);
            std::lock_guard<std::recursive_mutex> guard(bridge.owner_.filterMutex_);
            auto *filter = bridge.owner_.inputFilter_;
            if (filter && bridge.owner_.filterMode_ == 0 && filter->OnMouseWheel(delta, x, y)) return true;
            if (bridge.originalMouse_->OnMouseWheel(w, delta, x, y, flags)) return true;
            return filter && bridge.owner_.filterMode_ == 1 && filter->OnMouseWheel(delta, x, y);
        }
        bool Button(SakuraGL::Window *w, int32_t x, int32_t y, int64_t flags, int action) {
            PointerCommandScope dispatch(&bridge.owner_);
            bridge.RememberPointer(x,y,flags);
            struct FinishTouch {WindowBridge& bridge;int64_t flags;bool up;~FinishTouch(){if(up&&(flags&TouchFlag))bridge.EndPointer(flags);}} finish{bridge,flags,action==0};
            std::lock_guard<std::recursive_mutex> guard(bridge.owner_.filterMutex_);
            const unsigned button = GetButtonID(flags);
            const int code = button == 0 ? 1 : button == 1 ? 2 : button == 2 ? 4 : button == 3 ? 5 : 6;
            auto *filter = bridge.owner_.inputFilter_;
            if (filter && bridge.owner_.filterMode_ == 0 && filter->OnMouseButton(code, x, y, action != 0)) return true;
            const bool handled = action == 0 ? bridge.originalMouse_->OnButtonUp(w, x, y, flags) :
                action == 1 ? bridge.originalMouse_->OnButtonDown(w, x, y, flags) :
                              bridge.originalMouse_->OnButtonDblClk(w, x, y, flags);
            return handled || (filter && bridge.owner_.filterMode_ == 1 && filter->OnMouseButton(code, x, y, action != 0));
        }
        bool OnButtonDown(SakuraGL::Window *w, int32_t x, int32_t y, int64_t f) override { return Button(w,x,y,f,1); }
        bool OnButtonUp(SakuraGL::Window *w, int32_t x, int32_t y, int64_t f) override { return Button(w,x,y,f,0); }
        bool OnButtonDblClk(SakuraGL::Window *w, int32_t x, int32_t y, int64_t f) override { return Button(w,x,y,f,2); }
    } mouseProxy_;
    struct KeyProxy final : SakuraGL::SGLKeyInterface {
        WindowBridge &bridge;
        explicit KeyProxy(WindowBridge &b) : bridge(b) {}
        bool Key(SakuraGL::Window *w, int64_t key, int64_t flags, bool down) {
            PointerCommandScope dispatch(nullptr);
            std::lock_guard<std::recursive_mutex> guard(bridge.owner_.filterMutex_);
            auto *filter = bridge.owner_.inputFilter_;
            if (filter && bridge.owner_.filterMode_ == 0 && filter->OnKey(key, flags, down)) return true;
            if (down ? bridge.originalKey_->OnKeyDown(w, key, flags) : bridge.originalKey_->OnKeyUp(w, key, flags)) return true;
            return filter && bridge.owner_.filterMode_ == 1 && filter->OnKey(key, flags, down);
        }
        bool OnKeyDown(SakuraGL::Window *w, int64_t k, int64_t f) override { return Key(w,k,f,true); }
        bool OnKeyUp(SakuraGL::Window *w, int64_t k, int64_t f) override { return Key(w,k,f,false); }
        void OnSetFocus(SakuraGL::Window *w) override { bridge.originalKey_->OnSetFocus(w); }
        void OnKillFocus(SakuraGL::Window *w) override {
            std::lock_guard<std::recursive_mutex> guard(bridge.owner_.filterMutex_);
            if (bridge.owner_.inputFilter_) bridge.owner_.inputFilter_->OnFocusLost();
            bridge.originalKey_->OnKillFocus(w);
        }
    } keyProxy_;
    struct CommandListener final : SakuraGL::SGLSpriteKeyListener {
        ECSWindow &owner;
        explicit CommandListener(ECSWindow &w) : owner(w) {}
        bool OnCommand(SakuraGL::SGLSprite &, const wchar_t *id, int64_t parameter,
                       int64_t notification, int priority, bool overwrite) override {
            return owner.NotifyCommand(id, notification, parameter, priority, overwrite);
        }
    } commandListener_;
    SakuraGL::SGLMouseInterface *originalMouse_ = nullptr;
    SakuraGL::SGLKeyInterface *originalKey_ = nullptr;
    std::mutex paintMutex_;
    std::condition_variable paintChanged_;
    bool gatePaint_ = false, stopped_ = false;
    uint64_t paintSerial_ = 0;
    SakuraGL::SGLPoint lastPointer_;
    int64_t lastPointerFlags_=0;
    bool pointerPresent_=false;
    void RememberPointer(int32_t x,int32_t y,int64_t flags) {
        using Mouse=SakuraGL::SGLMouseInterface;
        if(pointerPresent_&&(lastPointerFlags_&Mouse::TouchFlag)&&(flags&Mouse::TouchFlag)&&
            Mouse::GetMouseID(lastPointerFlags_)!=Mouse::GetMouseID(flags))return;
        lastPointer_=SakuraGL::SGLPoint(x,y);lastPointerFlags_=flags;pointerPresent_=true;
        // Keep SDK cursor readers synchronized without feeding received input
        // back into an OS cursor warp. The Android method is cache-only; the
        // SDL method performs a real warp for explicit cursor-move requests.
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        SSystem::SSmartLock<SSystem::SMutex> guard(SakuraGL::SGLGenericWindow::m_pMutexUI);
        if (Mouse::GetMouseID(flags) == m_idPrimaryTouch) {
            S2DDVector physical(x,y);
            ScreenPositionFromClient(physical);
            m_vPrimaryTouch=physical;
        }
#else
        SakuraGL::SGLWindow::MoveCursorPosition(x,y,Mouse::GetMouseID(flags));
#endif
    }
    void EndPointer(int64_t flags) {
        using Mouse=SakuraGL::SGLMouseInterface;
        if(Mouse::GetMouseID(lastPointerFlags_)==Mouse::GetMouseID(flags)&&
            bool(lastPointerFlags_&Mouse::TouchFlag)==bool(flags&Mouse::TouchFlag))pointerPresent_=false;
    }
public:
    explicit WindowBridge(ECSWindow &owner)
        : owner_(owner), mouseProxy_(*this), keyProxy_(*this), commandListener_(owner) {
        SetWindowUIThreadMutex(SSystem::g_mutexGlobal);
        SetOptionalFlags(SakuraGL::Window::flagBlackBack);
        AttachKeyPostListener(&commandListener_);
    }
    void BeginPaintFreeze() {
        paintFreezeGate.Freeze(this);
    }
    bool EndPaintFreeze() {
        return paintFreezeGate.Unfreeze(this);
    }
    void ResetPaintFreeze() {
        paintFreezeGate.Reset(this);
    }
    void InstallInputProxies() {
        // BindWindowToSprite normally reinstalls the SDK listeners on create.
        // Keep repeated installation safe if called after an already-bound view.
        if (GetMouseInterface() != &mouseProxy_) originalMouse_ = GetMouseInterface();
        if(!originalMouse_)originalMouse_=&m_listenMouse;
        if (GetKeyInterface() != &keyProxy_) originalKey_ = GetKeyInterface();
        if(!originalKey_)originalKey_=&m_listenKey;
        SakuraGL::SGLWindow::SetMouseInterface(&mouseProxy_);
        SakuraGL::SGLWindow::SetKeyInterface(&keyProxy_);
        std::lock_guard<std::mutex> guard(paintMutex_);
        stopped_ = false;
    }
    bool NotifyCommand(const wchar_t *id, int64_t parameter, int64_t notification,
                       int priority, bool overwrite) override {
        return owner_.NotifyCommand(id, notification, parameter, priority, overwrite);
    }
    void CallMouseMove() {
        SSystem::Lock();
        struct Unlock {~Unlock(){SSystem::Unlock();}} unlock;
        if(!pointerPresent_||!originalMouse_)return;
        PointerCommandScope dispatch(&owner_);
        // Re-enter the original native listener, which performs GlobalToLocal
        // and raw touch-id normalization just like the actual Android event.
        // Do not emit another InputFilter event for a script-requested refresh.
        originalMouse_->OnMouseMove(this,lastPointer_.x,lastPointer_.y,lastPointerFlags_);
    }
    ESLError MoveLogicalCursor(int x,int y) {
        SSystem::Lock();struct Unlock {~Unlock(){SSystem::Unlock();}} unlock;
        if(!originalMouse_)return ESLErrorMsg("Window input is not ready for logical cursor movement");
        const int64_t flags=pointerPresent_&&(lastPointerFlags_&SakuraGL::SGLMouseInterface::TouchFlag)?lastPointerFlags_:0;
        // This is an explicit in-game cursor move, so update InputFilter's
        // logical cursor and perform native hover/hit testing immediately.
        mouseProxy_.OnMouseMove(this,x,y,flags);
        return eslErrSuccess;
    }
    bool HasReplayPointer() const {return pointerPresent_;}
    SakuraGL::SGLSize PhysicalSize() const { return m_sizePhysicalDisplay; }
    void StopPaintGate() {
        std::lock_guard<std::mutex> guard(paintMutex_);
        stopped_ = true;
        gatePaint_ = false;
        paintChanged_.notify_all();
    }
    ESLError SyncPaint(ECSContext &context, int64_t timeout) {
        std::unique_lock<std::mutex> guard(paintMutex_);
        if (stopped_) return eslErrGeneral;
        gatePaint_ = true;
        const uint64_t serial = paintSerial_;
        const auto until = Clock::now() + std::chrono::milliseconds(timeout < 0 ? INT32_MAX : timeout);
        while (serial == paintSerial_ && !stopped_) {
            if (context.GetStatus() != ECSContext::xsExecution) return eslErrAbort;
            if (Clock::now() >= until) return eslErrTimeout;
            paintChanged_.wait_until(guard, std::min(until, Clock::now() + std::chrono::milliseconds(10)));
        }
        return stopped_ ? eslErrGeneral : eslErrSuccess;
    }
    void AsyncPaint() {
        std::lock_guard<std::mutex> guard(paintMutex_);
        gatePaint_ = false;
        paintChanged_.notify_all();
    }
    void PrepareDrawFrame() override {
        // GLS3 signalled WM_PAINT/WM_TIMER before taking its window lock and
        // allowed a script update until AsyncTimePaint (at most 100 ms).
        // The Android renderer arrives with its UI mutex held, so release it
        // only around the corresponding gate, before touching sprite data.
        std::unique_lock<std::mutex> guard(paintMutex_);
        ++paintSerial_;
        paintChanged_.notify_all();
        if (gatePaint_ && !stopped_) {
            const auto count = UnlockAll();
            paintChanged_.wait_for(guard, std::chrono::milliseconds(100),
                                  [this] { return !gatePaint_ || stopped_; });
            guard.unlock();
            Relock(count);
        } else guard.unlock();
        SakuraGL::SGLWindowSprite::PrepareDrawFrame();
    }

};

ECSWindow::ECSWindow() : window_(new WindowBridge(*this)) {
    window_->AddChild(sprite_.get());
}
ECSWindow::~ECSWindow() {
    CloseDisplay();
    {
        std::lock_guard<std::recursive_mutex> guard(filterMutex_);
        if (inputFilter_) inputFilter_->OnWindowDestroyed(this);
        inputFilter_ = nullptr;
    }
    window_->DetachChild(sprite_.get());
    window_.reset();
}
const wchar_t *ECSWindow::GetTypeName() const { return L"Window"; }
ECSObject *ECSWindow::GetTypeOf(const wchar_t *name) {
    return name && !std::wcscmp(name,L"Window") ? this : ECSSprite::GetTypeOf(name);
}
ECSObject *ECSWindow::Duplicate() { return new ECSWindow; }
SakuraGL::SGLWindowSprite *ECSWindow::GetWindow() const { return window_.get(); }
SakuraGL::SGLImageObject *ECSWindow::GetImage() const {
    if(!snapshot_&&const_cast<ECSWindow*>(this)->RefreshSnapshot())return nullptr;
    return snapshot_->GetImage();
}
ESLError ECSWindow::RefreshSnapshot(int width,int height) {
    using namespace SakuraGL;
    SSystem::Lock();struct Unlock {~Unlock(){SSystem::Unlock();}} unlock;
    if(!width||!height){
        SGLSize size;const auto error=window_->GetDisplaySize(size);
        if(error)return static_cast<ESLError>(error);
        width=size.w;height=size.h;
    }
    if(!ValidSize(width,height))return eslErrInvalidParam;
    if(!snapshot_||snapshot_->GetImage()->GetImageSize()!=SGLSize(width,height)){
        std::unique_ptr<SGLSprite::Buffer> next(new SGLSprite::Buffer);
        if(const auto error=next->CreateBuffer(width,height))return static_cast<ESLError>(error);
        snapshot_=std::move(next);
    }
    // The modern window renders directly to GLES and has no Resource image.
    // Re-render its actual scene into an independent target for GLS3's
    // screen.Refresh(); thumbnail.DrawImage(screen, ...) protocol.
    // Keeping this target separate avoids changing the live sprite tree.
    // Prepare generated images (E-mote, tiles, particles) without entering
    // WindowBridge's asynchronous paint gate or advancing simulation time.
    window_->SGLSprite::PrepareDrawFrame();
    auto& direct=window_->GetDirectRootSprite();
    direct.PrepareDrawFrame();
    struct FinishFrame {SGLWindowSprite& window;SGLSprite& direct;
        ~FinishFrame(){direct.FinishDrawFrame();window.SGLSprite::FinishDrawFrame();}
    } frameScope{*window_,direct};
    window_->BeforeDraw();
    auto* target=snapshot_->AttachRenderTarget(SGLSprite::s3dMonoview,nullptr);
    if(!target){window_->AfterDraw();return eslErrGeneral;}
    auto& render=snapshot_->Renderer();
    render.SetProjectionScreen(S3DVector(width*0.5,height*0.5,width),1.0);
    render.SetParallax(0.0,1.0,0.0);
    uint32_t background=0xff000000;window_->GetFillBackColor(background);
    render.FillClearTarget(background);
    window_->Draw(render);
    direct.BeforeDraw();direct.Draw(render);direct.AfterDraw();
    snapshot_->DetachRenderTarget(SGLSprite::s3dMonoview,nullptr);
    window_->AfterDraw();
    study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Legacy Window snapshot: %dx%d",width,height);
    return eslErrSuccess;
}
ESLError ECSWindow::MoveLogicalCursor(int x,int y) {return window_->MoveLogicalCursor(x,y);}
void ECSWindow::AttachInputFilter(ECSInputFilter *filter, int mode) {
    std::lock_guard<std::recursive_mutex> guard(filterMutex_);
    if (inputFilter_ && inputFilter_ != filter) inputFilter_->OnWindowDestroyed(this);
    inputFilter_ = filter;
    filterMode_ = mode;
    if (filter && !closed_) filter->OnWindowReady(this);
}
void ECSWindow::DetachInputFilter(ECSInputFilter *filter) {
    std::lock_guard<std::recursive_mutex> guard(filterMutex_);
    if (inputFilter_ == filter) inputFilter_ = nullptr;
}
void ECSWindow::WindowReady() {
    {
        std::lock_guard<std::mutex> guard(queueMutex_);
        closed_ = false;
    }
    window_->InstallInputProxies();
    std::lock_guard<std::recursive_mutex> guard(filterMutex_);
    if (inputFilter_) inputFilter_->OnWindowReady(this);
}
ESLError ECSWindow::CloseDisplay() {
    {
        SSystem::Lock();
        snapshot_.reset();
        SSystem::Unlock();
    }
    window_->StopPaintGate();
    {
        std::lock_guard<std::mutex> guard(queueMutex_);
        closed_ = true;
        commands_.clear();
        queueChanged_.notify_all();
    }
    window_->ResetPaintFreeze();
    return static_cast<ESLError>(window_->CloseDisplay());
}
void ECSWindow::OnDestruction(ECSContext &context) {
    CloseDisplay();
    ECSSprite::OnDestruction(context);
}
ESLError ECSWindow::Save(ESLFileObject &, ECSContext &) {
    return ESLErrorMsg("Window execution-context serialization has not been ported");
}
ESLError ECSWindow::Load(ESLFileObject &, ECSContext &) {
    return ESLErrorMsg("Window execution-context restoration has not been ported");
}
void ECSWindow::QueueCommand(const wchar_t *id, int64_t notification,
                             int64_t parameter, int priority, bool overwrite) {
    QueueCommandImpl(id,notification,parameter,priority,overwrite,false);
}
void ECSWindow::QueueCommandImpl(const wchar_t* id,int64_t notification,int64_t parameter,
                                int priority,bool overwrite,bool matchParameter) {
    Command cmd;
    cmd.fullID = id ? id : L"";
    const auto separator = cmd.fullID.find_last_of(L"/\\");
    cmd.id = separator == std::wstring::npos ? cmd.fullID : cmd.fullID.substr(separator + 1);
    cmd.notification = notification;
    cmd.parameter = parameter;
    cmd.priority = priority;
    if(std::wcsstr(cmd.fullID.c_str(),L"ID_"))
        study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Legacy Window queue id=%ls full=%ls notification=%lld parameter=%lld priority=%d",
            cmd.id.c_str(),cmd.fullID.c_str(),static_cast<long long>(notification),static_cast<long long>(parameter),priority);
    std::lock_guard<std::mutex> guard(queueMutex_);
    if (overwrite) {
        auto old = std::find_if(commands_.begin(), commands_.end(), [&cmd,matchParameter](const Command &c) { return c.fullID == cmd.fullID && (!matchParameter || c.parameter == cmd.parameter); });
        if (old != commands_.end()) commands_.erase(old);
    }
    auto at = std::find_if(commands_.begin(), commands_.end(), [&cmd](const Command &c) { return c.priority < cmd.priority; });
    commands_.insert(at, std::move(cmd));
    queueChanged_.notify_all();
}
void ECSWindow::QueueInputCommand(const wchar_t *id, int priority, bool signal) {
    QueueCommand(id, 0, 0, priority, signal);
}
bool ECSWindow::NotifyCommand(const wchar_t *id, int64_t notification,
                              int64_t parameter, int priority, bool overwrite) {
    bool enabled;
    {
        std::lock_guard<std::mutex> guard(queueMutex_);
        enabled = queueEnabled_;
    }
    // Like GLS3 WM_CLOSE, application exit remains deliverable before queue
    // activation. Ordinary sprite commands obey EnableCommandQueue.
    if (enabled || (id && !std::wcscmp(id,L"ID_APP_EXIT"))) {
        QueueCommandImpl(id, notification, parameter, priority, overwrite, pointerCommandWindow==this);
        return true;
    }
    if(id&&std::wcsstr(id,L"ID_"))study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Legacy Window ignored disabled command queue: %ls",id);
    return false;
}
ESLError ECSWindow::GetCommand(ECSContext &context, Command &cmd, int64_t timeout, bool remove) {
    std::unique_lock<std::mutex> guard(queueMutex_);
    const auto until = Clock::now() + std::chrono::milliseconds(timeout < 0 ? INT32_MAX : timeout);
    while (commands_.empty()) {
        if (closed_) return eslErrGeneral;
        if (context.GetStatus() != ECSContext::xsExecution) return eslErrAbort;
        if (Clock::now() >= until) return eslErrTimeout;
        queueChanged_.wait_until(guard, std::min(until, Clock::now() + std::chrono::milliseconds(10)));
    }
    cmd = commands_.front();
    if (remove) commands_.pop_front();
    if(remove&&std::wcsstr(cmd.fullID.c_str(),L"ID_"))
        study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Legacy Window dequeue id=%ls full=%ls notification=%lld parameter=%lld",
            cmd.id.c_str(),cmd.fullID.c_str(),static_cast<long long>(cmd.notification),static_cast<long long>(cmd.parameter));
    return eslErrSuccess;
}
ESLError ECSWindow::GetFunction(ECSContext &context, int &index, const wchar_t *name) {
    if (name) for (size_t i=0; i<sizeof(methods)/sizeof(methods[0]); ++i) {
        if (!std::wcscmp(name,methods[i])) { index = windowMethodBase + int(i); return eslErrSuccess; }
    }
    return ECSSprite::GetFunction(context,index,name);
}

ESLError ECSWindow::CallFunction(ECSContext &context, int index, ECSObjArray<ECSObject> &args) {
    if (index < windowMethodBase) {
        const auto status = window_->Lock();
        if (status) return static_cast<ESLError>(status);
        const auto error = ECSSprite::CallFunction(context,index,args);
        window_->Unlock();
        return error;
    }
    index -= windowMethodBase;
    if (index < 0 || size_t(index) >= sizeof(methods)/sizeof(methods[0])) return ESLErrorMsg("Invalid Window method index");
    const auto *name = methods[index];
    auto named = [name](const wchar_t *s) { return !std::wcscmp(name,s); };
    auto result = [&context](INT64 value=0) { return context.PushObject(new ECSInteger(value)); };
    ESLError error = eslErrSuccess;
    auto count = [&](int minimum,int maximum) { return context.VerifyArgumentCount(args,minimum,maximum); };
    auto integer = [&](int &value,int arg,int def=0) { return context.GetArgumentAsInt(value,args,arg,def); };
    if(named(L"Refresh")){
        if((error=count(1,1)))return error;
        return result(RefreshSnapshot());
    }
    if (named(L"CreateDisplay") || named(L"ChangeDisplaySize")) {
        const bool create = named(L"CreateDisplay");
        if ((error=count(1,create ? 7 : 5))) return error;
        WindowDefaults options(context);
        if (create && ((error=context.GetArgumentAsStr(options.caption,args,1,ECSWideString(options.caption))) ||
                       (error=integer(options.mode,2,options.mode)))) return error;
        int start = create ? 3 : 1;
        if ((error=integer(options.width,start,options.width)) || (error=integer(options.height,start+1,options.height)) ||
            (error=integer(options.depth,start+2,options.depth)) || (error=integer(options.frequency,start+3,options.frequency))) return error;
        if (!ValidSize(options.width,options.height)) return ESLErrorMsg("Invalid Window display dimensions");
        if (create) {
            CloseDisplay();
            const auto status = window_->CreateDisplay(options.caption,static_cast<SakuraGL::Window::CooperationMode>(options.mode),
                options.width,options.height,options.depth,options.frequency);
            study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy Window.CreateDisplay %dx%d depth=%d mode=%d status=%d",
                options.width,options.height,options.depth,options.mode,int(status));
            if (!status) WindowReady();
            return result(status);
        }
        return result(window_->ChangeDisplaySize(options.width,options.height,options.depth,options.frequency));
    }
    if (named(L"CloseDisplay") || named(L"CloseWindow")) {
        if ((error=count(1,1))) return error;
        return result(CloseDisplay());
    }
    if (named(L"GetOptionalFuncFlag") || named(L"IsWindowActive") || named(L"IsShowCursor")) {
        if ((error=count(1,1))) return error;
        if (named(L"GetOptionalFuncFlag")) return result(window_->GetOptionalFlags());
        if (named(L"IsWindowActive")) return result(window_->IsWindowActive() ? -1 : 0);
        return result(window_->IsShowCursor() ? -1 : 0);
    }
    if (named(L"SetOptionalFuncFlag") || named(L"ChangeCooperationLevel") || named(L"SetChangeDisplayModeFlag") || named(L"ShowCursor")) {
        if ((error=count(named(L"SetOptionalFuncFlag") || named(L"ShowCursor") ? 2 : 1,2))) return error;
        int value;
        const int defaultValue = named(L"ChangeCooperationLevel") ? WindowDefaults(context).mode : -1;
        if ((error=integer(value,1,defaultValue))) return error;
        if (named(L"SetOptionalFuncFlag")) { window_->SetOptionalFlags(uint32_t(value)); return result(); }
        if (named(L"ChangeCooperationLevel")) return result(window_->ChangeCooperationLevel(static_cast<SakuraGL::Window::CooperationMode>(value)));
        if (named(L"SetChangeDisplayModeFlag")) return result(window_->EnableChangePhysicalMode(value != 0));
        return result(window_->ShowCursor(value != 0));
    }
    if (named(L"SetStereoDisplayMode") || named(L"IsSupportedStereoDisplayMode")) {
        if ((error=count(2,named(L"SetStereoDisplayMode") ? 3 : 2))) return error;
        ECSWideString method;
        int parameter;
        if ((error=context.GetArgumentAsStr(method,args,1,L"")) || (error=integer(parameter,2,0))) return error;
        return result(named(L"SetStereoDisplayMode") ? window_->SetStereoDisplayMode(method,uint32_t(parameter)) :
                      (window_->IsSupportedStereoDisplayMode(method) ? -1 : 0));
    }
    if (named(L"GetDisplaySize")) {
        if ((error=count(1,1))) return error;
        SakuraGL::SGLSize size;
        const auto status = window_->GetDisplaySize(size);
        if (status) return static_cast<ESLError>(status);
        auto *out=context.CreateUserStructure(L"Size");
        if (!out) return ESLErrorMsg("Missing Size class");
        out->SetMemberAsInt(L"w",size.w); out->SetMemberAsInt(L"h",size.h);
        return context.PushObject(*out);
    }
    if (named(L"UpdateWindow") || named(L"CallMouseMove")) {
        if ((error=count(1,1))) return error;
        if (named(L"UpdateWindow")) return result(window_->UpdateWindow());
        window_->CallMouseMove(); return result();
    }
    if (named(L"ProcessUserInput")) {
        if ((error=count(1,2))) return error;
        int timeout;
        if ((error=integer(timeout,1,30))) return error;
        const auto status=window_->ProcessUserInput(timeout);
        if (status) return result(status);
        // Android dispatches UI events independently. Preserve the old wait
        // interval, waking early when a command arrives or execution stops.
        Command ignored;
        const auto wait=GetCommand(context,ignored,timeout,false);
        return result(wait == eslErrTimeout ? eslErrSuccess : wait);
    }
    if (named(L"InitWindowPosition")) {
        if ((error=count(1,5))) return error;
        int x,y,w,h;
        if ((error=integer(x,1,INT32_MIN)) || (error=integer(y,2,INT32_MIN)) ||
            (error=integer(w,3,INT32_MIN)) || (error=integer(h,4,INT32_MIN))) return error;
        SakuraGL::SGLSize size(w,h);
        return result(window_->InitWindowPosition(x,y,&size));
    }
    if (named(L"GetNormalWindowPosition") || named(L"GetPhysicalMonitorSize")) {
        const bool physical=named(L"GetPhysicalMonitorSize");
        if ((error=count(2,physical ? 2 : 3))) return error;
        auto *out=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,1,physical ? L"Size" : L"Point"));
        if (!out) return ESLErrorMsg("Window requires a Point or Size output structure");
        SakuraGL::SGLPoint position;
        SakuraGL::SGLSize size;
        if (physical) {
            size=window_->PhysicalSize();
            if (size.w <= 0 || size.h <= 0) return result(eslErrGeneral);
            out->SetMemberAsInt(L"w",size.w); out->SetMemberAsInt(L"h",size.h);
            return result();
        }
        if (window_->GetNormalWindowPosition(position,&size)) return result();
        out->SetMemberAsInt(L"x",position.x); out->SetMemberAsInt(L"y",position.y);
        auto *outSize=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,2,L"Size"));
        if (outSize) { outSize->SetMemberAsInt(L"w",size.w); outSize->SetMemberAsInt(L"h",size.h); }
        return result(-1);
    }
    if (named(L"SetExteriorBackgroundFrame")) {
        if ((error=count(8,8))) return error;
        int flags,color;
        if ((error=integer(flags,1)) || (error=integer(color,2))) return error;
        SakuraGL::SGLImageObject *images[5]{};
        for (int i=0;i<5;++i) {
            auto *resource=ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args,i+3,L"Resource"));
            if (resource) images[i]=resource->GetImage();
        }
        return result(window_->SetExteriorBackgroundFrame(flags,color,images[0],images[1],images[2],images[3],images[4]));
    }
    if (named(L"MessageBox")) {
        if ((error=count(2,4))) return error;
        ECSWideString message,caption;
        int style;
        if ((error=context.GetArgumentAsStr(message,args,1,L"")) ||
            (error=context.GetArgumentAsStr(caption,args,2,WindowDefaults(context).caption)) || (error=integer(style,3,0))) return error;
        return result(SSystem::MessageBox(message,caption,style,window_.get()));
    }
    if (named(L"CreateWindow")) {
        if ((error=count(4,5))) return error;
        ECSWideString caption;
        int w,h;
        if ((error=context.GetArgumentAsStr(caption,args,1,L"")) || (error=integer(w,2,640)) || (error=integer(h,3,480))) return error;
        if (!ValidSize(w,h)) return ESLErrorMsg("Invalid Window dimensions");
        auto *parent=ESLTypeCast<ECSWindow>(context.GetArgumentObjectAs(args,4,L"Window"));
        CloseDisplay();
        const auto status=window_->CreateWindow(caption,w,h,0,parent ? parent->GetWindow() : nullptr);
        if (!status) WindowReady();
        return result(status);
    }
    if (named(L"ChangeWindowSize")) {
        if ((error=count(3,3))) return error;
        int w,h;
        if ((error=integer(w,1)) || (error=integer(h,2))) return error;
        if (!ValidSize(w,h)) return ESLErrorMsg("Invalid Window dimensions");
        return result(window_->ChangeWindowSize(w,h));
    }
    if (named(L"SetLayeredWindow")) return ESLErrorMsg("Android Window cannot become a desktop layered window");
    if (named(L"SetWindowLayout")) {
        if ((error=count(2,4))) return error;
        int flags,x,y;
        if ((error=integer(flags,1)) || (error=integer(x,2)) || (error=integer(y,3))) return error;
        return result(window_->SetWindowLayout(flags,x,y));
    }
    if (named(L"EnableCommandQueue") || named(L"FlushCommandQueue")) {
        const bool flush=named(L"FlushCommandQueue");
        if ((error=count(1,flush ? 3 : 2))) return error;
        int enabled,priority;
        if ((error=integer(enabled,1,-1)) || (flush && (error=integer(priority,2,10)))) return error;
        std::lock_guard<std::mutex> guard(queueMutex_);
        queueEnabled_=enabled != 0;
        if (flush) commands_.erase(std::remove_if(commands_.begin(),commands_.end(),
            [priority](const Command &cmd) { return cmd.priority <= priority; }),commands_.end());
        return result();
    }
    if (named(L"QueueCommand")) {
        if ((error=count(2,6))) return error;
        ECSWideString id;
        int notification,parameter,priority,overwrite;
        if ((error=context.GetArgumentAsStr(id,args,1,L"")) || (error=integer(notification,2)) ||
            (error=integer(parameter,3)) || (error=integer(priority,4)) || (error=integer(overwrite,5))) return error;
        QueueCommand(id,notification,parameter,priority,overwrite != 0);
        return result();
    }
    if (named(L"GetCommand")) {
        if ((error=count(3,4))) return error;
        auto *out=ESLTypeCast<ECSStructure>(context.GetArgumentObjectAs(args,1,L"WndSpriteCmd"));
        if (!out) return ESLErrorMsg("GetCommand requires WndSpriteCmd output");
        int timeout,remove;
        if ((error=integer(timeout,2,-1)) || (error=integer(remove,3,1))) return error;
        Command cmd;
        const auto status=GetCommand(context,cmd,timeout,remove != 0);
        if (!status) {
            out->SetMemberAsStr(L"strID",cmd.id.c_str()); out->SetMemberAsStr(L"strFullID",cmd.fullID.c_str());
            out->SetMemberAsInt(L"nNotification",cmd.notification); out->SetMemberAsInt(L"nParameter",cmd.parameter);
        }
        return result(status);
    }
    if (named(L"Lock")) {
        if ((error=count(1,2))) return error;
        int timeout;
        if ((error=integer(timeout,1,-1))) return error;
        const auto until=Clock::now()+std::chrono::milliseconds(timeout < 0 ? INT32_MAX : timeout);
        do {
            const auto status=window_->Lock(std::min<int64_t>(33,timeout < 0 ? 33 : std::max<int64_t>(0,
                std::chrono::duration_cast<std::chrono::milliseconds>(until-Clock::now()).count())));
            if (!status) return result();
            if (context.GetStatus() != ECSContext::xsExecution) return result(eslErrAbort);
        } while (Clock::now() < until);
        return result(eslErrTimeout);
    }
    if (named(L"Unlock")) {
        if ((error=count(1,1))) return error;
        return result(window_->Unlock());
    }
    if (named(L"FreezePaint") || named(L"UnfreezePaint")) {
        if ((error=count(1,1))) return error;
        if (named(L"FreezePaint")) window_->BeginPaintFreeze();
        else if (!window_->EndPaintFreeze())
            return ESLErrorMsg("UnfreezePaint without matching FreezePaint");
        return result();
    }
    if (named(L"SyncTimePaint")) {
        if ((error=count(2,2))) return error;
        int timeout;
        if ((error=integer(timeout,1,16))) return error;
        return result(window_->SyncPaint(context,timeout));
    }
    if (named(L"AsyncTimePaint")) {
        if ((error=count(1,1))) return error;
        window_->AsyncPaint(); return result();
    }
    return ESLErrorMsg("Unimplemented Window method");
}


bool CheckLegacyWindowCommandQueue() {
    auto check=[](bool value,const char* stage){
        study::platform::LogPrint(value?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady",
            "Legacy Window command probe %s: %s",value?"OK":"FAIL",stage);return value;
    };
    {
        ECSWindow frozen;
        auto& bridge = *frozen.window_;
        const auto original = bridge.GetOptionalFlags();
        bridge.BeginPaintFreeze();
        bridge.BeginPaintFreeze();
        if (!check(bridge.GetOptionalFlags() == original && IsPaintFrozen(&bridge),
            "nested paint freeze preserves the actual SDK window flags")) return false;
        std::mutex returnedMutex;
        std::condition_variable returnedChanged;
        bool returned = false;
        SSystem::Lock();
        std::thread draw([&] {
            StudySteadyDrawAndroidWindow(&bridge);
            std::lock_guard<std::mutex> guard(returnedMutex);
            returned = true;
            returnedChanged.notify_one();
        });
        bool returnedWhileUILocked;
        {
            std::unique_lock<std::mutex> guard(returnedMutex);
            returnedWhileUILocked = returnedChanged.wait_for(guard, std::chrono::seconds(1), [&] {return returned;});
        }
        SSystem::Unlock();
        draw.join();
        if (!check(returnedWhileUILocked,
            "frozen JNI draw returns on another thread while the script owns the SDK UI mutex")) return false;
        const auto changed = original ^ uint64_t(SakuraGL::Window::flagBlackBack);
        bridge.SetOptionalFlags(changed);
        if (!check(bridge.EndPaintFreeze() && bridge.GetOptionalFlags() == changed && IsPaintFrozen(&bridge),
            "flag changes and the first unfreeze preserve the remaining freeze")) return false;
        if (!check(bridge.EndPaintFreeze() && !IsPaintFrozen(&bridge) &&
            bridge.GetOptionalFlags() == changed && !bridge.EndPaintFreeze(),
            "final unfreeze resumes drawing and rejects an unmatched unfreeze")) return false;
        bridge.BeginPaintFreeze();
        bridge.BeginPaintFreeze();
        frozen.CloseDisplay();
        if (!check(!IsPaintFrozen(&bridge) && bridge.GetOptionalFlags() == changed && !bridge.EndPaintFreeze(),
            "closing a frozen display removes every nested freeze without altering flags")) return false;
    }
    ECSWindow window;window.queueEnabled_=true;
    SakuraGL::SGLSpriteButton button;button.SetID(L"ID_TOUCH_QUEUE");
    button.SetButtonSize(SakuraGL::SGLSize(64,32));
    auto buttonStyle=button.GetButtonStyle();buttonStyle.flagHitRect=true;button.SetButtonStyle(buttonStyle);
    if(!check(!button.InvokeCommands(L"<status_notification parameter=\"1\"/>"),"actual native button status notification"))return false;
    window.NativeSprite().AddChild(&button);
    struct Detach {ECSWindow& window;SakuraGL::SGLSpriteButton& button;~Detach(){window.NativeSprite().DetachChild(&button);}} detach{window,button};
    constexpr auto touch=SakuraGL::SGLMouseInterface::TouchFlag;
    {
        PointerCommandScope dispatch(&window);
        button.OnLButtonDown(10,10,touch);
    }
    // Match the script consuming all Down notifications before the finger lifts.
    window.commands_.clear();
    {
        PointerCommandScope dispatch(&window);
        button.OnLButtonUp(10,10,touch);
    }
    {
        PointerCommandScope dispatch(&window);
        button.OnMouseLeave(touch);
    }
    int clicks=0,statuses=0;
    for(const auto& command:window.commands_)if(command.id==L"ID_TOUCH_QUEUE") {
        if(command.parameter==0)++clicks;else if(command.parameter==1)++statuses;
    }
    if(!check(clicks==1&&statuses==1,"touch Up and immediate Leave preserve exactly one click and latest status"))return false;
    window.commands_.clear();
    window.window_->InstallInputProxies();
    window.NativeSprite().SetVisible(true);button.SetVisible(true);
    auto* mouse=window.window_->GetMouseInterface();
    if(!check(button.IsHitSprite(10,10),"fixture native button has a real rectangular hit area"))return false;
    mouse->OnMouseMove(window.window_.get(),10,10,touch);
    mouse->OnButtonDown(window.window_.get(),10,10,touch);
    window.commands_.clear();
    for(int i=0;i<3;++i)window.window_->CallMouseMove();
    if(!check(window.window_->HasReplayPointer(),"held touch remains available to script mouse refresh"))return false;
    mouse->OnButtonUp(window.window_.get(),10,10,touch);
    mouse->OnMouseLeave(window.window_.get(),touch);
    clicks=0;for(const auto& command:window.commands_)if(command.id==L"ID_TOUCH_QUEUE"&&command.parameter==0)++clicks;
    if(!check(clicks==1,"actual listener Down, script refresh, Up/Leave preserves active button and click"))return false;
    const auto afterUp=window.commands_.size();window.window_->CallMouseMove();
    if(!check(!window.window_->HasReplayPointer()&&window.commands_.size()==afterUp,"lifted finger is not replayed as mouse hover"))return false;
    window.commands_.clear();
    // Desktop mouse Up has the same click-then-status ordering as touch.
    // Leave all Up commands queued, then refresh hover and leave before the
    // script reads them: the click must survive every overwriteable status.
    mouse->OnMouseMove(window.window_.get(),10,10,0);
    mouse->OnButtonDown(window.window_.get(),10,10,0);
    window.commands_.clear();
    mouse->OnButtonUp(window.window_.get(),10,10,0);
    clicks=0;statuses=0;
    for(const auto& command:window.commands_)if(command.id==L"ID_TOUCH_QUEUE") {
        if(command.parameter==0)++clicks;else if(command.parameter==1)++statuses;
    }
    if(!check(clicks==1&&statuses==1,"actual desktop mouse Up keeps click and following focus status"))return false;
    for(int i=0;i<3;++i)window.window_->CallMouseMove();
    mouse->OnMouseLeave(window.window_.get(),0);
    clicks=0;statuses=0;int64_t latestStatus=-1;
    for(const auto& command:window.commands_)if(command.id==L"ID_TOUCH_QUEUE") {
        if(command.parameter==0)++clicks;
        else if(command.parameter==1){++statuses;latestStatus=command.notification;}
    }
    if(!check(clicks==1&&statuses==1&&latestStatus==SakuraGL::SGLSpriteButton::statusNormal,
        "desktop hover refresh and Leave preserve one click and only the latest status"))return false;
    window.commands_.clear();
    {
        ECSInputFilter input;
        if(!check(!input.OpenFilter(&window,0),"attach actual input filter for logical cursor move"))return false;
        if(!check(!input.MoveCursorPosition(10,10),"explicit logical cursor moves through real native hover"))return false;
        double x=0,y=0;input.GetCursorPosition(x,y);
        if(!check(x==10&&y==10&&window.window_->HasReplayPointer()&&!window.commands_.empty(),
            "logical cursor getter, Window replay and button hover agree"))return false;
        window.window_->CallMouseMove();
        if(!check(!input.MoveCursorPosition(200,200),"logical cursor can leave button"))return false;
        input.GetCursorPosition(x,y);
        if(!check(x==200&&y==200,"moved cursor uses the same client coordinates"))return false;
        input.CloseFilter();
    }
    window.commands_.clear();
    {
        PointerCommandScope dispatch(&window);
        window.QueueCommand(L"ID_SCRIPT_OVERWRITE",0,0,0,false);
        window.QueueCommand(L"ID_SCRIPT_OVERWRITE",0,99,0,true);
    }
    if(!check(window.commands_.size()==1&&window.commands_.front().parameter==99,
        "public script overwrite still replaces a different parameter during pointer dispatch"))return false;
    {
        PointerCommandScope outer(&window);
        if(!check(pointerCommandWindow==&window,"pointer scope active"))return false;
        {PointerCommandScope nonPointer(nullptr);if(!check(pointerCommandWindow==nullptr,"nested non-pointer dispatch resets pointer policy"))return false;}
        if(!check(pointerCommandWindow==&window,"nested scope restores prior pointer policy"))return false;
    }
    if(!check(pointerCommandWindow==nullptr,"pointer policy never leaks beyond callback"))return false;
    window.commands_.clear();window.NotifyCommand(L"ID_NONTOUCH",0,0,0,false);window.NotifyCommand(L"ID_NONTOUCH",0,1,0,true);
    if(!check(window.commands_.size()==1&&window.commands_.front().parameter==1,"native overwrite outside pointer dispatch retains previous semantics"))return false;
    {
        button.SetVisible(false);
        SakuraGL::SGLImage source;
        if(!check(!source.CreateImage(2,2,SakuraGL::formatImageARGB,32)&&
            source.GetImageSize()==SakuraGL::SGLSize(2,2),"window snapshot source image"))return false;
        source.FillImage(SakuraGL::SGLPalette(0));
        class GeneratedImage final:public SakuraGL::SGLSprite {
            SakuraGL::SGLImage& source_;
        public:
            uint32_t nextColor=0xffbc3412;bool dirty=true;
            unsigned prepared=0,finished=0,advanced=0;
            explicit GeneratedImage(SakuraGL::SGLImage& source):source_(source){}
            void PrepareDrawFrame() override {
                ++prepared;
                if(dirty){source_.FillImage(SakuraGL::SGLPalette(nextColor));dirty=false;NotifyUpdate();}
                SGLSprite::PrepareDrawFrame();
            }
            void FinishDrawFrame() override {++finished;SGLSprite::FinishDrawFrame();}
            void AdvanceTime(uint32_t ms) override {++advanced;SGLSprite::AdvanceTime(ms);}
        } child(source);
        child.AttachImage(&source);child.SetPosition(3,2);child.SetVisible(true);
        window.NativeSprite().AddChild(&child);
        struct DetachImage {ECSWindow& window;SakuraGL::SGLSprite& child;
            ~DetachImage(){window.NativeSprite().DetachChild(&child);}} imageScope{window,child};
        if(!check(!window.RefreshSnapshot(16,8),"render actual window tree to independent screenshot"))return false;
        SakuraGL::SGLPalette pixel;
        if(!check(window.GetImage()&&!window.GetImage()->GetPixelRGBA(pixel,3,2)&&pixel.ui32==0xffbc3412,
            "window Resource image contains positioned scene pixels"))return false;
        child.nextColor=0xff1234bc;child.dirty=true;
        if(!check(!window.GetImage()->GetPixelRGBA(pixel,3,2)&&pixel.ui32==0xffbc3412,
            "repeated Resource reads retain one consistent screenshot"))return false;
        if(!check(!window.RefreshSnapshot(16,8)&&!window.GetImage()->GetPixelRGBA(pixel,3,2)&&pixel.ui32==0xff1234bc&&
            child.GetParent()==&window.NativeSprite()&&!window.NativeSprite().GetFrameBuffer()&&
            child.prepared==2&&child.finished==2&&child.advanced==0,
            "snapshot prepares new generated pixels without advancing time, reparenting or buffering live scene"))return false;
    }
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy Window command probe PASS: actual mouse/touch Up/Leave, preserved clicks and latest status, script overwrite and nested dispatch scopes");
    return true;
}
