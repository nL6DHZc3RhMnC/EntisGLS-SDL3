# StudySteady mesh-combinator reference

Input: `StudySteadyR18/emotedriver.dll`, SHA-256
`40d2e510106fc8a79e0fd07e5c0d4839d75503089d6fc9de0f0904f0ab73bd4d`.
Analysis uses the read-only binary copy in `build/analysis`. The DLL itself is
neither patched nor linked into the Android target. Addresses below are virtual
addresses at its preferred image base `0x10000000`.

| Function | Evidence |
|---|---|
| `0x1005AAB0` | Strict `meshCombinator/combinatorList/rawMeshList` loading; 128 bytes per patch; neutral result clears output vector |
| `0x1005B390` | Variable lookup defaults to zero; binary32 clamped normalization; initial interpolation and neutral-index comparison |
| `0x1005B9B0` | First axis copy followed by axis additions in list order |
| `0x1005BA10` | Changed-variable update; swap previous/current axis buffers; full recombine if dirty count >= total >> 1, otherwise add/subtract changed axes |
| `0x10059270` | Linear interpolation of 32 floats; abs(ratio) < 2^-23 copies first patch without reading the next |
| `0x10059480` | 32 float additions, supports destination/source aliasing |
| `0x100595C0` | 32 `(sum + new) - old` operations in that order |

Retained JSON contains decompiler output and the initialization call-site
assembly. Decompiled argument recovery is imperfect (custom x86 register
conventions); `axis_init_asm` confirms `ecx=output`, `edx=first patch`,
`xmm3=fraction`, stack argument=next patch at the interpolation call.

The optional Unicorn oracle executes these three original math kernels, not a
translation, and covers both SIMD-global settings and output aliasing. It does
not execute the original STL maps, whole controller, or render pipeline. The
portable state/controller translation and generated MotionNode publication are
separately exercised on the actual game PSB with strict unsupported-data errors.

The existing imported MotionPlayer remains unmodified in `vendor`. All new
format handling is in `native/extensions/emote/tjs_runtime`; its generator adds
ownership and publication at the node/timeline boundaries. Verified compatibility
currently covers meshType 1 with null frame `mesh.bp`/`mesh.cc`, as used by all
78 meshCombinator nodes in `haz_a`.
