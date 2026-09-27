#pragma once

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>

namespace study::platform::sdl {
inline unsigned int ResolveWindowFramebuffer(unsigned int framebuffer) {
#if defined(SDL_PLATFORM_IOS)
    // EAGL presents an SDL-owned framebuffer, not framebuffer zero. Query on
    // each use because resizing the drawable can replace its GL objects.
    if (!framebuffer) {
        if (auto* window = SDL_GL_GetCurrentWindow()) {
            framebuffer = static_cast<unsigned int>(SDL_GetNumberProperty(SDL_GetWindowProperties(window),
                SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0));
        }
    }
#endif
    return framebuffer;
}
inline unsigned int WindowBackBuffer() {
#if defined(SDL_PLATFORM_IOS)
    return 0x8ce0; // GL_COLOR_ATTACHMENT0 on SDL's EAGL framebuffer.
#else
    return 0x0405; // GL_BACK on the window-system default framebuffer.
#endif
}
}
