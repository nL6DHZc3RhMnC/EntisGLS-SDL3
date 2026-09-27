#include "MotionBezierPatch.h"

// The imported runtime selects its existing arithmetic path from these actual
// compile-target features. AArch64 mandates Advanced SIMD.
extern "C" {
#if defined(__aarch64__)
tjs_uint32 TVPCPUType = TVP_CPU_FAMILY_ARM | TVP_CPU_HAS_NEON;
#elif defined(__x86_64__)
tjs_uint32 TVPCPUType = TVP_CPU_FAMILY_X64 | TVP_CPU_HAS_SSE2;
#else
tjs_uint32 TVPCPUType = 0;
#endif
}
namespace motion::internal {
// Original process globals from motionplayer/main.cpp.
std::array<float, 8> unitBezierPatchQuad_guess = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
std::vector<detail::MeshPoint> defaultBezierPatchPoints_guess;
}
