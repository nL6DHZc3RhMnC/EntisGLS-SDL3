#pragma once

#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_log.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include "platform/sdl/gl_window_lease.h"

namespace studysteady::motion {

class CurrentGlContextScope {
    study::platform::sdl::GlWindowLease parentLease_;
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
    bool restored_ = false;
public:
    CurrentGlContextScope() {
        if (!SDL_IsMainThread()) throw std::runtime_error("SDL GL context access requires the main thread");
        window_ = SDL_GL_GetCurrentWindow();
        context_ = SDL_GL_GetCurrentContext();
        parentLease_ = study::platform::sdl::FindGlWindowLease(window_);
        if (parentLease_ && parentLease_->context() != context_) parentLease_.reset();
    }
    void restore() {
        if (restored_) return;
        // A logically closed SDK window may stay physically alive for a shared
        // renderer. Do not restore it as the application's active host context.
        if (parentLease_ && parentLease_->closing()) { window_ = nullptr; context_ = nullptr; }
        if (SDL_GL_GetCurrentContext() != context_ || SDL_GL_GetCurrentWindow() != window_) {
            if (!SDL_GL_MakeCurrent(window_, context_)) throw std::runtime_error(std::string("Restore host GL context: ") + SDL_GetError());
        }
        restored_ = true;
    }
    ~CurrentGlContextScope() {
        try { restore(); }
        catch (const std::exception& error) { SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", error.what()); }
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
        try { restore.restore(); }
        catch (...) { SDL_GL_DestroyContext(context_); context_ = nullptr; throw; }
    }
    ~RuntimeGl() {
        if (context_ && !SDL_GL_DestroyContext(context_)) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Destroy motion GL context: %s", SDL_GetError());
    }
    void makeCurrent() {
        if (!SDL_IsMainThread() || !SDL_GL_MakeCurrent(window_, context_)) throw std::runtime_error(std::string("Activate motion SDL GL context: ") + SDL_GetError());
    }
};
} // namespace studysteady::motion
#else
#include "runtime_egl.h"
namespace studysteady::motion { using RuntimeGl = RuntimeEgl; }
#endif
