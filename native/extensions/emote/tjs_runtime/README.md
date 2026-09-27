# Real TJS / NCB / EmoteEngine and GLES renderer

This is an independent native CMake project with an opaque APK integration API.
It builds the imported **real** TJS interpreter, dictionary/array/variant objects,
Oniguruma regular expressions, and NCB native class bindings. There are no fake
TJS dispatches that return success without doing work.

Verified on the Xiaomi 10 Pro / Android 13:

- `motion_tjs_probe`: executes a real script manipulating a Dictionary and Array;
  returns `StudySteady:5:4`, and constructs `Math.RandomGenerator`.
- `motion_ncb_probe`: real NCB class registration, constructor, property setters/
  getters, and a native method call return 42.
- `motion_player_probe`: the real ResourceManager loads `haz_a.psb`, resolves
  `motion/all_parts/全体構造`, and the original Player/NodeTree methods construct
  **26 local nodes including the synthetic root, 13 child Player instances, and
  3 parameters**. `move_UD`, `move_LR`, and `body_slant` normalize to 30.
  The child Players are created here; their own motion playback is not advanced.
  `artifacts/motion-player-arm64.txt` records the device run.
- `motion_psb_tjs_probe`: reads metadata through the genuine PSBValueDispatch and
  converts the 64 MiB atlas to a genuine TJS Octet. The extended probe also checks
  a 384-byte PSB v4 `rawMeshList` resource through that same dispatch path.
- `motion_scene_gles_probe`: actual EmoteEngine metadata/selector initialization,
  Player playMotion/frameProgress, recursive child hierarchy, texture sources,
  Bezier meshes, priorities and stencil batches produce the default `haz_a`
  character on GLES. Three frames each have 124,871 nontransparent pixels;
  the graph contains 36 Players and 406 nodes. The observed normal/multiply
  blend modes use the imported renderer's original expressions. ResourceManager
  release returns tracked texture bytes to zero. See
  `artifacts/motion-scene-gles-arm64.txt` and `artifacts/motion-scene.png`.

The first GPU image exposed a duplicate red/blue swap and missing metadata
selectors. Both were fixed: retain the original Win resource color conversion
once, and initialize the complete original EmoteEngine. The latest actual image
was visually checked for natural colors and one selected pair of hands.

The Player target preserves whole CPU source files for PlayerCore,
PlayerVariable, PlayerMotionLoad, PlayerUpdateLayerEval, PlayerFrameProgress,
NodeTree, MotionNodeBridge, and RuntimeSupport.
For ResourceManager, layer ownership, and layer queries, the generator extracts
complete original functions without changing their bodies. Local file loading
and PSB header decoding are implemented by `psb_storage.cpp`.

Unsupported general Kirikiri Layer/source operations fail explicitly rather than
pretending to render. The supported Win atlas path uses real texture objects,
real geometry dispatch classes and the original Player render-item/backend
algorithms. This does not establish that every game asset or the full game is
playable. `motion_timeline_probe` separately exercises the
original parse/merge/two-slot seek/interpolate methods on all 45 `haz_a` motions
and 379 nodes: 2,940 forward/backward samples, including 560 crossfade samples.
That narrow probe is complemented by the complete Engine/child/render pipeline
exercised in the scene probe.

## Do not link both PSB readers into the APK

The root project's `psb_reader`/standalone `motion_psb_reader` and this project's
`motion_psb_tjs` both define the same `PSB::*` C++ types and symbols. They have
**different allocation contracts**. The small parser adapter uses `new[]`, while
the real TJS parser frees buffers with `TJSAlignedDealloc` and must receive
`TJSAlignedAlloc` allocations. Combining these libraries is an ODR violation and
can produce invalid frees even when names happen to link.

The APK has selected the single real TJS reader and adapted `native/platform/android/psb_jni.cpp`
ownership. The old standalone atlas/pose project remains a separate executable;
do not link its duplicate parser into this runtime.

## Reproducible build

All source dependencies required for this target are checked in under
`vendor/kirikiroid2` and `vendor/motion-deps`, with source paths, SHA-256 hashes,
and licenses. The user repository is not consulted by a normal build.
Bison 3.8.2 or newer generates the parser from the original `.y` files; CMake and
a C/C++ compiler must be installed. The Android build uses the project's NDK r27c,
API 29, arm64-v8a, and static libc++.

From the project root, explicitly select the build tools:

```sh
python3 tools/diagnostics/motion_build.py --platform both \
  --cmake /Users/fenghengzhi/Developer/toolchains/krkr2/cmake-pkg/cmake/data/bin/cmake \
  --ndk "$PWD/.android-tools/ndk/android-ndk-r27c" \
  --generator 'Unix Makefiles' --jobs 4
```

The CMake path above is a locally installed 4.4.2 binary. A different working
CMake can be passed with `--cmake`. The script verifies all imported hashes,
then builds both independent projects without changing the root APK CMake.
Build trees are `build/motion-bridge-{host,arm64}` and
`build/motion-tjs-{host,arm64}`. No device installation occurs.

Run the host success/negative/lifecycle checks:

```sh
python3 tools/diagnostics/motion_verify.py
```

This runs real TJS and NCB scripts, PSB-to-dispatch/Octet reads, 25 repeated Player
construction/destruction cycles, and rejection of a wrong seed, truncated PSB,
and missing file. It also exercises real EmoteEngine owner lifecycles, archive
callbacks, root controllers and the complete CPU scene hierarchy. It reports
exit codes and diagnostics in
`artifacts/motion-verification-host.json`. A pass verifies this CPU surface,
not character rendering or the complete game.

Host AddressSanitizer + UndefinedBehaviorSanitizer checks run with
`halt_on_error=1`, including the 25 construction/destruction cycles.
Results are in `artifacts/motion-verification-asan.json`; this is a memory-access/UB
check of the exercised CPU path, not proof of leak freedom or renderer correctness.

For a sanitizer build, use `motion_build.py --platform host --sanitizers` and then
`motion_verify.py --build-dir build/motion-tjs-host-asan` (choose a distinct
`--output` path when preserving ordinary results).

## Sources and adaptation boundaries

- `tools/sdk/motion_import_tjs.py` is the optional import/refresh tool. It copies only
  TJS, four NCB files, two rectangle files, the renderer interface header, logging
  header, and the source locale map. It also selects real fmt/spdlog headers,
  Oniguruma source, and compiler-observed Boost header dependencies. It does not
  copy the whole Kirikiri repository or the full 86 MiB Boost installation.
- `vendor/motion-deps/provenance.json` records all new imported bytes. The original
  MotionPlayer/PSB manifest remains `vendor/kirikiroid2/provenance.json`.
- `tools/sdk/motion_prepare_tjs.py` changes NCB's includes to the real TJS host
  services and compiles the original English message map into a local map.
  A generated `tjsConfig.cpp` fixes bounded UTF-16 copying to check remaining
  length before reading the next code unit; ASAN caught the original reading
  beyond the one-character source used by `ttstr('/')`. Vendor remains intact.
- `tools/sdk/motion_prepare_tjs_psb.py` retains PSBValueDispatch and updates its PSB v4
  resource classification/lookup. The generic PSB script-class/media registry
  is outside this target; adopted/native-loaded files have the real dispatch.
- `tools/sdk/motion_prepare_player.py` prepares generated copies and extracts the
  selected original ownership/resource/query functions. Vendor originals are
  unchanged. Generated files live only in `build/generated`.

## Game-specific mesh combinator support

`haz_a.psb` contains **78 `meshCombinator` nodes and 278 binary `rawMeshList`
records**. Their sizes match `meshCount × 16 × 2 × sizeof(float)`. At the recorded
neutral indices, 78 records contain the regular Bezier grid and 200 contain zero
vectors. The actual algorithm is now recovered from the game's own Windows
`emotedriver.dll`; addresses and retained pseudocode are in `../evidence`.

The imported vendor MotionPlayer does not read those new fields. Generated
NodeTree/MotionNode adapters now own a strict `MeshCombinator` per relevant node,
and an evaluation wrapper publishes the combined patch after the unchanged
legacy timeline evaluator. `updatePlayerCombinators(player, rawVariables)` marks
changed nodes dirty; it consumes raw E-mote variables, not the old normalized
parameter-table values. The actual DLL normalizes each float into `[0, meshCount
- 1]`, interpolates two consecutive 16-point patches, sums entries in their
recorded order, and clears the output when all entries are neutral. Updates use
the DLL's full-sum versus `(sum + new) - old` dirty-count split. FP contraction
is disabled for these kernels to retain Windows binary32 operation order.

`motion_mesh_probe` parses all 78 combinators/278 axes through genuine TJS,
performs 1,112 axis updates, verifies neutral returns, and rejects unsupported
mesh types, truncated raw resources, zero ranges, and nonfinite variables.
`motion_timeline_probe` additionally verifies all 78 generated real MotionNodes
receive the non-neutral patch and clear it at neutral. Simultaneously nonempty
timeline `mesh.bp`/`mesh.cc` and combinator data is explicitly rejected: the
observed 78 nodes all have null timeline mesh payloads, and precedence for a
different asset has not been verified.

Optional machine-code differential validation:

```sh
python3 -m venv build/motion-oracle-venv
build/motion-oracle-venv/bin/pip install unicorn==2.1.4
build/motion-oracle-venv/bin/python3 tools/diagnostics/motion_mesh_oracle.py
```

This executes only the actual DLL's three bounded arithmetic kernels in x86
emulation, with both SIMD settings and separate/in-place output. The host C++
output matches **3,505 cases / 14,020 native executions bit-for-bit**, including
all 278 real raw mesh resources. `artifacts/motion-mesh-oracle.json` records DLL
and output hashes. This is a kernel oracle, not execution of the entire original
mesh controller or a rendered-image comparison. Inputs/Windows expected output
are also saved under `build/motion-mesh-oracle` for Android byte comparison.

Source resolution, hierarchy transforms, live Bezier geometry, composite
stencils, controller physics and priority/Z ordering are now connected for the
observed `haz_a` pipeline. Unsupported blends, out-of-atlas repeated textures,
and generic Kirikiri Layer sources raise errors. The old independent pose audit
does not use this real TJS/Player adapter and still reports its own combinator
blocker. See `INTEGRATION.md` for the opaque API and shared-texture lifetime.
