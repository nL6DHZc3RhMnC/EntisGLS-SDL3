#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <zlib.h>

namespace studysteady {
inline uint32_t u32(const uint8_t *p) { uint32_t x; std::memcpy(&x, p, 4); return x; }

// The game PSBs use the same xorshift byte stream as MotionPlayer's seed
// filter, but apply it to the header [8,56). Verified against all 68 base
// character files using their stored Adler-32 header checksums.
inline void decodePsbHeader(std::vector<uint8_t> &data, uint32_t seed) {
    if (data.size() < 56 || std::memcmp(data.data(), "PSB\0", 4))
        throw std::runtime_error("Not a PSB file");
    const uint16_t version = data[4] | (data[5] << 8);
    const uint16_t flags = data[6] | (data[7] << 8);
    if (version != 4 || (flags & ~1u))
        throw std::runtime_error("Unsupported PSB version/encryption flags");
    if (flags & 1) {
        uint32_t x = 123456789u, y = 362436069u, z = 521288629u, w = seed, bytes = 0;
        for (size_t i = 8; i < 56; ++i) {
            if (!bytes) {
                const uint32_t t = x ^ (x << 11);
                x = y; y = z; z = w;
                w = w ^ (w >> 19) ^ t ^ (t >> 8);
                bytes = w;
            }
            data[i] ^= static_cast<uint8_t>(bytes);
            bytes >>= 8;
        }
    }
    uLong checksum = adler32(1, data.data() + 8, 32);
    checksum = adler32(checksum, data.data() + 44, 12);
    if (checksum != u32(data.data() + 40))
        throw std::runtime_error("PSB header checksum mismatch");
    for (size_t i = 8; i < 56; i += 4) {
        if (i == 40) continue;
        if (u32(data.data() + i) > data.size())
            throw std::runtime_error("PSB offset out of range");
    }
    if (u32(data.data() + 8) != 56 || u32(data.data() + 36) >= data.size())
        throw std::runtime_error("Invalid PSB header/entry offset");
    data[6] = data[7] = 0;
}
}
