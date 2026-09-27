#pragma once
// Algorithm adapted from Entis GLS3 glssupsprite.cpp.
// Original: Copyright (c) 2004-2007 Leshade Entis, Entis-soft. All rights reserved.
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

// GLS3 ECSSuperSprite::draw_RasterScroll / OnAdvanceAnimation. The saved
// EFFECT_PARAM stays unchanged; transformation is an internal runtime flag.
class LegacySuperRasterMath {
    uint32_t flags_, interval_, step_, width_, mesh_, frequency_;
    uint32_t degree_, counter_ = 0;
public:
    explicit LegacySuperRasterMath(const std::array<uint32_t,33>& words, uint32_t degree)
        : flags_(words[1] | 4u), interval_(words[2]), step_(words[3]),
          width_(words[4]), mesh_(words[5]), frequency_(words[7]), degree_(degree) {}
    uint32_t Degree() const { return degree_; }
    void SetDegree(uint32_t degree) { degree_ = degree; }
    uint32_t IntervalCounter() const { return counter_; }
    uint32_t Width() const { return width_; }
    uint32_t Flags() const { return flags_; }
    uint32_t Transparency() const {
        // The original DWORD multiplication wraps before division.
        return (flags_ & 1u) ? uint32_t(degree_ * degree_) / 256u : 0u;
    }
    bool Advance(uint32_t milliseconds, bool enabled) {
        const uint32_t previous = degree_;
        counter_ += milliseconds; // defined Win32 32-bit counter wrap
        const int32_t interval = int32_t(interval_), elapsed = int32_t(counter_);
        if (interval > 0 && elapsed >= interval) {
            const int32_t remainder = elapsed % interval;
            const uint32_t count = enabled ? uint32_t((elapsed - remainder) / interval) : 0u;
            counter_ = uint32_t(remainder);
            int32_t next = int32_t(degree_ + count * step_);
            if (next >= 768) next = 512 + (next - 512) % 256;
            degree_ = enabled ? uint32_t(next) : 0u;
        }
        return degree_ != previous;
    }
    static bool RoundNearestEven(double value, int32_t& result) {
        if (!std::isfinite(value)) return false;
        const double lower = std::floor(value), fraction = value - lower;
        const double rounded = lower + ((fraction > .5 ||
            (fraction == .5 && std::fmod(std::fabs(lower), 2.) == 1.)) ? 1. : 0.);
        if (rounded < std::numeric_limits<int32_t>::min() ||
            rounded > std::numeric_limits<int32_t>::max()) return false;
        result = int32_t(rounded); return true;
    }
    bool Offset(uint32_t row, int32_t& result) const {
        if (!mesh_) return false;
        constexpr double pi = 3.14159265; // original constant, not M_PI
        const int32_t degree = int32_t(degree_);
        const double ratio = double(degree) / 256.;
        double amplitude = double(int32_t(width_));
        if (degree < 256) amplitude *= ratio;
        const double phase = (pi * int32_t(frequency_)) * ratio;
        const double wavelength = pi / double(int32_t(mesh_));
        // Keep multiply/add separate, as on the original non-FMA x86 path.
        volatile double position = double(row) * wavelength;
        return RoundNearestEven(amplitude * std::sin(position + phase), result);
    }
};
