#include "platform/ios/window_orientation.h"
#include <SDL3/SDL.h>
#import <UIKit/UIKit.h>

namespace study::platform::sdl {
namespace {
void Refresh(UIWindow* window, bool library) {
    UIViewController* controller = window.rootViewController;
    if (!controller) return;
    if (@available(iOS 16.0, *)) {
        [controller setNeedsUpdateOfSupportedInterfaceOrientations];
        if (window.windowScene) {
            UIInterfaceOrientationMask mask = controller.supportedInterfaceOrientations;
            if (library) {
                // Prefer the library's normal presentation orientation when a
                // game forced landscape while the phone's rotation was locked.
                const auto preferred = controller.preferredInterfaceOrientationForPresentation;
                const auto preferredMask = UIInterfaceOrientationMask(1u << preferred);
                if (mask & preferredMask) mask = preferredMask;
            }
            auto* geometry = [[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:mask];
            [window.windowScene requestGeometryUpdateWithPreferences:geometry errorHandler:^(NSError* error) {
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "iOS orientation request: %s", error.localizedDescription.UTF8String);
            }];
        }
    }
    // Also covers pre-scene UIKit windows and iOS 13–15. Setting the SDL hint
    // alone does not ask UIKit to reevaluate an already visible controller.
    [UIViewController attemptRotationToDeviceOrientation];
}
}
bool RefreshIOSWindowOrientation(SDL_Window* window) {
    if (!SDL_IsMainThread()) return SDL_SetError("Update iOS orientation on the main thread");
    auto* native = (__bridge UIWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);
    if (!native || !native.rootViewController) return SDL_SetError("The UIKit game window is unavailable");
    Refresh(native, false);
    return true;
}
void RefreshIOSLibraryOrientation(void* nativeWindow) {
    if (SDL_IsMainThread()) Refresh((__bridge UIWindow*)nativeWindow, true);
}
}
