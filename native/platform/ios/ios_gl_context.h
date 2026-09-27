#pragma once

namespace study::platform::sdl {
// SDL's UIKit context handle is an EAGLContext object. Keep the Objective-C
// boundary here: offscreen shared contexts must not create another UIKit view.
void* CreateIOSSharedGLContext(void* parent);
void DestroyIOSSharedGLContext(void* context);
bool MakeIOSGLContextCurrent(void* context);
bool IsIOSGLContextCurrent(void* context);
void* RetainIOSCurrentGLContext();
void ReleaseIOSGLContextReference(void* context);
}
