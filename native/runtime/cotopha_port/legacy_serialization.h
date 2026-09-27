// Fixed Windows x86 CSX wire format. Included into generated GLS3 sources by
// patch_legacy_serialization.py; native object layouts remain host layouts.
#ifndef STUDYSTEADY_LEGACY_SERIALIZATION_H
#define STUDYSTEADY_LEGACY_SERIALIZATION_H
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include <memory>

namespace StudySteadyLegacyWire {
inline uint32_t Read32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
inline void Write32(uint8_t* p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = uint8_t(value >> (8 * i));
}
inline uint64_t Read64(const uint8_t* p) {
    return uint64_t(Read32(p)) | (uint64_t(Read32(p + 4)) << 32);
}
inline void Write64(uint8_t* p, uint64_t value) {
    Write32(p, uint32_t(value)); Write32(p + 4, uint32_t(value >> 32));
}

template<class String>
bool DecodeUtf16(const uint8_t* bytes, size_t units, String& out) {
    // At most one native wchar per UTF-16 unit, even on a UTF-32 host.
    if (units > size_t(std::numeric_limits<int>::max())) return false;
    wchar_t* dst = out.GetBuffer(int(units));
    if (units && !dst) return false;
    size_t written = 0;
    for (size_t i = 0; i < units; ++i) {
        uint32_t cp = uint32_t(bytes[i * 2]) | (uint32_t(bytes[i * 2 + 1]) << 8);
        if (sizeof(wchar_t) > 2 && cp >= 0xd800 && cp <= 0xdbff && i + 1 < units) {
            uint32_t low = uint32_t(bytes[(i + 1) * 2]) | (uint32_t(bytes[(i + 1) * 2 + 1]) << 8);
            if (low >= 0xdc00 && low <= 0xdfff) {
                cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                ++i;
            }
        }
        dst[written++] = wchar_t(cp);
    }
    out.ReleaseBuffer(int(written));
    return true;
}

template<class File, class String>
bool ReadWideString(File& file, String& out) {
    uint8_t count[4];
    if (file.Read(count, 4) != 4) return false;
    uint32_t units = Read32(count);
    auto length = file.GetLength(), position = file.GetPosition();
    if (position > length || units > (length - position) / 2) return false;
    std::vector<uint8_t> bytes(size_t(units) * 2);
    if (!bytes.empty() && file.Read(bytes.data(), bytes.size()) != bytes.size()) return false;
    return DecodeUtf16(bytes.data(), units, out);
}

template<class String>
bool EncodeUtf16(const String& value, std::vector<uint8_t>& bytes) {
    bytes.clear();
    auto append = [&bytes](uint32_t unit) {
        bytes.push_back(uint8_t(unit)); bytes.push_back(uint8_t(unit >> 8));
    };
    for (size_t i = 0; i < size_t(value.GetLength()); ++i) {
        uint32_t cp = uint32_t(value.CharPtr()[i]);
        if (cp > 0x10ffff) return false;
        if (cp >= 0x10000) {
            cp -= 0x10000;
            append(0xd800 | (cp >> 10)); append(0xdc00 | (cp & 0x3ff));
        } else append(cp);
    }
    return bytes.size() / 2 <= std::numeric_limits<uint32_t>::max();
}

template<class File, class String>
bool WriteWideString(File& file, const String& value) {
    std::vector<uint8_t> bytes;
    if (!EncodeUtf16(value, bytes)) return false;
    uint8_t count[4]; Write32(count, uint32_t(bytes.size() / 2));
    return file.Write(count, 4) == 4 &&
           (bytes.empty() || file.Write(bytes.data(), bytes.size()) == bytes.size());
}

template<class Cast>
void DecodeCast(const uint8_t* bytes, Cast& cast) {
    cast.iNativeParent = Read32(bytes);
    cast.iVarOffset = int32_t(Read32(bytes + 4));
    cast.nVarBounds = int32_t(Read32(bytes + 8));
    cast.iFuncOffset = int32_t(Read32(bytes + 12));
}
template<class Cast>
bool EncodeCast(uint8_t* bytes, const Cast& cast) {
    if (uint64_t(cast.iNativeParent) > std::numeric_limits<uint32_t>::max()) return false;
    Write32(bytes, uint32_t(cast.iNativeParent));
    Write32(bytes + 4, uint32_t(cast.iVarOffset));
    Write32(bytes + 8, uint32_t(cast.nVarBounds));
    Write32(bytes + 12, uint32_t(cast.iFuncOffset));
    return true;
}
template<class File, class Cast>
bool ReadCast(File& file, Cast& cast) {
    uint8_t bytes[16];
    if (file.Read(bytes, sizeof(bytes)) != sizeof(bytes)) return false;
    DecodeCast(bytes, cast); return true;
}
template<class File, class Cast>
bool WriteCast(File& file, const Cast& cast) {
    uint8_t bytes[16];
    return EncodeCast(bytes, cast) && file.Write(bytes, sizeof(bytes)) == sizeof(bytes);
}

template<class File, class Function>
bool ReadFunction(File& file, Function& fn) {
    uint8_t bytes[40];
    if (file.Read(bytes, sizeof(bytes)) != sizeof(bytes)) return false;
    // Persisted index/script addresses are portable. Native process pointers
    // are not; never reinterpret a Windows pointer as an Android callback.
    uint32_t type = Read32(bytes);
    if (type > 1) return false;
    fn.m_ftType = static_cast<decltype(fn.m_ftType)>(type);
    DecodeCast(bytes + 4, fn.m_castThis);
    fn.m_dwAlign = Read32(bytes + 20);
    std::memset(&fn.m_varFunc, 0, sizeof(fn.m_varFunc));
    fn.m_varFunc.addrScript = Read32(bytes + 24);
    for (unsigned i = 0; i < 3; ++i) fn.m_dwPadding[i] = Read32(bytes + 28 + i * 4);
    return true;
}
template<class File, class Function>
bool WriteFunction(File& file, const Function& fn) {
    if (uint32_t(fn.m_ftType) > 1 ||
        uint64_t(fn.m_varFunc.addrScript) > std::numeric_limits<uint32_t>::max()) return false;
    uint8_t bytes[40];
    Write32(bytes, uint32_t(fn.m_ftType));
    if (!EncodeCast(bytes + 4, fn.m_castThis)) return false;
    Write32(bytes + 20, fn.m_dwAlign);
    Write32(bytes + 24, uint32_t(fn.m_varFunc.addrScript));
    for (unsigned i = 0; i < 3; ++i) Write32(bytes + 28 + i * 4, fn.m_dwPadding[i]);
    return file.Write(bytes, sizeof(bytes)) == sizeof(bytes);
}
} // namespace StudySteadyLegacyWire
#endif
