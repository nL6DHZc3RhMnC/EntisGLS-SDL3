#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// GLS3's EBezierCurves<Vector2D>: x/y are left/right channel volumes,
// not time/value coordinates. Every cubic segment gets equal elapsed time.
struct LegacyVolumePoint { float left = 1, right = 1; };
using LegacyVolumeCurve = std::vector<LegacyVolumePoint>;

inline bool LegacyValidVolumeCurve(const LegacyVolumeCurve &curve) {
    if (curve.size() < 4 || (curve.size() - 1) % 3) return false;
    for (const auto &point : curve)
        if (!std::isfinite(point.left) || !std::isfinite(point.right) ||
            point.left < 0 || point.right < 0) return false;
    return true;
}

// Callers validate the 3*n+1 control-point shape before evaluation.
inline LegacyVolumePoint LegacyEvaluateVolumeCurve(const LegacyVolumeCurve &curve, double time) {
    const size_t segments = (curve.size() - 1) / 3;
    if (time <= 0) return curve.front();
    if (time >= 1) return curve.back();
    const double divided = time * segments;
    const size_t segment = std::min(static_cast<size_t>(divided), segments - 1);
    const double t = divided - segment, u = 1 - t;
    const double weights[4] = {u*u*u, 3*t*u*u, 3*t*t*u, t*t*t};
    const auto *points = curve.data() + segment * 3;
    double left = 0, right = 0;
    for (size_t i = 0; i < 4; ++i) {
        left += points[i].left * weights[i];
        right += points[i].right * weights[i];
    }
    return {static_cast<float>(left), static_cast<float>(right)};
}
