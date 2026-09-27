#pragma once
#include "tjs.h"
#include "MeshPoint.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace studysteady::motion {
using MeshPatch = std::array<float, 32>;
using MeshVariables = std::unordered_map<std::string, float>;

// Recovered from StudySteady's emotedriver.dll, not the older Kirikiroid2
// Player. Float operation order and the all-neutral empty-patch state matter.
void meshInterpolate(MeshPatch &out, const MeshPatch &a, const MeshPatch &b, float ratio);
void meshAdd(MeshPatch &out, const MeshPatch &a, const MeshPatch &b);
void meshAddSubtract(MeshPatch &out, const MeshPatch &a, const MeshPatch &add, const MeshPatch &subtract);

class MeshCombinator {
public:
    struct Axis {
        std::string key;
        float begin = 0, end = 0;
        int neutralIndex = -1;
        std::vector<MeshPatch> frames;
        MeshPatch current{}, previous{};
        float position = 0;
        bool neutral = false;
    };
    // Argument is the real PSBValueDispatch value of layer.meshCombinator.
    explicit MeshCombinator(const tTJSVariant &value);
    // Missing variables have the DLL's actual initial value, zero.
    // First update initializes. Later updates use the DLL's dirty-count split.
    bool update(const MeshVariables &variables);
    bool allNeutral() const { return allNeutral_; }
    bool initialized() const { return initialized_; }
    const MeshPatch &combined() const { return combined_; }
    const std::vector<Axis> &axes() const { return axes_; }
    // The DLL clears its output vector at all-neutral, even though the sum is a
    // unit patch. Write into a real Player node at the layer evaluation boundary.
    void publish(std::vector<::motion::detail::MeshPoint> &points) const;
private:
    std::vector<Axis> axes_;
    MeshPatch combined_{};
    bool initialized_ = false, allNeutral_ = false;
};
}
