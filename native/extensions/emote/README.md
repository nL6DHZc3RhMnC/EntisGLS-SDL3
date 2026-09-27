# Native E-mote texture and frame inspection bridge

This directory builds independently of the Android application. It uses the
existing PSB reader and the actual game file. The separate real TJS/NCB/Player
foundation is documented in [tjs_runtime/README.md](tjs_runtime/README.md). It does **not** emulate TJS,
return fabricated Player results, or claim a playable character renderer.

## Implemented and verified

- `motion_inspect`: exports the complete raw PSB tree as JSON, retaining binary
  references as byte offsets/sizes rather than dumping image bytes.
- `WinAtlas`: follows MotionPlayer's `spec=win` source group/icon lookup; retains
  the PSB owner; validates every atlas payload and icon rectangle; supports
  `RGBA8` and the imported `[alpha,luminance]` `A8L8` conversion. `RGBA8` upload
  uses the existing raw payload without making another 64 MiB bitmap copy.
- `uploadWinAtlasGles`: uploads a real GLES texture on the caller's current
  context and preserves texture binding/unpack alignment.
- `motion_gles_probe`: creates an EGL pbuffer context, uploads the atlas, attaches
  it to a framebuffer, and checks the full GPU readback against the CPU CRC32.
- `motion_pose_probe`: traverses `metadata.base`, follows child motions, performs
  parameter normalization and exact-keyframe selection, and records affine
  transforms and actual atlas rectangles. Parameter normalization and matrix
  evaluation are extracted from the imported MotionPlayer source at configure
  time by `tools/sdk/motion_prepare_math.py`, preserving those algorithms.

Host run on `build/game/haz_a.psb`:

```
atlas=4096x4096 format=RGBA8 icons=120 rgba_bytes=67108864 rgba_crc32=9d5638c1
visible_pixels=4786870 opaque_pixels=4165394
motions=36 layers=336 parameters=47 mesh_combinators=72 source_quads=70 live_patches=4 stencils=6 blockers=83 renderable=false
```

The full PSB contains 45 motions and 379 layers; the initial base traversal reaches
36 motions and 336 live layers. All 47 reachable parameterized layers land on
exact keyframes with the audited zero-variable/first-selector state. In particular,
`move_LR=0` and `move_UD=0` select timeline frame 30, **not** the first keyframe at 0.

The pose probe currently writes an **incomplete audit**, then exits with code 3.
Its 70 candidate atlas sources are **not a ready draw list**. The reachable graph now also flags 72 `meshCombinator` nodes that the imported
MotionPlayer does not consume. Its 83 blocking records include these 72 nodes,
four live Bezier patches (`mabuta` and `eye_pos` for each eye), six composite
stencils, and one record for missing final draw order/controller physics.
Do not draw the candidates as if they were the default character.

The audit state explicitly seeds zero variable values plus selector option 0.
It does not assert that this equals the Windows runtime's post-physics first
frame. Actual game timeline commands will also determine pose and expression.

## Build and run

From the project root:

Set `PSB_KEY` to the parameter obtained from the supplied game's configuration
or E-mote driver before running the probes below. These standalone diagnostic
tools require an explicit runtime argument; no game-specific key is embedded.

```sh
cmake -S native/extensions/emote -B build/motion-bridge-host -DCMAKE_BUILD_TYPE=Release -DENTISGLS_BUILD_DIAGNOSTICS=ON
cmake --build build/motion-bridge-host -j4
build/motion-bridge-host/motion_inspect build/game/haz_a.psb "$PSB_KEY" build/motion-bridge-host/haz_a.json
build/motion-bridge-host/motion_texture_probe build/game/haz_a.psb "$PSB_KEY" tex
build/motion-bridge-host/motion_pose_probe build/game/haz_a.psb "$PSB_KEY" artifacts/motion-haz-a-pose-audit.json
```

The last command intentionally returns 3. Codes 1 and 2 mean parsing/error and
usage failure respectively. `motion_texture_probe` can additionally export a PAM
atlas or exact cropped icon with `[output.pam [icon]]` arguments.

```sh
cmake -S native/extensions/emote -B build/motion-bridge-arm64 \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/.android-tools/ndk/android-ndk-r27c/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 \
  -DANDROID_STL=c++_static -DCMAKE_BUILD_TYPE=Release -DENTISGLS_BUILD_DIAGNOSTICS=ON
cmake --build build/motion-bridge-arm64 -j4
```

All ARM64 probes use static libc++, so they need no extra C++ shared library.
The GLES probe command on the device is:

```
motion_gles_probe /path/to/haz_a.psb "$PSB_KEY" tex
```

## Remaining renderer work, mapped to imported code

The real TJS/NCB/Player CPU foundation is now built independently in
[`tjs_runtime`](tjs_runtime/README.md). Its Android probe constructs the actual
Player tree. The following full-rendering boundaries remain:

1. The independent `tjs_runtime` now supplies real TJS variants, dictionaries,
   arrays, NCB, `Math.RandomGenerator`, and PSB v4 value dispatch. APK integration
   must replace the old PSB reader with that single real-TJS reader; linking both
   PSB libraries is invalid because they share symbol names and use different
   allocation contracts. See its README before integrating `WinAtlas`.
2. `SourceCache`, `PrivateMotionGLL`, and `D3DAdaptor` depend on Kirikiri Layer,
   Bitmap, and `iTVPRenderManager`/`iTVPTexture2D`. Their rendered output must be
   handed to an EntisGLS sprite/texture through a real adapter.
3. Default eye patches need `MotionBezierPatch.h`,
   `PlayerUpdateLayersInternal.h::mapMeshPointThroughAncestor_guess`, and
   `PlayerUpdateGeometry.cpp::updateLayersPhase3_VertexComputation`, including
   the mesh ancestor chain and source rectangle/origin conventions.
4. The six type-12 masks need `NodeTree.cpp::buildNodeTree`'s named mask links,
   `PlayerRenderItems.cpp::appendPreparedRenderItems`'s wrapper/auxiliary lists,
   and `PlayerRenderExecute.cpp`/`PlayerRenderTargets.cpp`'s stencil passes.
   Some mask targets are type-3 child Players, so using only the local atlas
   image's alpha cannot reproduce these masks.
5. Draw admission walks each motion's `priority` content in reverse, recurses into
   child Players, then stable-sorts the final list by accumulated Z. Tree order
   or atlas `metadata.zorder` is insufficient.
6. `EmoteEngine::applyMetadata_guess` and `progress` supply selector, eye,
   mouth, hair/bust/parts physics, timelines, and root transform. They must be
   preserved for a faithful default state and later motion playback.

The static snapshot probe deliberately fails unsupported interpolation, 3D,
ground correction, child overrides, live meshes, and composite stencils rather
than replacing them with identity transforms or empty callbacks. The math
extraction only represents independent numeric record fields; it does not
pretend to implement a TJS runtime.
