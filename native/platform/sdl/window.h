#pragma once
#include <SDL3/SDL.h>
namespace SakuraGL { class SGLGenericWindow; }

namespace study::platform::sdl {
// Called from the SDL main thread. SDK windows and render contexts live there;
// the Cotopha script runs separately and dispatches platform work to this loop.
void ProcessWindowEvent(const SDL_Event& event);
void DrawWindows();
// Main-thread cleanup for exceptional closes; safe after SDK finalization.
void CollectRetiredWindows();
bool HasWindows();
// Called on the main thread with the app's current GL context, after a
// successful draw and before presentation. Pass nullptr to disable.
void SetFrameObserver(void (*observer)(SDL_Window*, void*), void* user);
void SetWindowDrawHandler(bool (*handler)(SakuraGL::SGLGenericWindow*));
}
