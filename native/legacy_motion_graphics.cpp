#include "legacy_compat/gls.h"
#include "legacy_motion_graphics.h"
#include "legacy_window.h"
#include <sakuragl/sgl_opengl_context.h>
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_video.h>
#else
#include <EGL/egl.h>
#endif
#include "platform/log.h"

namespace {
class CaptureGL final : public SSystem::SProcedure {
public:
    SakuraGL::SGLWindowSprite* window = nullptr;
    LegacyMotionGLParent parent;
    void Run() override {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        const auto display = SDL_GL_GetCurrentWindow();
        const auto context = SDL_GL_GetCurrentContext();
        if (!display || !context) return;
        int version = 0;
        if (!SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &version)) return;
#else
        const auto display = eglGetCurrentDisplay();
        const auto context = eglGetCurrentContext();
        if (display == EGL_NO_DISPLAY || context == EGL_NO_CONTEXT) return;
        EGLint version = 0;
        if (!eglQueryContext(display, context, EGL_CONTEXT_CLIENT_VERSION, &version)) return;
#endif
        parent = {reinterpret_cast<uintptr_t>(display), reinterpret_cast<uintptr_t>(context),
                  version, SakuraGL::SGLOpenGLContext::GetCurrentGLContext()};
        if (!parent.sdkContext && window)
            parent.sdkContext = ESLTypeCast<SakuraGL::SGLOpenGLContext>(window->GetRenderDevice());
    }
};
}

bool CaptureLegacyMotionGLParent(ECSWindow& window, LegacyMotionGLParent& parent) {
    CaptureGL capture;
    auto* native = window.NativeWindow();
    if (!native) return false;
    capture.window = native;
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    const bool hasCurrent = SDL_GL_GetCurrentContext() != nullptr;
#else
    const bool hasCurrent = eglGetCurrentContext() != EGL_NO_CONTEXT;
#endif
    if (hasCurrent && SakuraGL::SGLOpenGLContext::GetCurrentGLContext()) {
        capture.Run();
    } else {
        // postNormal is synchronous in EntisGLS's Android bridge. Release the
        // locks its UI/render thread needs while waiting, then restore ownership.
        const auto windowLocks = native->UnlockAll();
        const auto systemLocks = SSystem::UnlockAll();
        const auto error = native->PostRenderingThread(&capture, SakuraGL::Window::postNormal);
        SSystem::Relock(systemLocks);
        native->Relock(windowLocks);
        if (error) return false;
    }
    parent = capture.parent;
    const bool valid = parent.display && parent.context && parent.clientVersion >= 2;
    study::platform::LogPrint(valid ? study::platform::LogPriority::Info : study::platform::LogPriority::Error, "StudySteady",
        "Motion parent GL: display/window=%p context=%p sdk=%p major=%d valid=%d",
        reinterpret_cast<void*>(parent.display), reinterpret_cast<void*>(parent.context),
        parent.sdkContext, parent.clientVersion, valid);
    return valid;
}
