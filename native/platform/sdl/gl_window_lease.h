#pragma once

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_log.h>
#include <map>
#include <memory>
#include <stdexcept>

namespace study::platform::sdl {

inline void ForgetGlWindow(SDL_Window* window);

// SDK window state owns one lease; shared renderers own additional leases.
// SDK Close hides/unregisters the window immediately, but physical GL teardown
// waits until every dependent context is destroyed on the SDL main thread.
class GlWindowOwner {
    SDL_Window* window_;
    SDL_GLContext context_;
    bool closing_ = false;
public:
    GlWindowOwner(SDL_Window* window, SDL_GLContext context) : window_(window), context_(context) {}
    GlWindowOwner(const GlWindowOwner&) = delete;
    GlWindowOwner& operator=(const GlWindowOwner&) = delete;
    ~GlWindowOwner() {
        if (!SDL_IsMainThread()) {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "GL window lease released outside the SDL main thread");
            return; // Do not call main-thread-only SDL destruction from a worker.
        }
        ForgetGlWindow(window_);
        if (context_ && !SDL_GL_DestroyContext(context_))
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Destroy leased parent GL context: %s", SDL_GetError());
        if (window_) SDL_DestroyWindow(window_);
    }
    SDL_Window* window() const { return window_; }
    SDL_GLContext context() const { return context_; }
    bool closing() const { return closing_; }
    void retire() { closing_ = true; }
};
using GlWindowLease = std::shared_ptr<GlWindowOwner>;

inline std::map<SDL_Window*, std::weak_ptr<GlWindowOwner>>& GlWindowOwners() {
    static std::map<SDL_Window*, std::weak_ptr<GlWindowOwner>> owners;
    return owners;
}

inline void ForgetGlWindow(SDL_Window* window) { GlWindowOwners().erase(window); }

inline GlWindowLease RegisterGlWindow(SDL_Window* window, SDL_GLContext context) {
    if (!SDL_IsMainThread() || !window || !context) throw std::runtime_error("Register GL window on the SDL main thread with valid handles");
    auto& owners = GlWindowOwners();
    if (const auto found = owners.find(window); found != owners.end() && !found->second.expired())
        throw std::runtime_error("SDL GL window is already registered");
    auto owner = std::make_shared<GlWindowOwner>(window, context);
    owners[window] = owner;
    return owner;
}

inline GlWindowLease FindGlWindowLease(SDL_Window* window) {
    if (!SDL_IsMainThread()) throw std::runtime_error("Acquire GL window lease on the SDL main thread");
    auto& owners = GlWindowOwners();
    const auto found = owners.find(window);
    if (found == owners.end()) return {}; // External API callers own their host lifetime.
    auto owner = found->second.lock();
    if (!owner) { owners.erase(found); return {}; }
    return owner;
}

inline GlWindowLease AcquireGlWindowLease(SDL_Window* window, SDL_GLContext context) {
    auto owner = FindGlWindowLease(window);
    if (owner && (owner->closing() || owner->context() != context))
        throw std::runtime_error("Shared GL parent is closing or differs from the registered window context");
    return owner;
}

inline void UnregisterGlWindow(SDL_Window* window) {
    if (!SDL_IsMainThread()) throw std::runtime_error("Unregister GL window on the SDL main thread");
    if (auto owner = FindGlWindowLease(window)) owner->retire();
}

} // namespace study::platform::sdl
