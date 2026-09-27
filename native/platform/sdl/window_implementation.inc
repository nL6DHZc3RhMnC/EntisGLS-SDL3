// Included by the generated SDK window translation unit. The original SDK's
// layout, frame production and input-handler contracts remain in that unit.
#include "platform/sdl/window.h"
#include "platform/sdl/main_thread.h"
#include "platform/sdl/gl_window_lease.h"
#include "platform/sdl/presentation.h"
#include "platform/sdl/system.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

namespace {
void (*frameObserver)(SDL_Window*, void*) = nullptr;
void* frameObserverUser = nullptr;
bool (*windowDrawHandler)(SakuraGL::SGLGenericWindow*) = nullptr;
struct SDLPendingEvent {
    SDL_Event event{};
    std::string text;
    explicit SDLPendingEvent(const SDL_Event& source) : event(source) {
        if (source.type == SDL_EVENT_TEXT_INPUT && source.text.text) text = source.text.text;
    }
    SDL_Event Get() const {
        auto value = event;
        if (value.type == SDL_EVENT_TEXT_INPUT) value.text.text = text.c_str();
        return value;
    }
};
struct SDLRenderTask {
    SSystem::SProcedure* procedure = nullptr;
    std::shared_ptr<std::promise<SGLError>> completed;
};
struct SDLWindowState {
    study::platform::sdl::GlWindowLease glLease;
    study::platform::sdl::WindowFramePacer framePacer;
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    bool active = true;
    bool background = false;
    bool closing = false, executing = false, drainingEvents = false;
    std::map<SDL_FingerID, int> fingers;
    std::set<int> pressedKeys;
    std::deque<SDLPendingEvent> events;
    std::deque<SDLRenderTask> tasks;
};
std::map<SGLGenericWindow*, std::shared_ptr<SDLWindowState>> sdlWindows;
std::vector<std::shared_ptr<SDLWindowState>> retiredWindows;
SDLWindowState* StateFor(SGLGenericWindow* window) {
    const auto found = sdlWindows.find(window);
    return found == sdlWindows.end() ? nullptr : found->second.get();
}
std::string WindowUTF8(const wchar_t* text) {
    const auto value = SString(text ? text : L"").ToUTF8();
    return {reinterpret_cast<const char*>(value.GetConstArray()), value.GetLength()};
}
template<class Work> auto WindowUnlocked(SSystem::SMutex* mutex, Work&& work) -> decltype(work()) {
    if (SDL_IsMainThread()) return std::forward<Work>(work)();
    // Rendering callbacks need this same recursive UI lock. Never wait on the
    // main thread while retaining it on a script/media worker.
    const auto locks = mutex ? mutex->UnlockAll() : 0;
    const auto systemLocks = SSystem::UnlockAll();
    struct Restore {SSystem::SMutex* mutex; atomic_int_t locks, systemLocks;
        ~Restore() {SSystem::Relock(systemLocks); if (mutex) mutex->Relock(locks);}} restore{mutex, locks, systemLocks};
    return std::forward<Work>(work)();
}
template<class Work> auto WindowMain(SSystem::SMutex* mutex, Work&& work) -> decltype(work()) {
    if (SDL_IsMainThread()) return std::forward<Work>(work)();
    return WindowUnlocked(mutex, [&]() -> decltype(work()) {
        return study::platform::sdl::RunOnMainThreadSync(std::forward<Work>(work));
    });
}
template<class Work> SGLError WindowMainRetry(SSystem::SMutex* mutex, Work&& work) {
    const bool callerMain = SDL_IsMainThread();
    auto perform = [&] {
        for (;;) {
            bool retry = false;
            const auto result = study::platform::sdl::RunOnMainThreadSync([&] { return work(retry, callerMain); });
            if (!retry || callerMain) return result;
            SDL_Delay(1); // Only the requesting worker waits for a busy UI lock.
        }
    };
    return callerMain ? perform() : WindowUnlocked(mutex, perform);
}
struct SDLUILock {
    SSystem::SMutex* mutex;
    bool locked;
    explicit SDLUILock(SSystem::SMutex* value) : mutex(value), locked(value && value->Lock(0) == errSuccess) {}
    ~SDLUILock() {if (locked) mutex->Unlock();}
};
SGLError ExecuteTask(SDLWindowState& state, SProcedure* procedure) {
    // "NoRender" SDK tasks can upload/delete GL resources. They still need a
    // current context even though they do not present a window frame.
    if (!SDL_GL_MakeCurrent(state.window, state.context)) return sglErrFailed;
    const bool previous = state.executing;
    state.executing = true;
    bool prepared = false;
    SGLError result = sglErrSuccess;
    try {
        procedure->Prepare(); prepared = true;
        procedure->Run(); prepared = false;
        procedure->Finalize();
    } catch (...) {
        if (prepared) {try {procedure->Finalize();} catch (...) {}}
        Trace("SDL rendering task threw an exception\n");
        result = sglErrFailed;
    }
    state.executing = previous;
    return result;
}
void DrainTasks(SDLWindowState& state) {
    while (!state.tasks.empty()) {
        const auto task = state.tasks.front();
        state.tasks.pop_front();
        const auto result = ExecuteTask(state, task.procedure);
        if (task.completed) task.completed->set_value(result);
        else if (result != sglErrSuccess) Trace("SDL asynchronous rendering task failed\n");
    }
}
int VirtualKey(const SDL_KeyboardEvent& key) {
    if (key.scancode >= SDL_SCANCODE_A && key.scancode <= SDL_SCANCODE_Z)
        return 'A' + int(key.scancode - SDL_SCANCODE_A);
    if (key.key >= SDLK_0 && key.key <= SDLK_9) return int(key.key);
    if (key.scancode >= SDL_SCANCODE_F1 && key.scancode <= SDL_SCANCODE_F12)
        return vkeyFunction1 + int(key.scancode - SDL_SCANCODE_F1);
    switch (key.key) {
    case SDLK_RETURN: case SDLK_KP_ENTER: return vkeyReturn;
    case SDLK_ESCAPE: return vkeyEscape;
    case SDLK_BACKSPACE: return vkeyBack;
    case SDLK_TAB: return vkeyTab;
    case SDLK_SPACE: return ' ';
    case SDLK_LEFT: return vkeyLeft; case SDLK_RIGHT: return vkeyRight;
    case SDLK_UP: return vkeyUp; case SDLK_DOWN: return vkeyDown;
    case SDLK_LCTRL: case SDLK_RCTRL: return vkeyControl;
    case SDLK_LSHIFT: case SDLK_RSHIFT: return vkeyShift;
    case SDLK_LALT: case SDLK_RALT: return vkeyMenu;
    case SDLK_HOME: return vkeyHome; case SDLK_END: return vkeyEnd;
    case SDLK_PAGEUP: return vkeyPageUp; case SDLK_PAGEDOWN: return vkeyPageDown;
    case SDLK_DELETE: return vkeyDelete; case SDLK_INSERT: return vkeyInsert;
    default: return 0;
    }
}
}

SGLError SGLGenericWindow::CreateDisplay(const wchar_t* title, Window::CooperationMode mode,
        uint32_t width, uint32_t height, uint32_t depth, uint32_t frequency) {
    if (m_flagCreated) return sglErrFailed;
    m_flagModeDisplay = true; m_modeCooperation = mode;
    m_flagFullscreen = mode == Window::modeFullScreen || mode == Window::modeExclusive;
    m_flagLayeredWindow = false; m_nBitsPerPixel = depth; m_nFrequency = frequency;
    return CreateWindowSimply(title, width, height);
}

SGLError SGLGenericWindow::CreateWindowSimply(const wchar_t* title, uint32_t width, uint32_t height) {
    if (m_flagCreated || !width || !height) return sglErrFailed;
    m_sizeVirtual = SGLSize(width, height); m_sizeLayoutOriginal = m_sizeVirtual;
    m_strCaption = title;
    try {
        return WindowMainRetry(m_pMutexUI, [&](bool& retry, bool callerMain) -> SGLError {
            SDLUILock lock(m_pMutexUI);
            if (!lock.locked) { retry = !callerMain; return sglErrFailed; }
            auto state = std::make_shared<SDLWindowState>();
#if defined(STUDYSTEADY_GL_ES)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            const auto caption = WindowUTF8(title);
            const int initialWidth = std::min<int>(width, 1280);
            const int initialHeight = std::max(1, int(double(height) * initialWidth / width));
            const SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                SDL_WINDOW_HIGH_PIXEL_DENSITY | (m_flagFullscreen ? SDL_WINDOW_FULLSCREEN : 0);
            state->window = SDL_CreateWindow(caption.c_str(), initialWidth, initialHeight, flags);
            if (!state->window) {Trace("SDL window: %s\n", SDL_GetError()); return sglErrFailed;}
            state->context = SDL_GL_CreateContext(state->window);
            if (!state->context) {
                Trace("SDL GL context: %s\n", SDL_GetError()); SDL_DestroyWindow(state->window);
                return sglErrFailed;
            }
            if (!study::platform::sdl::ConfigureImmediatePresentation()) {
                Trace("SDL immediate presentation unavailable: %s\n", SDL_GetError());
                SDL_GL_DestroyContext(state->context); SDL_DestroyWindow(state->window);
                return sglErrFailed;
            }
            int pixelWidth = 0, pixelHeight = 0;
            if (!SDL_GetWindowSizeInPixels(state->window, &pixelWidth, &pixelHeight) || pixelWidth <= 0 || pixelHeight <= 0) {
                SDL_GL_DestroyContext(state->context); SDL_DestroyWindow(state->window); return sglErrFailed;
            }
            state->glLease = study::platform::sdl::RegisterGlWindow(state->window, state->context);
            m_sizePhysicalDisplay = SGLSize(pixelWidth, pixelHeight);
            m_rectWindow = SGLImageRect(0, 0, pixelWidth, pixelHeight);
            study::platform::sdl::SetDialogParent(state->window);
            sdlWindows[this] = std::move(state);
            AddWindowToChain(); m_flagCreated = true;
            if (auto* view = m_wvfFramework.GetView()) view->OnAttachedWindow(this);
            UpdateWindowLayout();
            return sglErrSuccess;
        });
    } catch (const std::exception& error) {
        Trace("SDL create window: %s\n", error.what()); return sglErrFailed;
    }
}

SGLError SGLGenericWindow::CloseWindowSimply() {
    if (!m_flagCreated) return sglErrFailed;
    try {
        return WindowMainRetry(m_pMutexUI, [&](bool& retry, bool callerMain) -> SGLError {
            const auto found = sdlWindows.find(this);
            if (found == sdlWindows.end()) { m_flagCreated = false; return sglErrSuccess; }
            const auto state = found->second;
            SDLUILock lock(m_pMutexUI);
            const bool busy = !lock.locked || state->executing;
            if (busy && !callerMain) { retry = true; return sglErrFailed; }
            if (busy || !SDL_GL_MakeCurrent(state->window, state->context)) {
                // An unguarded main-thread destructor cannot wait for a worker
                // or leave its raw SDK pointer in the next frame's registry.
                // Complete every pending synchronous request with failure.
                Trace("SDL close: retiring unavailable window (busy=%d); SDK detach callback was not completed\n", int(busy));
                state->closing = true;
                while (!state->tasks.empty()) {
                    const auto task = state->tasks.front(); state->tasks.pop_front();
                    if (task.completed) {
                        try { task.completed->set_value(sglErrFailed); }
                        catch (...) { Trace("SDL close: rendering request was already completed\n"); }
                    } else Trace("SDL close: canceled pending asynchronous rendering request\n");
                }
                state->events.clear();
                DetachWindowFromChain(); m_flagCreated = false;
                SDL_StopTextInput(state->window); SDL_HideWindow(state->window);
                study::platform::sdl::UnregisterGlWindow(state->window);
                // Keep GL alive while the caller finishes destroying its SDK
                // members. This list contains no SDK window pointer.
                retiredWindows.push_back(state);
                sdlWindows.erase(this);
                study::platform::sdl::SetDialogParent(sdlWindows.empty() ? nullptr : sdlWindows.begin()->second->window);
                return sglErrFailed;
            }
            state->closing = true;
            DrainTasks(*state);
            if (!StateFor(this)) return sglErrFailed;
            if (auto* parent = ESLTypeCast<SGLGenericWindow>(m_refParentWnd.GetReference())) {
                const auto child = parent->m_arrChildren.FindPtr(this);
                if (child >= 0) parent->m_arrChildren.RemoveAt(child);
            }
            m_refParentWnd = nullptr;
            DetachWindowFromChain();
            if (auto* view = m_wvfFramework.GetView()) view->OnDetachedWindow(this);
            m_flagCreated = false;
            SDL_StopTextInput(state->window);
            SDL_HideWindow(state->window);
            study::platform::sdl::UnregisterGlWindow(state->window);
            // The state releases its lease when removed; motion renderers keep
            // the physical parent window/context alive until their own teardown.
            sdlWindows.erase(this);
            study::platform::sdl::SetDialogParent(sdlWindows.empty() ? nullptr : sdlWindows.begin()->second->window);
            return sglErrSuccess;
        });
    } catch (const std::exception& error) {
        Trace("SDL close window: %s\n", error.what()); return sglErrFailed;
    }
}

void SGLGenericWindow::SetOptionalFlags(uint64_t flags) {
    m_flagsOption = flags & ~(flagDoMinimize | flagDoMaximize | flagDoNormalize | flagOpenIME);
    WindowMain(m_pMutexUI, [&] {
        if (auto* state = StateFor(this)) {
            bool success = true;
            if (flags & flagDoMinimize) success &= SDL_MinimizeWindow(state->window);
            if (flags & flagDoMaximize) success &= SDL_MaximizeWindow(state->window);
            if (flags & flagDoNormalize) success &= SDL_RestoreWindow(state->window);
            if (flags & (flagOpenIME | flagEnableIME)) success &= SDL_StartTextInput(state->window);
            else success &= SDL_StopTextInput(state->window);
            success &= flags & flagInvisibleWindow ? SDL_HideWindow(state->window) : SDL_ShowWindow(state->window);
            if (!success) Trace("SDL optional window flags: %s\n", SDL_GetError());
        }
    });
}
SGLError SGLGenericWindow::ChangeCooperationLevel(Window::CooperationMode mode) {
    const bool fullscreen = mode == Window::modeFullScreen || mode == Window::modeExclusive;
    const bool changed = WindowMain(m_pMutexUI, [&] {
        auto* state = StateFor(this); return state && SDL_SetWindowFullscreen(state->window, fullscreen);
    });
    if (!changed) return sglErrFailed;
    m_modeCooperation = mode; m_flagFullscreen = fullscreen; return sglErrSuccess;
}
SGLError SGLGenericWindow::PostUpdate(const SGLImageRect*) {
    if (!m_flagCreated) return sglErrFailed;
    SDL_Event wake{}; wake.type = SDL_EVENT_USER;
    return SDL_PushEvent(&wake) ? sglErrSuccess : sglErrFailed;
}
SGLError SGLGenericWindow::UpdateWindow(Window::UpdateParameter* update) {
    if (!m_flagCreated) return sglErrFailed;
    const auto started = SSystem::CurrentMilliSec();
    return WindowUnlocked(m_pMutexUI, [&]() -> SGLError {
        m_signalDonePaint.ResetSignal();
        SGLError result;
        if (SDL_IsMainThread()) {
            DrawSDLFrame();
            result = static_cast<SGLError>(m_signalDonePaint.Wait(0));
        } else {
            result = PostUpdate(nullptr);
            if (!result) result = static_cast<SGLError>(m_signalDonePaint.Wait(100));
        }
        if (update) update->WaitFrame(SSystem::CurrentMilliSec() - started, 0);
        return result;
    });
}
SGLError SGLGenericWindow::PostRenderingThread(SSystem::SProcedure* procedure, PostThreadType type) {
    if (!m_flagCreated || !procedure) return sglErrFailed;
    if (type != postNormal && type != postDelay && type != postAsyncNoRender) return sglErrNotSupported;
    try {
        return WindowUnlocked(m_pMutexUI, [&]() -> SGLError {
            auto completed = type == postNormal ? std::make_shared<std::promise<SGLError>>() : nullptr;
            auto future = completed ? completed->get_future() : std::future<SGLError>{};
            const bool alreadyMain = SDL_IsMainThread();
            const auto accepted = study::platform::sdl::RunOnMainThreadSync([&]() -> SGLError {
                auto* state = StateFor(this);
                if (!state || state->closing) return sglErrFailed;
                if (alreadyMain && type == postNormal) {
                    SDLUILock lock(m_pMutexUI);
                    if (!lock.locked) return sglErrFailed;
                    completed->set_value(ExecuteTask(*state, procedure));
                } else {
                    // Only enqueue in the SDL dispatcher. A worker retaining
                    // another UI lock must never make this callback block all
                    // other main-thread requests. DrawSDLFrame drains with a
                    // nonblocking lock, even while the window is backgrounded.
                    state->tasks.push_back({procedure, completed});
                }
                return sglErrSuccess;
            });
            if (accepted != sglErrSuccess) return accepted;
            return completed ? future.get() : sglErrSuccess;
        });
    } catch (const std::exception& error) {Trace("SDL rendering task: %s\n", error.what()); return sglErrFailed;}
}
SGLError SGLGenericWindow::PostUIThread(SSystem::SProcedure* procedure) {
    return PostRenderingThread(procedure, postNormal);
}
bool SGLGenericWindow::IsWindowActive() {
    return WindowMain(m_pMutexUI, [&] {auto* s = StateFor(this); return s && s->active && !s->background;});
}
SGLError SGLGenericWindow::SetWindowCaption(const wchar_t* text) {
    m_strCaption = text;
    return WindowMain(m_pMutexUI, [&] {auto* s = StateFor(this);
        return s && SDL_SetWindowTitle(s->window, WindowUTF8(text).c_str()) ? sglErrSuccess : sglErrFailed;});
}
SGLError SGLGenericWindow::ShowCursor(bool show) {
    return WindowMain(m_pMutexUI, [&] {return (show ? SDL_ShowCursor() : SDL_HideCursor()) ? sglErrSuccess : sglErrFailed;});
}
bool SGLGenericWindow::IsShowCursor() {return WindowMain(m_pMutexUI, [] {return SDL_CursorVisible();});}
SGLError SGLGenericWindow::SetCursor(const wchar_t*) {return sglErrNotSupported;}
SGLError SGLGenericWindow::MoveCursorPosition(int32_t x, int32_t y, int mouse) {
    if (mouse != 0) return sglErrNotSupported;
    return WindowMain(m_pMutexUI, [&]() -> SGLError {
        auto* state = StateFor(this); if (!state) return sglErrFailed;
        SDLUILock lock(m_pMutexUI); if (!lock.locked) return sglErrFailed;
        S2DDVector position(x, y); ScreenPositionFromClient(position);
        int width = 0, height = 0;
        if (!SDL_GetWindowSize(state->window, &width, &height) || !m_sizePhysicalDisplay.w || !m_sizePhysicalDisplay.h)
            return sglErrFailed;
        SDL_WarpMouseInWindow(state->window,
            float(position.x * width / m_sizePhysicalDisplay.w), float(position.y * height / m_sizePhysicalDisplay.h));
        m_idPrimaryTouch = 0; m_vPrimaryTouch = position;
        return sglErrSuccess;
    });
}
SGLError SGLGenericWindow::EnableChangePhysicalMode(bool enable) {
    // SDL fullscreen is desktop fullscreen here; a resolution-switching mode
    // must be implemented separately before advertising that capability.
    return enable ? sglErrNotSupported : sglErrSuccess;
}
int SGLGenericWindow::GetMonitorFrequency() {
    return WindowMain(m_pMutexUI, [&] {auto* s = StateFor(this); if (!s) return 0;
        const auto* mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(s->window));
        return mode ? int(mode->refresh_rate) : 0;});
}
SGLError SGLGenericWindow::AttachMenu(SGLWindowMenu*) {return sglErrNotSupported;}
SGLError SGLGenericWindow::CaptureMouse(int) {
    return WindowMain(m_pMutexUI, [] {return SDL_CaptureMouse(true) ? sglErrSuccess : sglErrFailed;});
}
SGLError SGLGenericWindow::ReleaseMouse(int) {
    return WindowMain(m_pMutexUI, [] {return SDL_CaptureMouse(false) ? sglErrSuccess : sglErrFailed;});
}
bool SGLGenericWindow::OnKeyDown(int key) {
    SSmartLock<SMutex> lock(m_pMutexUI);
    int64_t flags = 0;
    const auto mods = SDL_GetModState();
    if (mods & SDL_KMOD_SHIFT) flags |= vkeyContextShift;
    if (mods & SDL_KMOD_CTRL) flags |= vkeyContextControl;
    if (mods & SDL_KMOD_ALT) flags |= vkeyContextMenu;
    if (mods & SDL_KMOD_CAPS) flags |= vkeyContextCapital;
    return key && m_pKeyHandler && m_pKeyHandler->OnKeyDown(this, key, flags);
}
bool SGLGenericWindow::OnKeyUp(int key) {
    SSmartLock<SMutex> lock(m_pMutexUI);
    int64_t flags = 0;
    const auto mods = SDL_GetModState();
    if (mods & SDL_KMOD_SHIFT) flags |= vkeyContextShift;
    if (mods & SDL_KMOD_CTRL) flags |= vkeyContextControl;
    if (mods & SDL_KMOD_ALT) flags |= vkeyContextMenu;
    if (mods & SDL_KMOD_CAPS) flags |= vkeyContextCapital;
    return key && m_pKeyHandler && m_pKeyHandler->OnKeyUp(this, key, flags);
}

void SGLGenericWindow::OnSDLEvent(const SDL_Event& event) {
    auto* state = StateFor(this); if (!state) return;
    if (!state->drainingEvents && !state->events.empty()) {state->events.emplace_back(event); return;}
    SDLUILock lock(m_pMutexUI);
    if (!lock.locked) {state->events.emplace_back(event); return;}
    if (event.type == SDL_EVENT_WILL_ENTER_BACKGROUND || event.type == SDL_EVENT_DID_ENTER_BACKGROUND) {
        if (!state->background) {state->background = true; OnSystemEvent(SysCommandId::AppSuspend);} return;
    }
    if (event.type == SDL_EVENT_DID_ENTER_FOREGROUND) {
        if (state->background) {state->background = false; OnSystemEvent(SysCommandId::AppResume);} return;
    }
    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        if (event.window.data1 > 0 && event.window.data2 > 0) OnSurfaceChanged(event.window.data1, event.window.data2);
        return;
    }
    if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        state->active = true;
        if (m_pKeyHandler) m_pKeyHandler->OnSetFocus(this);
        OnSystemEvent(SysCommandId::WindowActive); return;
    }
    if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        state->active = false;
        for (const int key : state->pressedKeys) OnKeyUp(key);
        state->pressedKeys.clear();
        if (m_pKeyHandler) m_pKeyHandler->OnKillFocus(this);
        OnSystemEvent(SysCommandId::WindowInactive); return;
    }
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {OnSystemEvent(SysCommandId::AppExit); return;}
    if (event.type == SDL_EVENT_KEY_DOWN) {const int key = VirtualKey(event.key); if (key) state->pressedKeys.insert(key); OnKeyDown(key); return;}
    if (event.type == SDL_EVENT_KEY_UP) {const int key = VirtualKey(event.key); state->pressedKeys.erase(key); OnKeyUp(key); return;}
    if (event.type == SDL_EVENT_TEXT_INPUT) {
        SString text; text.FromUTF8(reinterpret_cast<const uint8_t*>(event.text.text));
        for (const wchar_t* c = text; *c; ++c) {
            const uint32_t point = *c;
            if (point > 0xffff) {OnChar(0xd800 + ((point - 0x10000) >> 10)); OnChar(0xdc00 + ((point - 0x10000) & 1023));}
            else OnChar(point);
        }
        return;
    }
    int width = 0, height = 0; SDL_GetWindowSize(state->window, &width, &height);
    const double sx = width ? double(m_sizePhysicalDisplay.w) / width : 1;
    const double sy = height ? double(m_sizePhysicalDisplay.h) / height : 1;
    if (event.type == SDL_EVENT_FINGER_DOWN || event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_FINGER_MOTION || event.type == SDL_EVENT_FINGER_CANCELED) {
        const auto finger = event.tfinger.fingerID;
        if (event.type == SDL_EVENT_FINGER_DOWN) {
            int next = 1; for (const auto& item : state->fingers) next = std::max(next, item.second + 1);
            state->fingers[finger] = next;
        }
        const auto found = state->fingers.find(finger); if (found == state->fingers.end()) return;
        const double x = event.tfinger.x * m_sizePhysicalDisplay.w, y = event.tfinger.y * m_sizePhysicalDisplay.h;
        if (found->second == m_idPrimaryTouch || state->fingers.size() == 1) m_vPrimaryTouch = S2DDVector(x, y);
        if (event.type == SDL_EVENT_FINGER_DOWN) OnTouchedDown(x, y, found->second);
        else if (event.type == SDL_EVENT_FINGER_UP || event.type == SDL_EVENT_FINGER_CANCELED) {OnTouchedUp(x, y, found->second); state->fingers.erase(found);}
        else OnTouchedMoved(x, y, found->second);
        return;
    }
    // Ignore SDL's synthetic mouse events after a touch, avoiding duplicate clicks.
    if ((event.type == SDL_EVENT_MOUSE_MOTION && event.motion.which == SDL_TOUCH_MOUSEID) ||
        ((event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) && event.button.which == SDL_TOUCH_MOUSEID)) return;
    if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        const double x = (event.type == SDL_EVENT_MOUSE_MOTION ? event.motion.x : event.button.x) * sx;
        const double y = (event.type == SDL_EVENT_MOUSE_MOTION ? event.motion.y : event.button.y) * sy;
        m_idPrimaryTouch = 0; m_vPrimaryTouch = S2DDVector(x, y);
        if (event.type != SDL_EVENT_MOUSE_MOTION && event.button.button == SDL_BUTTON_LEFT)
            m_flagPrimaryTouchDown = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        const int button = event.type == SDL_EVENT_MOUSE_MOTION ? 0 :
            event.button.button == SDL_BUTTON_RIGHT ? 1 : event.button.button == SDL_BUTTON_MIDDLE ? 2 : 0;
        const int64_t flags = int64_t(button) << SGLMouseInterface::ButtonIDShifter;
        SSmartLock<SMutex> lock(m_pMutexUI);
        auto dispatch = [&](SGLMouseInterface* handler, double px, double py) {
            if (!handler) return false;
            if (event.type == SDL_EVENT_MOUSE_MOTION) return handler->OnMouseMove(this, px, py, flags);
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) return handler->OnButtonDown(this, px, py, flags);
            return handler->OnButtonUp(this, px, py, flags);
        };
        if (!dispatch(m_pDirectMouseHandler, x - m_rectWindow.x, y - m_rectWindow.y)) {
            S2DDVector logical(x, y); ClientPositionFromScreen(logical);
            dispatch(m_pMouseHandler, logical.x, logical.y);
        }
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL && event.wheel.which != SDL_TOUCH_MOUSEID) {
        const double x = event.wheel.mouse_x * sx, y = event.wheel.mouse_y * sy;
        const double direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0 : 1.0;
        const int delta = int(std::lround(event.wheel.y * direction * SGLMouseInterface::WheelDeltaUnit));
        if (!m_pDirectMouseHandler || !m_pDirectMouseHandler->OnMouseWheel(this, delta, x - m_rectWindow.x, y - m_rectWindow.y, 0)) {
            S2DDVector logical(x, y); ClientPositionFromScreen(logical);
            if (m_pMouseHandler) m_pMouseHandler->OnMouseWheel(this, delta, logical.x, logical.y, 0);
        }
    }
}

void SGLGenericWindow::DrawSDLFrame() {
    const auto found = sdlWindows.find(this);
    if (found == sdlWindows.end() || !m_flagCreated) return;
    const auto state = found->second;
    // Do not block the main event dispatcher on a script-owned lock: a script
    // can be waiting for a queued main-thread task which must run next.
    SDLUILock lock(m_pMutexUI);
    if (!lock.locked) return;
    DrainTasks(*state);
    state->drainingEvents = true;
    while (!state->events.empty() && StateFor(this)) {
        const auto pending = std::move(state->events.front()); state->events.pop_front();
        OnSDLEvent(pending.Get());
    }
    state->drainingEvents = false;
    if (!StateFor(this) || state->background || state->closing || !m_flagCreated) return;
    if (!state->framePacer.IsDue()) return;
    if (SDL_GL_MakeCurrent(state->window, state->context)) {
        OnTimer();
        if (!StateFor(this) || !m_flagCreated || state->closing) return;
        if (!study::platform::sdl::IsWindowPresentable(state->window)) return;
        const bool drawn = windowDrawHandler ? windowDrawHandler(this) : (OnDraw(), true);
        if (drawn && StateFor(this) && m_flagCreated && !state->closing &&
                study::platform::sdl::IsWindowPresentable(state->window)) {
            if (frameObserver) frameObserver(state->window, frameObserverUser);
            if (!SDL_GL_SwapWindow(state->window)) Trace("SDL frame present: %s\n", SDL_GetError());
        }
    }
}

namespace study::platform::sdl {
void SetWindowDrawHandler(bool (*handler)(SakuraGL::SGLGenericWindow*)) { windowDrawHandler = handler; }
void CollectRetiredWindows() {
    retiredWindows.erase(std::remove_if(retiredWindows.begin(), retiredWindows.end(),
        [](const auto& state) { return !state->executing; }), retiredWindows.end());
}
void SetFrameObserver(void (*observer)(SDL_Window*, void*), void* user) {
    frameObserver = observer;
    frameObserverUser = user;
}
void ProcessWindowEvent(const SDL_Event& event) {
    SDL_Window* destination = SDL_GetWindowFromEvent(&event);
    const bool broadcast = event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WILL_ENTER_BACKGROUND ||
        event.type == SDL_EVENT_DID_ENTER_BACKGROUND || event.type == SDL_EVENT_DID_ENTER_FOREGROUND;
    std::vector<SGLGenericWindow*> windows;
    for (const auto& entry : sdlWindows)
        if (broadcast || destination == entry.second->window) windows.push_back(entry.first);
    for (auto* window : windows) if (StateFor(window)) window->OnSDLEvent(event);
}
void DrawWindows() {
    CollectRetiredWindows();
    std::vector<SGLGenericWindow*> windows;
    for (const auto& entry : sdlWindows) windows.push_back(entry.first);
    for (auto* window : windows) if (StateFor(window)) window->DrawSDLFrame();
}
bool HasWindows() {return !sdlWindows.empty();}
}
