#pragma once
// Standalone host adapter for the engine's unmodified entropy decoder.
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
using BYTE = uint8_t;
using SBYTE = int8_t;
using WORD = uint16_t;
using SWORD = int16_t;
using DWORD = uint32_t;
using UINT = uint32_t;
using PBYTE = BYTE *;
struct ESLObject { virtual ~ESLObject() = default; };
#define ESL_DECLARE_CLASS_INFO(...)
#define ESL_IMPLEMENT_CLASS_INFO(...)
#define ESLAssert(x) assert(x)
#define esl_malloc std::malloc
#define esl_free std::free
#define eslFillMemory std::memset
namespace SakuraGL {}
namespace SSystem {
using SError = int;
constexpr SError errSuccess = 0;
constexpr SError errFailed = 1;
struct SInputStream : ESLObject {
    virtual size_t Read(void *buffer, size_t bytes) = 0;
};
}
