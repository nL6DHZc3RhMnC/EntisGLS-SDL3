#pragma once

#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_log.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include "platform/sdl/gl_window_lease.h"
#if defined(SDL_PLATFORM_IOS)
#include "platform/sdl/ios_gl_context.h"
#endif

namespace studysteady::motion {

class CurrentGlContextScope {
    study::platform::sdl::GlWindowLease parentLease_;
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
    bool restored_ = false;
#if defined(SDL_PLATFORM_IOS)
    void* nativeContext_ = nullptr;
#endif
public:
    CurrentGlContextScope() {
        if (!SDL_IsMainThread()) throw std::runtime_error("SDL GL context access requires the main thread");
        window_ = SDL_GL_GetCurrentWindow();
        context_ = SDL_GL_GetCurrentContext();
        parentLease_ = study::platform::sdl::FindGlWindowLease(window_);
        if (parentLease_ && parentLease_->context() != context_) parentLease_.reset();
#if defined(SDL_PLATFORM_IOS)
        nativeContext_ = study::platform::sdl::RetainIOSCurrentGLContext();
#endif
    }
    CurrentGlContextScope(const CurrentGlContextScope&) = delete;
    CurrentGlContextScope& operator=(const CurrentGlContextScope&) = delete;
    void restore() {
        if (restored_) return;
        // A logically closed SDK window may stay physically alive for a shared
        // renderer. Do not restore it as the application's active host context.
        if (parentLease_ && parentLease_->closing()) { window_ = nullptr; context_ = nullptr; }
        if (SDL_GL_GetCurrentContext() != context_ || SDL_GL_GetCurrentWindow() != window_) {
            if (!SDL_GL_MakeCurrent(window_, context_)) throw std::runtime_error(std::string("Restore host GL context: ") + SDL_GetError());
        }
#if defined(SDL_PLATFORM_IOS)
        // SDL's TLS still points at the host while offscreen EAGL work runs.
        // SDL_GL_MakeCurrent also returns early when that TLS already matches,
        // so restore the actual EAGL context explicitly, including nested scopes.
        const auto target = parentLease_ && parentLease_->closing() ? nullptr : nativeContext_;
        if (!study::platform::sdl::MakeIOSGLContextCurrent(target))
            throw std::runtime_error(std::string("Restore host EAGL context: ") + SDL_GetError());
#endif
        restored_ = true;
    }
    ~CurrentGlContextScope() {
        try { restore(); }
        catch (const std::exception& error) { SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", error.what()); }
#if defined(SDL_PLATFORM_IOS)
        study::platform::sdl::ReleaseIOSGLContextReference(nativeContext_);
#endif
    }
};

class RuntimeGl {
    study::platform::sdl::GlWindowLease parentLease_;
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
public:
    RuntimeGl(std::uintptr_t window, std::uintptr_t parent) {
        if (!SDL_IsMainThread()) throw std::runtime_error("Create motion GL context on the SDL main thread");
        if (!window || !parent) throw std::runtime_error("Supply a live SDL window and shared parent context");
        window_ = reinterpret_cast<SDL_Window*>(window);
        parentLease_ = study::platform::sdl::AcquireGlWindowLease(window_, reinterpret_cast<SDL_GLContext>(parent));
        CurrentGlContextScope restore;
#if defined(SDL_PLATFORM_IOS)
        context_ = reinterpret_cast<SDL_GLContext>(
            study::platform::sdl::CreateIOSSharedGLContext(reinterpret_cast<void*>(parent)));
        if (!context_) throw std::runtime_error(SDL_GetError());
#else
        if (!SDL_GL_MakeCurrent(window_, reinterpret_cast<SDL_GLContext>(parent))) throw std::runtime_error(SDL_GetError());
        int previousShare = 0;
        if (!SDL_GL_GetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, &previousShare)) throw std::runtime_error(SDL_GetError());
        if (!SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1)) throw std::runtime_error(SDL_GetError());
        context_ = SDL_GL_CreateContext(window_);
        const std::string creationError = context_ ? "" : SDL_GetError();
        const bool reset = SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, previousShare);
        if (!context_ || !reset) {
            if (context_) SDL_GL_DestroyContext(context_);
            context_ = nullptr;
            throw std::runtime_error(creationError.empty() ? SDL_GetError() : creationError);
        }
#endif
        try { restore.restore(); }
        catch (...) { destroy(); throw; }
    }
    ~RuntimeGl() { destroy(); }
    void makeCurrent() {
#if defined(SDL_PLATFORM_IOS)
        if (!study::platform::sdl::MakeIOSGLContextCurrent(context_))
            throw std::runtime_error(std::string("Activate motion EAGL context: ") + SDL_GetError());
#else
        if (!SDL_IsMainThread() || !SDL_GL_MakeCurrent(window_, context_)) throw std::runtime_error(std::string("Activate motion SDL GL context: ") + SDL_GetError());
#endif
    }
private:
    void destroy() {
#if defined(SDL_PLATFORM_IOS)
        study::platform::sdl::DestroyIOSSharedGLContext(context_);
#else
        if (context_ && !SDL_GL_DestroyContext(context_)) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Destroy motion GL context: %s", SDL_GetError());
#endif
        context_ = nullptr;
    }
};
} // namespace studysteady::motion
#else
#include "runtime_egl.h"
namespace studysteady::motion { using RuntimeGl = RuntimeEgl; }
#endif
