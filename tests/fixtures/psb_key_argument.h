#pragma once
#include <charconv>
#include <cstdint>
#include <cstring>
#include <stdexcept>

// Probe input, never a compiled-in game parameter. Accept decimal or 0x hex,
// including zero, and reject signs, whitespace, trailing bytes and overflow.
inline std::uint32_t ParsePsbKeyArgument(const char *text) {
    if(!text || !*text)throw std::invalid_argument("PSB key must be a uint32 in decimal or 0x hex");
    const char *end=text+std::strlen(text);int base=10;
    if(end-text>2 && text[0]=='0' && (text[1]=='x'||text[1]=='X')) {text+=2;base=16;}
    std::uint32_t value=0;
    const auto result=std::from_chars(text,end,value,base);
    if(result.ec!=std::errc{} || result.ptr!=end)
        throw std::invalid_argument("PSB key must be a uint32 in decimal or 0x hex");
    return value;
}
