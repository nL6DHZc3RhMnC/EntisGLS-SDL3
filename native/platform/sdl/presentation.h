#pragma once

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

namespace study::platform::sdl {

inline bool ConfigureImmediatePresentation() {
#if defined(SDL_PLATFORM_IOS)
    // SDL's UIKit driver presents EAGL directly and implements neither swap
    // interval callback. Our WindowFramePacer still limits frames to 60 Hz.
    return SDL_GL_GetCurrentContext() != nullptr;
#else
    // The SDL event thread also services synchronous script/decoder requests.
    // It must not wait for a display-link callback inside SwapWindow (which
    // can stop arriving while Cocoa windows are hidden or occluded).
    if (!SDL_GL_SetSwapInterval(0)) return false;
    int interval = -1;
    if (!SDL_GL_GetSwapInterval(&interval)) return false;
    if (interval != 0) return SDL_SetError("GL driver retained blocking swap interval %d", interval);
    return true;
#endif
}

inline bool IsWindowPresentable(SDL_Window* window) {
    return window && !(SDL_GetWindowFlags(window) &
        (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_OCCLUDED));
}

class WindowFramePacer {
    Uint64 nextFrame_ = 0;
public:
    bool IsDue(Uint64 now = SDL_GetTicksNS()) {
        if (now < nextFrame_) return false;
        // Do not catch up missed frames or sleep here: queued main-thread
        // work and input must continue even when a window cannot present.
        nextFrame_ = now + 16666667;
        return true;
    }
};

} // namespace study::platform::sdl
