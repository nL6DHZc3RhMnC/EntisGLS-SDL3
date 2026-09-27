#pragma once

#include <SDL3/SDL_video.h>

namespace study::platform::sdl {
bool RefreshIOSWindowOrientation(SDL_Window* window);
// Current UIKit scene orientation, not the physical device/screenshot dimensions.
// Returns a static name; unknown when no native window is available.
const char* GetIOSWindowInterfaceOrientation(SDL_Window* window);
// A borrowed UIWindow from the native library, called after makeKeyAndVisible.
void RefreshIOSLibraryOrientation(void* nativeWindow);
}
