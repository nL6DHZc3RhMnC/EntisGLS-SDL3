// Standalone regression check for native/legacy_tone_math.h.
// Run from the repository root (Clang and C++17 required):
//   clang++ -std=c++17 -O2 -Wall -Wextra -Werror -fsanitize=undefined,address -I. tools/verify_legacy_tone_math.cpp -o /tmp/verify_legacy_tone_math
//   /tmp/verify_legacy_tone_math
//
// This is the harness used to validate the original helper implementation.
// It compares both conversions over all 16,777,216 byte triplets and compares
// 525,568 inversion entries, including INT_MIN and INT_MAX tone arguments.
// The reference follows EGL instructions rather than the helper's simplified
// formulas: build/legacy/EGL/Source/eglcvt.asm:27-30,79-107,208-244,1471-1533.

#include "native/legacy_tone_math.h"
#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <cstdio>

using Pixel = std::array<uint8_t, 3>;

// x86 arithmetic right shift: also expose the last shifted-out bit as CF.
int32_t sar(int32_t x, unsigned n, unsigned& carry) {
    uint32_t bits = static_cast<uint32_t>(x);
    carry = (bits >> (n - 1)) & 1;
    bits >>= n;
    if (x < 0) bits |= ~uint32_t{0} << (32 - n);
    return static_cast<int32_t>(bits);
}

int16_t paddsw(int16_t a, int16_t b) {
    return static_cast<int16_t>(std::clamp(int(a) + int(b), -32768, 32767));
}

uint8_t packuswb(int32_t x) {
    return static_cast<uint8_t>(std::clamp(x, 0, 255));
}

// Emulate PSLLW, PMULHW, PADDSW, PSRAW, PADDSW and PACKUSWB in source order.
Pixel rgbSSE(Pixel rgb) {
    static const int16_t matrix[3][3] = {
        {2048, 8192, -1365},
        {9557, -5461, -6372},
        {4779, -2731, 7737}
    };
    Pixel result{};
    unsigned carry;
    for (int lane = 0; lane < 3; ++lane) {
        int16_t reg[3];
        for (int channel = 0; channel < 3; ++channel) {
            int16_t input = static_cast<int16_t>(uint16_t(rgb[channel]) << 7);
            reg[channel] = static_cast<int16_t>(
                sar(int32_t(input) * matrix[channel][lane], 16, carry));
        }
        int16_t sum = paddsw(paddsw(reg[0], reg[1]), reg[2]);
        int16_t shifted = static_cast<int16_t>(sar(sum, 5, carry));
        result[lane] = packuswb(paddsw(shifted, lane ? 128 : 0));
    }
    return result;
}

// EGL disables the inverse SSE branch with .IF 0. Follow its scalar registers
// and SAR carry consumption directly; no simplified rounding formula is used.
Pixel yuvX86(Pixel p) {
    int32_t ecx = p[1], edx = p[2], eax = p[0];
    int32_t esi = edx + edx * 2 - 0x180;
    int32_t ebx = ecx + ecx * 2 - 0x180;
    int32_t saved = eax;
    unsigned carry;

    esi = sar(esi, 1, carry);
    eax = eax + esi + carry;
    uint8_t red = packuswb(eax);

    eax = saved;
    esi = sar(esi, 1, carry);
    eax = eax - esi - carry;
    esi = ebx + ecx * 4 - 0x200;
    ebx = sar(ebx, 3, carry);
    eax = eax - ebx - carry;
    uint8_t green = packuswb(eax);

    eax = saved;
    esi = sar(esi, 2, carry);
    eax = eax + esi + carry;
    return {packuswb(eax), green, red};
}

bool compare(Pixel input) {
    auto rgb = input, yuv = input;
    LegacyToneRGBToYUV(rgb[0], rgb[1], rgb[2]);
    LegacyToneYUVToRGB(yuv[0], yuv[1], yuv[2]);
    if (rgb != rgbSSE(input) || yuv != yuvX86(input)) {
        std::printf("Mismatch: %u,%u,%u\n", input[0], input[1], input[2]);
        return false;
    }
    return true;
}

int main() {
    uint64_t fixtures = 0;
    for (int a = 0; a < 256; ++a) {
        for (int b = 0; b < 256; ++b) {
            for (int c = 0; c < 256; ++c) {
                if (!compare({uint8_t(a), uint8_t(b), uint8_t(c)})) return 1;
                ++fixtures;
            }
        }
    }

    uint64_t toneEntries = 0;
    for (int k = -1026; k <= 1026; ++k) {
        int tone = k == -1026 ? INT_MIN : k == 1026 ? INT_MAX : k;
        std::array<uint8_t, 256> table{};
        LegacyToneInversion(table.data(), tone);
        int edx = std::clamp(tone, -256, 256);
        unsigned carry;
        int eax = sar(edx, 1, carry) + 0x80;
        if (edx >= 0) --eax;
        edx = -edx;
        eax *= 256;
        for (int i = 0; i < 256; ++i) {
            uint8_t ah = static_cast<uint8_t>(static_cast<uint32_t>(eax) >> 8);
            if (table[i] != ah) {
                std::printf("Inversion mismatch: tone %d i %d\n", tone, i);
                return 1;
            }
            eax += edx;
            ++toneEntries;
        }
    }
    std::printf(
        "PASS: RGB SSE and YUV scalar each matched all %llu byte triplets; "
        "inversion matched %llu entries.\n",
        (unsigned long long)fixtures, (unsigned long long)toneEntries);
}
