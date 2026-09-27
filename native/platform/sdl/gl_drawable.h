#pragma once

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_error.h>
#if defined(SDL_PLATFORM_IOS)
#include <OpenGLES/ES3/gl.h>
#endif

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

inline bool PresentWindow(SDL_Window* window) {
#if defined(SDL_PLATFORM_IOS)
    // UIKit presents the currently bound color renderbuffer. SDK render-target
    // allocation and E-mote drawing may leave zero or an offscreen buffer bound;
    // SDL's UIKit SwapWindow intentionally does not restore its drawable for us.
    const auto drawable = static_cast<GLuint>(SDL_GetNumberProperty(SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0));
    if (!drawable) return SDL_SetError("The UIKit window has no drawable renderbuffer");
    GLint previous = 0;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous);
    glBindRenderbuffer(GL_RENDERBUFFER, drawable);
    const bool presented = SDL_GL_SwapWindow(window);
    glBindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(previous));
    return presented;
#else
    return SDL_GL_SwapWindow(window);
#endif
}
}
