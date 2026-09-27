#pragma once
#include <cstdint>

class ECSWindow;
namespace SakuraGL { class SGLOpenGLContext; }

// Borrowed from the live platform window. display is an EGLDisplay for the
// Android backend or SDL_Window* for SDL; context follows the same backend.
// Its owning script Window must outlive
// the shared E-mote renderer and every imported texture.
struct LegacyMotionGLParent {
    uintptr_t display = 0;
    uintptr_t context = 0;
    int clientVersion = 0;
    SakuraGL::SGLOpenGLContext* sdkContext = nullptr;
};
bool CaptureLegacyMotionGLParent(ECSWindow&, LegacyMotionGLParent&);
