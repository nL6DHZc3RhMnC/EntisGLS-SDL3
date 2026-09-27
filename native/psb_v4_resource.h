#pragma once
#include "psb_header.h"
#include <cstddef>

namespace studysteady {
// PSB v4 tags 0x22..0x25 select the extra-resource tables at header offsets
// 44/48/52. The game driver's parser confirms the 1..4 byte index encoding.
// Unlike the imported trusted-buffer reader, validate this new boundary.
inline uint32_t packedAt(const uint8_t *data, size_t size, size_t offset, uint32_t index) {
    if (offset >= size) throw std::runtime_error("PSB array offset out of bounds");
    const unsigned countBytes = data[offset] - 0x0cu;
    if (countBytes < 1 || countBytes > 4 || offset + 2 + countBytes > size)
        throw std::runtime_error("Invalid PSB packed count");
    uint32_t count = 0;
    for (unsigned i = 0; i < countBytes; ++i) count |= uint32_t(data[offset + 1 + i]) << (8 * i);
    const unsigned width = data[offset + 1 + countBytes] - 0x0cu;
    const size_t begin = offset + 2 + countBytes;
    if (width < 1 || width > 4 || index >= count || count > (size - begin) / width)
        throw std::runtime_error("PSB resource table bounds violation");
    uint32_t value = 0;
    for (unsigned i = 0; i < width; ++i) value |= uint32_t(data[begin + size_t(index) * width + i]) << (8 * i);
    return value;
}

inline const uint8_t *extraResource(const uint8_t *data, size_t size,
                                     const uint8_t *node, uint32_t &length) {
    const unsigned width = node[0] - 0x21u;
    if (width < 1 || width > 4 || node < data || size_t(node - data) + width + 1 > size)
        throw std::runtime_error("Invalid PSB v4 resource reference");
    uint32_t index = 0;
    for (unsigned i = 0; i < width; ++i) index |= uint32_t(node[1 + i]) << (8 * i);
    const uint32_t offset = packedAt(data, size, u32(data + 44), index);
    length = packedAt(data, size, u32(data + 48), index);
    const uint64_t position = uint64_t(u32(data + 52)) + offset;
    if (position > size || length > size - position)
        throw std::runtime_error("PSB v4 resource data out of bounds");
    return data + position;
}
}
