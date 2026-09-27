#pragma once

namespace SakuraGL { class SGLGenericWindow; }

// Called by the generated VirtualWindow JNI entry. The freeze check must run
// before the official nonvirtual OnDraw acquires the SDK UI mutex.
void StudySteadyDrawAndroidWindow(SakuraGL::SGLGenericWindow* window);

// True only when the SDK draw actually ran; SDL must not swap buffers while
// painting is frozen or it could reveal an older back buffer.
bool StudySteadyDrawWindow(SakuraGL::SGLGenericWindow* window);
