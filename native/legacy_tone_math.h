#pragma once
#include <cstdint>

namespace LegacyToneMathDetail {

// Match x86 SAR, including negative values, without relying on a compiler's
// signed-right-shift behavior. The callers below only use shifts 1 through 9.
inline std::int32_t FloorShift(std::int32_t value, unsigned shift) {
    const std::int32_t divisor = std::int32_t{1} << shift;
    return value / divisor - (value < 0 && value % divisor != 0 ? 1 : 0);
}

inline std::uint8_t ClampByte(std::int32_t value) {
    return static_cast<std::uint8_t>(value < 0 ? 0 : value > 255 ? 255 : value);
}

} // namespace LegacyToneMathDetail

// GLS3 EGL's enabled SSE/MMX path: eglcvt.asm:27-30,79-107.
// Each PMULHW truncates separately before the sum is shifted again; combining
// the products first changes the result. Intermediate sums fit signed 16 bits,
// so PADDSW cannot saturate for input channels in [0,255].
// Input B,G,R is replaced in place by Y,U,V. Alpha is the caller's concern.
inline void LegacyToneRGBToYUV(std::uint8_t& b, std::uint8_t& g,
                               std::uint8_t& r) {
    using LegacyToneMathDetail::FloorShift;
    using LegacyToneMathDetail::ClampByte;
    const std::int32_t blue = b, green = g, red = r;
    const std::int32_t y = FloorShift(
        FloorShift(blue * 2048, 9) + FloorShift(green * 9557, 9)
            + FloorShift(red * 4779, 9), 5);
    const std::int32_t u = FloorShift(
        FloorShift(blue * 8192, 9) + FloorShift(green * -5461, 9)
            + FloorShift(red * -2731, 9), 5) + 128;
    const std::int32_t v = FloorShift(
        FloorShift(blue * -1365, 9) + FloorShift(green * -6372, 9)
            + FloorShift(red * 7737, 9), 5) + 128;
    b = ClampByte(y);
    g = ClampByte(u);
    r = ClampByte(v);
}

// EGL's inverse conversion always uses the scalar branch: its SSE condition
// is literally .IF 0 (eglcvt.asm:174). These offsets reproduce the SAR carry
// consumed by ADC/SBB at lines 208-244, including negative chroma values.
// Input Y,U,V is replaced in place by B,G,R.
inline void LegacyToneYUVToRGB(std::uint8_t& y, std::uint8_t& u,
                               std::uint8_t& v) {
    using LegacyToneMathDetail::FloorShift;
    using LegacyToneMathDetail::ClampByte;
    const std::int32_t luminance = y;
    const std::int32_t chromaU = static_cast<std::int32_t>(u) - 128;
    const std::int32_t chromaV = static_cast<std::int32_t>(v) - 128;
    const std::int32_t red = luminance + FloorShift(3 * chromaV + 1, 1);
    const std::int32_t green = luminance - FloorShift(3 * chromaV + 2, 2)
        - FloorShift(3 * chromaU + 4, 3);
    const std::int32_t blue = luminance + FloorShift(7 * chromaU + 2, 2);
    y = ClampByte(blue);
    u = ClampByte(green);
    v = ClampByte(red);
}

// EGL_TONE_INVERSION=1, eglcvt.asm:1471-1489,1528-1533. This is not modern
// toneMultiple=1: -256 is identity, 0 is constant 127, and 256 is 255-input.
inline void LegacyToneInversion(std::uint8_t* table, int tone) {
    using LegacyToneMathDetail::FloorShift;
    const std::int32_t value = tone < -256 ? -256 : tone > 256 ? 256 : tone;
    const std::int32_t base = FloorShift(value, 1) + 128 - (value >= 0 ? 1 : 0);
    for (int i = 0; i < 256; ++i)
        table[i] = static_cast<std::uint8_t>(FloorShift(base * 256 - i * value, 8));
}
