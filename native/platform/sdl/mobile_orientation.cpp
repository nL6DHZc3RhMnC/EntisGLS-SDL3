#include "platform/sdl/mobile_orientation.h"
#include <SDL3/SDL.h>
#if defined(SDL_PLATFORM_ANDROID)
#include <jni.h>
#elif defined(SDL_PLATFORM_IOS)
#include "platform/ios/window_orientation.h"
#endif

namespace study::platform::sdl {
ContentOrientation OrientationForContent(std::uint32_t width, std::uint32_t height) {
    if (!width || !height || width == height) return ContentOrientation::Any;
    return width > height ? ContentOrientation::Landscape : ContentOrientation::Portrait;
}
const char* OrientationHint(ContentOrientation orientation) {
    switch (orientation) {
    case ContentOrientation::Landscape: return "LandscapeLeft LandscapeRight";
    case ContentOrientation::Portrait: return "Portrait PortraitUpsideDown";
    default: return "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown";
    }
}
GameOrientationHintScope::~GameOrientationHintScope() { Restore(); }
bool GameOrientationHintScope::Set(ContentOrientation orientation) {
    if (!active_) {
        if (const auto* previous = SDL_GetHint(SDL_HINT_ORIENTATIONS)) previous_ = previous;
        else previous_.reset();
        active_ = true;
    }
    return SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, OrientationHint(orientation), SDL_HINT_OVERRIDE);
}
void GameOrientationHintScope::Restore() {
    if (!active_) return;
    if (previous_) SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, previous_->c_str(), SDL_HINT_OVERRIDE);
    else SDL_ResetHint(SDL_HINT_ORIENTATIONS);
    previous_.reset();
    active_ = false;
}

namespace {
bool RefreshOrientation(SDL_Window* window, std::uint32_t width, std::uint32_t height) {
    if (!window) return true;
#if defined(SDL_PLATFORM_ANDROID)
    // SDL has no orientation-update API or hint callback on Android. Dispatch
    // the same public SDLActivity method used by SDL's own CreateWindow path.
    auto* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    if (!env) return SDL_SetError("Cannot update the Android game orientation");
    if (env->PushLocalFrame(8) < 0) { env->ExceptionClear(); return SDL_SetError("Cannot allocate orientation JNI frame"); }
    struct PopFrame { JNIEnv* env; ~PopFrame() { env->PopLocalFrame(nullptr); } } frame{env};
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!activity) return SDL_SetError("The Android game activity is unavailable");
    jclass type = env->GetObjectClass(activity);
    jmethodID method = type ? env->GetMethodID(type, "setOrientationBis", "(IIZLjava/lang/String;)V") : nullptr;
    const auto* hint = SDL_GetHint(SDL_HINT_ORIENTATIONS);
    jstring value = method ? env->NewStringUTF(hint ? hint : "") : nullptr;
    if (value) env->CallVoidMethod(activity, method, static_cast<jint>(width), static_cast<jint>(height), JNI_TRUE, value);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return SDL_SetError("Android rejected the game orientation request"); }
    return value != nullptr || SDL_SetError("Cannot construct the Android orientation request");
#elif defined(SDL_PLATFORM_IOS)
    return RefreshIOSWindowOrientation(window);
#else
    (void)width; (void)height;
    return true;
#endif
}
}

bool ApplyMobileGameOrientation(GameOrientationHintScope& scope, SDL_Window* window,
        std::uint32_t width, std::uint32_t height) {
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
    if (!width || !height) return SDL_SetError("Game orientation requires nonzero content dimensions");
    if (!scope.Set(OrientationForContent(width, height))) return false;
    return RefreshOrientation(window, width, height);
#else
    (void)scope; (void)window; (void)width; (void)height;
    return true;
#endif
}
void RestoreMobileGameOrientation(GameOrientationHintScope& scope, SDL_Window* window) {
    scope.Restore();
#if defined(SDL_PLATFORM_ANDROID)
    int width = 0, height = 0;
    if (window && SDL_GetWindowSize(window, &width, &height) && width > 0 && height > 0)
        if (!RefreshOrientation(window, width, height)) SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
#else
    // UIKit's library owns a separate native UIWindow. Refresh it when visible,
    // rather than asking a retiring hidden GL window to rotate the scene.
    (void)window;
#endif
}
}
