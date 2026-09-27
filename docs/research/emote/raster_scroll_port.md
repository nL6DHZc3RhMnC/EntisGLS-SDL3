# RasterScroll helper

`native/legacy_super_raster.{h,cpp}` adapts the actual GLS3 scanline renderer.
`native/runtime/cotopha_port/legacy_super_raster_math.h` keeps its timing and displacement calculation
independent of the SDK so their integer and floating-point behavior can be
checked on the host. The original SDK directories are unmodified.

## Provenance and actual game parameters

The original is Entis GLS3 `glssupsprite.cpp`, copyright 2004–2007 Leshade Entis,
Entis-soft, all rights reserved. No additional redistribution license is
invented for the adapted algorithm. Source hashes:

| Source under `build/legacy` | SHA-256 |
| --- | --- |
| GLS3/Source/glssupsprite.cpp | 05fd919f6841a1821d4757b510aa8fb4416df8ae60554a352ec52e3ab103a511 |
| GLS3/Include/glsctpsprite.h | b69f0c80919b3dbe820a3e671b020d6dad9d33dd3114e9b2190311684cfe7ab0 |
| erisalib/asm/erisamatrixa.asm | f84ea4b14c24052a425608a71ca709e2f4a8c922721c12ec64500a5fffdf1221 |

In original `EFFECT_PARAM`'s 33-word record, native type 6 is RasterScroll;
flags/interval/degree-step/amplitude/wavelength/frequency are words
1/2/3/4/5/7. Word 6 is mesh division, not frequency. The private transformation
flag 4 is added at runtime without rewriting the original saved record.

The actual `ScreenData::GetEffectParam` CSX at 0x2106a uses amplitude 48 for
flag 8192 transitions and 16 otherwise, wavelength 128, frequency 6. Transition
flag 8192 selects effect flag 1 and leaves interval/step at zero, so the regular
effect-degree action controls the transition. Otherwise a nonzero cycle duration
selects interval 33 ms and step `max(1,256/max(1,duration/33))`. First occurrence
is `common1_11(_R)` line 16, `chgscrn`, 1500 ms, flags 4608; ChangeScreen adds
8192 for the transition. See `super_effect_usage.md/json` for all real scripts.

## Preserved behavior

The original computes `sin(row*pi/mesh + pi*frequency*degree/256)` with
`pi=3.14159265`. Amplitude ramps with signed degree below 256 and remains at the
configured width above it. Windows `eriRoundR64ToLInt` uses x87 `fistp`; this
port makes its default nearest-even rule explicit instead of using ARM's
half-away-from-zero `std::round`. This is not a claim of exhaustive x87/libm
bit equivalence for arbitrary inputs.

Each source row is drawn at destination minus rotation centre plus its integer
offset. Source alpha is blended normally. Only effect flag 1 adds transparency
`uint32(degree*degree)/256`. Ordinary sprite transparency, rotation, and scale
are not part of the original Raster draw function. There is an important
earlier stage: SuperSprite::BeforeMTDraw calls Refresh, and ECSSprite's
RefreshRectPostFilter (original lines 934–983) writes attached tone and alpha
mask processing into GetInfo. The helper therefore receives the modern private
filtered result from GetLegacySpriteFilteredImage, rather than bypassing those
filters or applying them twice. Target and viewport edges clip normally; there
is no wrap or filling of vacated areas.
The dirty rectangle expands horizontally by full configured amplitude, even
at degree zero. The legacy hidden-rectangle optimization returns false for this
effect; GLS4 exposes no equivalent opaque-region method on SGLSprite.

The interval counter preserves Win32 32-bit arithmetic. At each positive
interval boundary, degree advances by complete interval count times signed
step; values >=768 repeatedly subtract 256. Disabled effects reset to degree
zero only at a due interval, while interval zero never ticks. No negative
degree clamp is added. The counter resets when parameters are reapplied during
Load/Commit, matching the original unsaved interval remainder.

The modern helper reads exactly the selected source frame into a private image
of the same dimensions, depth, channel order, and alpha mode. This supports a
source that only permits ReadFrameBuffer (including GPU-backed scene snapshots)
and prevents modified/reused source pixels. It uses the actual SGL paint
context for each row; source and target clip origins were checked against
`SGLPaintBuffer::DrawImage`. Unsupported compressed/palette/layout formats,
zero wavelength, negative amplitude and unrepresentable drawing coordinates
fail explicitly.

## Integration

- Add `native/runtime/cotopha_port/legacy_super_raster.cpp` to the existing APK target.
- ECSSuperSprite owns a shared `LegacySuperRaster`, registering it through
  `SetLegacySpriteBlendEffect` before restoring the ordinary degree action.
- Apply/Commit call `Validate` then construct from the original words and saved
  degree. Save retains the existing original 132-byte wire and resource refs.
- Call `Advance(ms,LegacyEffectAnimationEnabled())` from the native sprite
  clock. In its visible draw branch, first obtain the base-refreshed source
  through GetLegacySpriteFilteredImage, then call
  `Draw(render,image,int(dst.x-center.x),int(dst.y-center.y))`.
  Check and log real Draw errors.
- Override that branch's GetRectangle with the helper's inclusive rectangle.
  Keep all these operations under the owning Sprite's SSystem lock.
- Root can add standalone `CheckLegacySuperRaster()` to the aggregate runner.
  The UI agent owns SuperSprite Apply/Save/Commit/action integration separately.

## Verification

The helper compiled as an isolated ARM64 API 29 object using the current APK
target's exact `flags.make` defines/includes. Log:
`build/analysis/raster-arm64-compile.txt` (no diagnostics).

Host math regression passed under AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
clang++ -std=c++17 -O2 -fsanitize=undefined,address -Wall -Wextra \
  tests/probes/emote/motion_raster_math_probe.cpp -o build/analysis/motion-raster-math-probe
build/analysis/motion-raster-math-probe
```

It checks 17 literal actual-game wave samples, x87 tie rounding, interval edges,
remainder, signed step, flags, and quadratic transparency endpoints. Output is
in `build/analysis/raster-host-math.txt`.

`CheckLegacySuperRaster` passed on the Xiaomi 10 Pro in the complete APK
self-test at 08:23:30, `artifacts/all-effects-self-test.txt`. It draws with the
actual SDK, testing readback-only multiframe
sources, precise golden row positions, all four target edges and a restricted
viewport, unmodified source pixels, enlarged bounds, actual alpha blending,
and the fully transparent endpoint. It does not replace drawing with a mock.
The same run passed the UI agent's integrated SuperSprite tests, including
RasterScroll's original wire, degree/ordinary-opacity separation, interval
remainder reset on Load/Commit, and second Save. This does not claim that every
later game scene using the effect has been played through.
