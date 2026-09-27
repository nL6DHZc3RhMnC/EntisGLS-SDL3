#pragma once

#include <array>
#include <cmath>
#include <stdexcept>

namespace studysteady::motion {

// A PSB icon's width/height describe its logical geometry. Its pixels occupy
// width*resolution by height*resolution in the atlas. Keep fractional sample
// coordinates through mesh tessellation; rounding here changes the sampled
// image and can include a neighbouring packed icon.
struct AtlasSampleRect {
    double left, top, right, bottom;
};

inline std::array<double, 4> winAtlasSampleRect(
    int left, int top, double width, double height, double resolution,
    unsigned textureWidth, unsigned textureHeight) {
    if (!std::isfinite(width) || !std::isfinite(height) ||
        !std::isfinite(resolution) || width <= 0 || height <= 0 || resolution <= 0)
        throw std::runtime_error("Invalid Win PSB icon dimensions or resolution");
    const double right = left + width * resolution;
    const double bottom = top + height * resolution;
    if (left < 0 || top < 0 || !std::isfinite(right) || !std::isfinite(bottom) ||
        right <= left || bottom <= top || right > textureWidth || bottom > textureHeight)
        throw std::runtime_error("Win PSB icon sample rectangle exceeds its atlas after applying resolution");
    return {double(left), double(top), right, bottom};
}

} // namespace studysteady::motion
