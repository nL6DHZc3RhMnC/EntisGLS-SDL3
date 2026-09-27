#include "extensions/emote/tjs_runtime/atlas_projection.h"
#include <iostream>
#include <limits>
#include <stdexcept>

using studysteady::motion::winAtlasSampleRect;
namespace {
void Require(bool result, const char* message) {
    if (!result) throw std::runtime_error(message);
}
template<class F> void Reject(F action) {
    try { action(); } catch (const std::runtime_error&) { return; }
    throw std::runtime_error("Invalid atlas projection was accepted");
}
}
int main() {
    try {
        // The logical image extends beyond an eight-pixel atlas. Its actual
        // packed pixels fit, and an endpoint between pixels must survive.
        const auto scaled = winAtlasSampleRect(4, 2, 7, 5, 0.5, 8, 8);
        Require(scaled == std::array<double, 4>{4, 2, 7.5, 4.5}, "Fractional scaled UVs were rounded or clamped");
        const auto ordinary = winAtlasSampleRect(1, 2, 7, 6, 1.0, 8, 8);
        Require(ordinary == std::array<double, 4>{1, 2, 8, 8}, "Resolution 1 changed the original rectangle");
        const auto enlarged = winAtlasSampleRect(1, 1, 3, 3, 2.0, 8, 8);
        Require(enlarged == std::array<double, 4>{1, 1, 7, 7}, "Resolution above one was lost");
        Reject([] { winAtlasSampleRect(4, 2, 7, 5, 1.0, 8, 8); });
        Reject([] { winAtlasSampleRect(-1, 0, 1, 1, 1.0, 8, 8); });
        for (double bad : {0.0, -1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
            Reject([=] { winAtlasSampleRect(0, 0, 2, 2, bad, 8, 8); });
        Reject([] { winAtlasSampleRect(0, 0, 2, std::numeric_limits<double>::quiet_NaN(), 1, 8, 8); });
        std::cout << "Win atlas projection PASS: fractional resolution, atlas edges, scale 1/2, invalid metadata\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
