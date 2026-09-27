# SuperSprite original save wire and Android implementation

The Android `ECSSuperSprite` now implements the original `Nothing` and
`TileImage` records, with reconstruction through the real Sprite/Resource
backend. Other effect types are explicit errors. This removes the previous
unconditional Save/Load error for even an unused SuperSprite, which the actual
game stores in ScreenData before autosaving.

## Source and executable evidence

Read-only original GLS3 source inspected:

- `build/legacy/GLS3/Source/glssupsprite.cpp`, SHA-256
  `05fd919f6841a1821d4757b510aa8fb4416df8ae60554a352ec52e3ab103a511`.
- `build/legacy/GLS3/Include/glsctpsprite.h`, SHA-256
  `b69f0c80919b3dbe820a3e671b020d6dad9d33dd3114e9b2190311684cfe7ab0`.

The supplied `ststeady.exe`, SHA-256
`9750f67ae51ba158cda4f756ccbd0875fb837ddd15e84db713be53f8573d91b3`,
confirms this ABI. SuperSprite's vtable starts at `0x940034`:

- Save `0x54F890` calls the Sprite base, writes size **132**, writes the
  parameter block at `this+2168`, then saves the reference at `this+2468`.
- Load `0x54F980` accepts a size at most 132, zero-fills the parameter block,
  reads that many bytes and loads the reference. Effect 9 has extra mesh data.
- Commit `0x54F640` calls the Sprite base and commits the resource reference.
  For effects other than 9, it copies exactly 132 bytes and calls the real
  SetEffectParameter at `0x54CE20`.

Retained decompilations are in `super_sprite_save_wire.json`. No original
source or executable was modified.

## Fixed-width layout

After the existing ECSSprite base record:

1. A little-endian u32 parameter byte count (132 for new saves).
2. That many parameter bytes, with the following Win32 offsets.
3. The original `m_refParticleImage` ECSReference record.

| Offset | Value |
|---|---|
| 0 | i32 effect type: Nothing=0, TileImage=1 |
| 4 | u32 requested flags |
| 8–28 | Six i32: interval, degree step, shaking width, mesh size, mesh division, frequency |
| 32, 36 | i32 viewport width and height |
| 40, 44 | i32 scroll speed X and Y |
| 48 | Original process image pointer, Win32 u32 |
| 52, 56 | i32 alpha range and milliseconds per degree |
| 60, 64 | i32 smash point X and Y |
| 68–80 | Four f32: delay, power, random power, deceleration |
| 84–128 | Four vectors of three f32: velocity, gravity, rotation speed, random rotation |

Android keeps 33 explicitly encoded u32 words. It never writes a native C++
struct or an Android pointer. Offset 48 is written as zero and ignored on read;
Nothing/TileImage never use this field. Resource ownership and restoration use
the actual ECS references. Other inactive parameter fields are retained.
Fresh no-effect objects have a deterministic zeroed parameter block.

The script SetEffectParameter path records all original parameter fields and
defaults, including alpha range 1 and milliseconds per degree 1000. The
optional effect-image reference is indexed, saved and committed. The port's
hidden index -11 supports reference lifetime; it is not an extra wire field.

Scroll coordinates and the interval remainder are **not saved**. Reapplying
SetEffectParameter during Commit resets both to zero, exactly as in the old
implementation. The tiled viewport is generated output and is recreated from
the base's real resource/clip reference; its pixels are not added to the wire.

## Scope and failures

Supported effects are Nothing and TileImage. Load rejects unknown effects
(including MeshWarp=9) before attempting unsupported mesh restoration. Tile
dimensions and allocation sizes are bounded and negative intervals rejected.
Read/write errors propagate. Save rejects an uncommitted restore and anonymous
tile input without restorable resource provenance. Commit propagates base or
reference errors and rejects TileImage without an actual restored image.
Unlike the old Commit, the port does not suppress failed base reconstruction.

## Validation and integration

`native/legacy_super_sprite.cpp` passed a direct NDK ARM64 compilation using
the real `legacy_objects` flags. Artifact:
`build/motion-super-sprite-arm64.o`; diagnostic log:
`build/motion-super-sprite-arm64.compile.log`. This is compile validation, not
a claim that the new object-graph probe has already passed on the phone.

The existing `CheckLegacySuperSprite` now takes `ECSEnvironment&`; its caller
must pass the already loaded game environment. No new CMake target is required.
The probe retains the previous real 2x2 tiled-pixel test and additionally:

- Loads `particle_light1.eri` from the mounted original NOA and invokes the
  actual script AttachImage/SetEffectParameter methods with a 2x2 crop.
- Saves global source, TileImage and Nothing sprites under a static scene;
  checks the precise 132-byte block and zero pointer.
- Destroys all globals and recreates them through the real factory, checking
  the same static parent, reopened resource references and Nothing's buffer.
- Checks exact rendered source pixels, scroll reset, the 15+1 ms interval
  boundary and a second full graph save.
- Requires errors for truncation, oversized block, unsupported mesh type,
  zero tile dimensions and structurally valid TileImage without a source.

The caller context and primary-context pointer are preserved using an isolated
execution image. The root agent ran the complete probe on the Xiaomi 10 Pro /
Android 13 successfully: **1,671-byte graph**, all reconstruction/pixel/negative
checks PASS. Evidence is `artifacts/heap-thread-full-self-test.txt`; the unified
self-test completed at 2026-09-23 **06:42:40.928** device time. This validates
the component probe, not the later continuation of a complete game save.
