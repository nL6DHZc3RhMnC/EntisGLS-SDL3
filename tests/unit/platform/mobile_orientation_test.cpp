#include "platform/sdl/mobile_orientation.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace {
void Check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
bool HintIs(const char* expected) {
    const auto* actual = SDL_GetHint(SDL_HINT_ORIENTATIONS);
    return actual && !std::strcmp(actual, expected);
}
}
int main() {
    using namespace study::platform::sdl;
    try {
        Check(OrientationForContent(1920, 1080) == ContentOrientation::Landscape, "wide content must select landscape");
        Check(OrientationForContent(720, 1280) == ContentOrientation::Portrait, "tall content must select portrait");
        Check(OrientationForContent(100, 100) == ContentOrientation::Any, "square content must allow both axes");
        Check(OrientationForContent(0, 100) == ContentOrientation::Any, "unset dimensions must not force a direction");
        Check(OrientationForContent(std::numeric_limits<unsigned>::max(), 1) == ContentOrientation::Landscape,
            "orientation comparison must not overflow");
        SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "Portrait", SDL_HINT_OVERRIDE);
        {
            GameOrientationHintScope game;
            Check(game.Set(ContentOrientation::Landscape), "cannot apply game orientation");
            Check(HintIs("LandscapeLeft LandscapeRight"), "both landscape sides must remain available");
            Check(game.Set(ContentOrientation::Portrait), "cannot change game orientation");
            Check(HintIs("Portrait PortraitUpsideDown"), "portrait game must replace the old axis");
            game.Restore();
            Check(HintIs("Portrait"), "logical close must restore the launcher before the GL lease dies");
            SDL_SetHintWithPriority(SDL_HINT_ORIENTATIONS, "LandscapeRight", SDL_HINT_OVERRIDE);
        }
        Check(HintIs("LandscapeRight"), "later destruction must not overwrite the restored launcher hint");
        SDL_ResetHint(SDL_HINT_ORIENTATIONS);
        const auto* prior = SDL_GetHint(SDL_HINT_ORIENTATIONS);
        const std::string priorValue = prior ? prior : "";
        const bool hadPrior = prior != nullptr;
        {
            GameOrientationHintScope game;
            Check(game.Set(ContentOrientation::Landscape), "cannot set a new game hint");
        }
        Check(hadPrior ? HintIs(priorValue.c_str()) : SDL_GetHint(SDL_HINT_ORIENTATIONS) == nullptr,
            "failed game setup must restore an absent/environment launcher hint");
        std::puts("Mobile orientation policy PASS: dimensions, both sides, logical close, setup cleanup");
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Mobile orientation policy FAIL: %s\n", error.what());
        SDL_Quit();
        return 1;
    }
}
