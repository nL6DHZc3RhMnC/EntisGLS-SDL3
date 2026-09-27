#include "ios_gl_context.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#import <OpenGLES/EAGL.h>

namespace study::platform::sdl {
void* CreateIOSSharedGLContext(void* parent) {
    if (!SDL_IsMainThread() || !parent) {
        SDL_SetError("Create the shared EAGL context from a live parent on the main thread");
        return nullptr;
    }
    EAGLContext* host = (__bridge EAGLContext*)parent;
    // SDL_GL_CreateContext(window) also creates and installs a new drawable
    // UIView on UIKit. E-mote only needs shared textures and its own GL state.
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:host.API sharegroup:host.sharegroup];
    if (!context) {
        SDL_SetError("Cannot create the offscreen shared EAGL context");
        return nullptr;
    }
    return (__bridge_retained void*)context;
}

bool MakeIOSGLContextCurrent(void* context) {
    if (!SDL_IsMainThread()) return SDL_SetError("Activate EAGL contexts on the main thread");
    if (![EAGLContext setCurrentContext:(__bridge EAGLContext*)context])
        return SDL_SetError("Cannot activate the requested EAGL context");
    return true;
}

bool IsIOSGLContextCurrent(void* context) {
    return EAGLContext.currentContext == (__bridge EAGLContext*)context;
}

void* RetainIOSCurrentGLContext() {
    return (__bridge_retained void*)EAGLContext.currentContext;
}

void ReleaseIOSGLContextReference(void* context) {
    if (context) {
        EAGLContext* held = (__bridge_transfer EAGLContext*)context;
        (void)held;
    }
}

void DestroyIOSSharedGLContext(void* context) {
    if (context && IsIOSGLContextCurrent(context)) MakeIOSGLContextCurrent(nullptr);
    ReleaseIOSGLContextReference(context);
}
}
