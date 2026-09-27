#pragma once

#include <SDL3/SDL_video.h>

namespace study::platform::sdl {
bool RefreshIOSWindowOrientation(SDL_Window* window);
// A borrowed UIWindow from the native library, called after makeKeyAndVisible.
void RefreshIOSLibraryOrientation(void* nativeWindow);
}
