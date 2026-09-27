#pragma once

#include <SDL3/SDL_video.h>
#include <cstdint>
#include <optional>
#include <string>

namespace study::platform::sdl {
enum class ContentOrientation { Any, Landscape, Portrait };
ContentOrientation OrientationForContent(std::uint32_t width, std::uint32_t height);
const char* OrientationHint(ContentOrientation orientation);

// SDL mobile uses one game window on the primary display. Own the process hint
// only while that game window is logically open. Physical GL
// window leases may outlive that point, so close explicitly calls Restore().
class GameOrientationHintScope {
    bool active_ = false;
    std::optional<std::string> previous_;
public:
    GameOrientationHintScope() = default;
    GameOrientationHintScope(const GameOrientationHintScope&) = delete;
    GameOrientationHintScope& operator=(const GameOrientationHintScope&) = delete;
    ~GameOrientationHintScope();
    bool Set(ContentOrientation orientation);
    void Restore();
};

// Use original game dimensions, not the phone's physical drawable size. Call
// once before CreateWindow with null, then after GL view creation with the host
// window, and whenever the game changes its logical display dimensions.
bool ApplyMobileGameOrientation(GameOrientationHintScope& scope, SDL_Window* window,
    std::uint32_t width, std::uint32_t height);
void RestoreMobileGameOrientation(GameOrientationHintScope& scope, SDL_Window* window);
}
