#pragma once
#include <cstdint>
#include <string>
#include <stdexcept>
// These scalar aliases match the parser's public return types. No TJS runtime
// or script dispatch is emulated by this adapter.
using tjs_int = int32_t;
using tjs_int64 = int64_t;
using tjs_real = double;
using ttstr = std::string;
#define TJS_W(text) text
inline void *TJSAlignedAlloc(size_t size, unsigned) { return new uint8_t[size]; }
inline void TJSAlignedDealloc(void *p) { delete[] static_cast<uint8_t *>(p); }
[[noreturn]] inline void TVPThrowExceptionMessage(const char *text) {
    throw std::runtime_error(text);
}
[[noreturn]] inline void TVPThrowExceptionMessage(const char *text, const std::string &detail) {
    throw std::runtime_error(std::string(text) + " " + detail);
}
