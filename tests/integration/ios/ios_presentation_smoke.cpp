#include "ios_presentation_smoke.h"
#include "platform/sdl/gl_drawable.h"
#include "platform/sdl/mobile_orientation.h"
#include "platform/ios/ios_gl_context.h"
#include "platform/ios/window_orientation.h"
#include "extensions/emote/tjs_runtime/runtime_gl.h"
#include <SDL3/SDL.h>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>

namespace study::platform::sdl {
namespace {
void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(std::string(message) + ": " + SDL_GetError());
}
}

int RunIOSPresentationSmoke() {
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    GLuint offscreen = 0;
    GameOrientationHintScope orientation;
    GlWindowLease lease;
    std::unique_ptr<studysteady::motion::RuntimeGl> motion;
    auto cleanup = [&] {
        motion.reset();
        if (context) MakeIOSGLContextCurrent(context);
        if (offscreen) glDeleteRenderbuffers(1, &offscreen);
        RestoreMobileGameOrientation(orientation, window);
        if (lease) {
            UnregisterGlWindow(window);
            lease.reset(); // Owns physical SDL context and window destruction.
        } else {
            if (context) SDL_GL_DestroyContext(context);
            if (window) SDL_DestroyWindow(window);
        }
    };
    try {
        Require(SDL_IsMainThread(), "The presentation diagnostic needs the main thread");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        Require(ApplyMobileGameOrientation(orientation, nullptr, 360, 640), "Cannot select portrait setup");
        window = SDL_CreateWindow("EntisGLS presentation diagnostic", 360, 640,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
        Require(window != nullptr, "Cannot create the presentation window");
        context = SDL_GL_CreateContext(window);
        Require(context != nullptr, "Cannot create the GLES context");
        Require(ApplyMobileGameOrientation(orientation, window, 360, 640), "Cannot request portrait setup");
        const auto portraitStarted = SDL_GetTicks();
        for (;;) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) Require(event.type != SDL_EVENT_QUIT, "Presentation check closed during orientation setup");
            int width = 0, height = 0;
            Require(SDL_GetWindowSizeInPixels(window, &width, &height), "Cannot read initial drawable size");
            if (height > width && width > 2 &&
                SDL_strcmp(GetIOSWindowInterfaceOrientation(window), "portrait") == 0) break;
            Require(SDL_GetTicks() - portraitStarted < 10000, "The simulator did not enter the initial portrait orientation");
            SDL_Delay(10);
        }
        // Match a wide game launched from the portrait library. RESIZABLE is
        // intentional: relying only on SDL's window aspect would allow portrait.
        Require(ApplyMobileGameOrientation(orientation, window, 1920, 1080), "Cannot request landscape game orientation");
        Require(ResolveWindowFramebuffer(0) != 0, "Missing UIKit drawable framebuffer");
        lease = RegisterGlWindow(window, context);
        const auto properties = SDL_GetWindowProperties(window);
        const auto drawableFramebuffer = SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0);
        const auto drawableRenderbuffer = SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0);
        motion = std::make_unique<studysteady::motion::RuntimeGl>(
            reinterpret_cast<std::uintptr_t>(window), reinterpret_cast<std::uintptr_t>(context));
        Require(IsIOSGLContextCurrent(context), "Motion creation did not restore the actual host EAGL context");
        Require(SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0) == drawableFramebuffer &&
            SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0) == drawableRenderbuffer,
            "Motion creation replaced the window drawable");

        GLuint sharedTexture = 0;
        glGenTextures(1, &sharedTexture);
        glBindTexture(GL_TEXTURE_2D, sharedTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFinish();
        {
            studysteady::motion::CurrentGlContextScope host;
            motion->makeCurrent();
            Require(!IsIOSGLContextCurrent(context), "Motion did not activate a separate EAGL context");
            Require(glIsTexture(sharedTexture), "Motion context cannot see the host's shared texture");
            GLuint target = 0;
            glGenFramebuffers(1, &target);
            glBindFramebuffer(GL_FRAMEBUFFER, target);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sharedTexture, 0);
            Require(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Motion texture framebuffer is incomplete");
            glClearColor(0, 0, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glFinish();
            glDeleteFramebuffers(1, &target);
            host.restore();
        }
        Require(IsIOSGLContextCurrent(context), "Motion draw did not restore the actual host EAGL context");
        GLuint readback = 0;
        glGenFramebuffers(1, &readback);
        glBindFramebuffer(GL_FRAMEBUFFER, readback);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sharedTexture, 0);
        std::array<unsigned char, 4> sharedPixel{};
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, sharedPixel.data());
        Require(glGetError() == GL_NO_ERROR && sharedPixel[0] < 16 && sharedPixel[1] < 16 && sharedPixel[2] > 240,
            "The host could not read the texture drawn in the motion context");
        glDeleteFramebuffers(1, &readback);
        glDeleteTextures(1, &sharedTexture);
        // Keep the real RuntimeGl instance alive through the screenshot. An
        // accidental child UIView would otherwise disappear during teardown.
        glGenRenderbuffers(1, &offscreen);
        glBindRenderbuffer(GL_RENDERBUFFER, offscreen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 8, 8);
        Require(glGetError() == GL_NO_ERROR, "Cannot create the offscreen renderbuffer");

        unsigned frames = 0;
        unsigned landscapeFrames = 0;
        bool ready = false;
        bool running = true, background = false;
        const auto started = SDL_GetTicks();
        while (running && SDL_GetTicks() - started < 120000) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) running = false;
                if (event.type == SDL_EVENT_WILL_ENTER_BACKGROUND || event.type == SDL_EVENT_DID_ENTER_BACKGROUND)
                    background = true;
                if (event.type == SDL_EVENT_DID_ENTER_FOREGROUND) background = false;
            }
            if (!running) break;
            if (background) { SDL_Delay(10); continue; }
            int width = 0, height = 0;
            Require(SDL_GetWindowSizeInPixels(window, &width, &height) && width > 2 && height > 2,
                "Invalid presentation drawable size");
            const char* interfaceOrientation = GetIOSWindowInterfaceOrientation(window);
            const bool sceneIsLandscape = SDL_strcmp(interfaceOrientation, "landscape-left") == 0 ||
                SDL_strcmp(interfaceOrientation, "landscape-right") == 0;
            landscapeFrames = width > height && sceneIsLandscape ? landscapeFrames + 1 : 0;
            Require(ready || SDL_GetTicks() - started < 10000,
                "The resizable game window did not present in landscape after the orientation request");
            glBindFramebuffer(GL_FRAMEBUFFER, ResolveWindowFramebuffer(0));
            glViewport(0, 0, width, height);
            glDisable(GL_SCISSOR_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(0, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_SCISSOR_TEST);
            glScissor(width / 2, 0, width - width / 2, height);
            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            std::array<unsigned char, 4> green{}, red{};
            glReadPixels(width / 4, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, green.data());
            glReadPixels(3 * width / 4, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, red.data());
            Require(glGetError() == GL_NO_ERROR && green[1] > 240 && green[0] < 16 &&
                red[0] > 240 && red[1] < 16, "GLES framebuffer pixels do not match the diagnostic pattern");

            // Reproduce both production callers: the SDK unbinds its temporary
            // renderbuffer, and the motion renderer leaves a different one bound.
            const auto previous = frames % 2 ? offscreen : GLuint(0);
            glBindRenderbuffer(GL_RENDERBUFFER, previous);
            Require(PresentWindow(window), "Cannot present the UIKit drawable");
            GLint restored = -1;
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &restored);
            Require(restored == GLint(previous) && glGetError() == GL_NO_ERROR,
                "Presentation failed to preserve the caller's renderbuffer binding");
            ++frames;
            if (!ready && frames >= 4 && landscapeFrames >= 4) {
                ready = true;
                SDL_Log("IOS_PRESENTATION_READY framebuffer=%u zero_and_offscreen_bindings=verified shared_motion_context=verified orientation=landscape initial_portrait=verified pattern=green-left-red-right interface_orientation=%s drawable_width=%d drawable_height=%d",
                    ResolveWindowFramebuffer(0), interfaceOrientation, width, height);
            }
            SDL_Delay(16);
        }
        cleanup();
        return 0;
    } catch (...) {
        cleanup();
        throw;
    }
}
}
