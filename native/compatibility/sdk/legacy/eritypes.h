#pragma once
#include "compatibility/sdk/legacy/windows.h"
union REAL_DWORD {
    REAL32 r32;
    DWORD dw32;
    REAL_DWORD(REAL32 value) : r32(value) {}
    REAL_DWORD(DWORD value) : dw32(value) {}
};
inline INT64 eriRoundR64ToLInt(double value) { return llround(value); }
