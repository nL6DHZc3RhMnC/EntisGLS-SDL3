# APK integration boundary

The independent target builds and runs real TJS/NCB/PSB/EmoteEngine and the
observed `haz_a` GLES pipeline on Android arm64. The SDK/Cotopha EmoteDevice and
EmoteSprite wrappers are outside this directory. Capability reflects actual
initialized backend state, and unsupported rendering operations fail explicitly.

## One PSB implementation per process

Reuse targets `motion_tjs`, `motion_ncb`, `motion_psb_tjs`, and
`motion_player_cpu` from this directory. The new `motion_apk_runtime` static
target owns these dependencies and exports `motion_apk_runtime.h`, an opaque C
boundary with no TJS, Cotopha, SDK, JNI, or GLES headers. They propagate genuine TJS headers,
NCB, Oniguruma and zlib dependencies. Do not link root `psb_reader` or standalone
`motion_psb_reader` alongside them; they define the same `PSB::*` symbols with
different allocation contracts. Remove old TJS stub include paths from all
consumers of the actual reader. `PSBRawNode` and `PSBFile` ABI must be uniform.

The real reader owns and frees `TJSAlignedAlloc` buffers. For an extracted NOA
file, decode the header once, allocate using `TJSAlignedAlloc(size, 4)`, copy the
decoded bytes, then `PSBFile::Adopt`. On a failed Adopt, use `TJSAlignedDealloc`.
Do not pass a `std::vector` buffer or `new[]` allocation. `psb_storage.cpp`
provides real filesystem loading/decryption and a host archive reader callback.
With `study_motion_set_reader`, APK archive lookup supplies **encrypted PSB
bytes**, and the same LoadStorage boundary decrypts once and adopts a genuine
TJS-aligned buffer. Do not decrypt again in the NOA callback.
Adapt `WinAtlas::loadPsb`/`native/platform/android/psb_jni.cpp` to this ownership rule before
combining atlas and Player code.

## Thread and lifetime ownership

Keep one real `tTJS` engine, NCB registry, ResourceManager, Player graph, and their
Variants on one owning thread. The imported runtime has process globals,
mutable property hints, and unsynchronized caches. `motionSetTjsEngine(engine)`
selects the host dispatch. It is not a per-thread interpreter selector.
Create genuine NCB ResourceManager and Player registrations as demonstrated in
`player_probe.cpp`/`timeline_probe.cpp`; no constructor proxy should claim an
initialized Player if registration or PSB parsing fails.

The zero-argument ResourceManager creates genuine dictionaries and
Math.RandomGenerator, and can load win PSBs without KAG/Layer. Its owner-based
constructor and full EmoteObject currently require the real Layer/KAG rendering
contract and are not interchangeable with this CPU constructor.

Preserve the ResourceManager dispatch Variant while Players use it. Destroy
Players and all PSB/Octet dispatch owners, then ResourceManager, then unregister
NCB and shut down/release TJS. GPU textures must be released on the thread that
owns their EGL context before that context is destroyed. Pass commands or raw
variable values across threads; do not share live TJS Variants concurrently.

## Verified reusable interfaces

* `PSBFile::LoadStorage` and `ResourceManager::load/findMotion`: actual local PSB
  and genuine dispatch resources. Launcher loads resolve their encrypted header
  key with `study_motion_set_psb_key_resolver`; the callback receives borrowed,
  read-only raw bytes on the owner thread. Failed resolution never falls back
  to the explicit load argument. Unencrypted headers skip resolution and still
  undergo checksum and offset validation. Standalone probes instead pass an
  explicit runtime key to `study_motion_load_project` / `motionSetPsbHeaderSeed`.
* `MeshCombinator(tTJSVariant)` accepts the actual `layer.meshCombinator` value;
  its immutable mesh bytes are copied into owned vectors.
* `updatePlayerCombinators(Player&, MeshVariables)` updates that Player's local
  nodes from **raw** variable values and marks changed nodes dirty. Call it for
  each child Player as part of its update; it deliberately does not pretend to
  advance an uninitialized child motion.
* Original `internal::parseNodeFrame_guess`, `mergeNodeFrameContent_guess`,
  `seekNodeFrameSelection_guess`, `evaluateTimeline_guess` are callable. The
  evaluator is wrapped only to publish supported new-format combinator output.
* Atlas texture upload in the separate motion_bridge project has already been
  exercised through real EGL/GLES, but its parser must be unified as above.

`texture_bridge.cpp` and `gles_render_manager.cpp` implement genuine texture
objects, uploads, normal/multiply draws, alpha testing and framebuffer stencils.
Generated `MotionRenderBackendNative.cpp`/`GlesSceneItems.cpp` retain the original
batching, Bezier and stencil algorithms. General Layer sources, repeated atlas
rectangles and other blend methods remain explicit unsupported operations.

## Opaque device owner

`study_motion_create(error, capacity)` returns a genuine initialized
TJS/NCB/ResourceManager owner. Initial capability value 15 includes TJS, NCB,
PSB v4 and Player CPU. Successful `study_motion_initialize_gles` adds the real
GLES renderer bit, giving 31. The same owner holds the Engine/Player graph and
textures throughout loading, drawing and release.

Set `study_motion_set_reader(runtime, read, release, context)` before loading a
project. For an existence check, `read` receives a null `bytes` argument and must
not allocate. For the actual read it returns an owned buffer and size; every
non-null buffer is released exactly once, including error cases. The two
callbacks must be provided together, must not throw, and the context must
outlive the reader attachment. Callback paths are passed through as game UTF-8
paths, while filesystem mode canonicalizes paths. Reader replacement with live
projects is rejected. Destroy resets the attachment before releasing TJS.

`study_motion_load_project` calls the actual ResourceManager and returns an
opaque project id. `study_motion_project_base` returns real metadata identifiers
without truncation. Duplicate paths retain the same id; unload removes both the
host id and actual ResourceManager entry. The host builds NCB's append-only
registrar index once and erases this module's registered marker on owner
shutdown, so a second owner can initialize after the first is destroyed.

## Device probes

Root has already run `motion_mesh_probe` and `motion_timeline_probe` on Xiaomi
10 Pro / Android 13, and compared the ARM64 kernel output bit-for-bit with the
Windows machine-code oracle. To reproduce with an existing writable probe
directory (replace the example directory if the device uses another location):

Set the host shell's `PSB_KEY` variable to the parameter obtained from the
supplied game's configuration or E-mote driver first. The probes take this
explicit runtime argument; these commands contain no game-specific default.

```sh
adb push build/motion-tjs-arm64/motion_mesh_probe /data/local/tmp/studysteady/
adb push build/motion-tjs-arm64/motion_timeline_probe /data/local/tmp/studysteady/
adb push build/motion-tjs-arm64/motion_runtime_probe /data/local/tmp/studysteady/
adb push build/motion-mesh-oracle/cases.bin /data/local/tmp/studysteady/
adb shell chmod 755 /data/local/tmp/studysteady/motion_mesh_probe /data/local/tmp/studysteady/motion_timeline_probe /data/local/tmp/studysteady/motion_runtime_probe
adb shell /data/local/tmp/studysteady/motion_mesh_probe /data/local/tmp/studysteady/haz_a.psb "$PSB_KEY"
adb shell /data/local/tmp/studysteady/motion_timeline_probe /data/local/tmp/studysteady/haz_a.psb "$PSB_KEY"
adb shell /data/local/tmp/studysteady/motion_runtime_probe /data/local/tmp/studysteady/haz_a.psb "$PSB_KEY"
adb shell /data/local/tmp/studysteady/motion_mesh_probe --kernels /data/local/tmp/studysteady/cases.bin /data/local/tmp/studysteady/android.bin
adb pull /data/local/tmp/studysteady/android.bin build/motion-mesh-oracle/android.bin
cmp build/motion-mesh-oracle/windows.bin build/motion-mesh-oracle/android.bin
```

The runtime probe independently verifies real creation/load/unload/destruction,
archive callbacks, duplicate-owner rejection, wrong-thread rejection and short
UTF-8 output-buffer rejection. It intentionally leaves GLES uninitialized.

## Rendering and shared EGL integration

Link `motion_apk_runtime`; include only `motion_apk_runtime.h` in the SDK bridge.
On the SDK render thread, capture the current EGLDisplay/EGLContext. Pass their
integer handles to `study_motion_initialize_gles` on the owner's dedicated
worker before loading any project. It creates its own pbuffer and shared context;
it never destroys or terminates the borrowed parent display/context. Both zero
selects a standalone context for a probe. The worker owns all subsequent API
calls. Serialize texture updates against SDK sampling.

`study_motion_create_player(project)` uses the genuine EmoteEngine, force-plays
the actual `metadata.base`, applies the complete metadata including selectors,
and progresses zero frames. Progress/durations are 60-Hz frame units, not
milliseconds. Coordinate and scale calls target the actual D3DEmotePlayer root
controllers and pass easing through unchanged. Native scale multiplies the
stored user scale by a base scale of one; the outer stage owns viewport scaling.
`is_timeline_playing("")` queries any active timeline; `stop_timeline("")`
clears all. Host PostTimeline queue semantics belong to the SDK wrapper.

The original `ststeady.exe` native interface audit maps SetCoord to player
vtable `+84` with `(x,y,duration,0)` and SetScale to `+92` with
`(scale,duration,0)`. These are D3DEmotePlayer semantics, not the script-facing
EmotePlayer ease-to-power conversion. Both native wrapper scale members start
at 1. `setScale` stores the new user scale then targets `baseScale*userScale`;
it does not multiply the previous user scale again. Root bounds testing verifies
scale 2 and coordinate `(20,-10)` transform all edges, allowing the original
per-node floor/ceil pixel rounding.

`render_player` accepts affine coefficients `(m11,m21,m12,m22,tx,ty)`, renders
the real prepared items into the actor's RGBA texture, and calls `glFinish`
before publishing its name. It does not fit or reposition the character itself.
Texture coordinates have bottom-left origin and pixels are **premultiplied
alpha**. The returned texture is borrowed: never delete it from the SDK. It
remains valid until that player's next output size change or destruction; detach
the SDK texture wrapper before either operation. The serial increments on each
successful render.

`read_pixels` is an optional fallback for software SDK images: premultiplied
RGBA8, top row first, tightly packed `width*4` pitch. Select matching blending or
convert to straight alpha at the software-image boundary. Atlas CPU buffers and
GPU output targets are distinct; use this explicit readback API for targets.

The public API probe verifies actual cross-thread texture sharing, compares
parent-context readback bit-for-bit with worker readback, and checks worker
shutdown leaves the parent context alive:

```sh
adb push build/motion-tjs-arm64/motion_runtime_gles_probe /data/local/tmp/studysteady/
adb shell chmod 755 /data/local/tmp/studysteady/motion_runtime_gles_probe
adb shell /data/local/tmp/studysteady/motion_runtime_gles_probe /data/local/tmp/studysteady/haz_a.psb "$PSB_KEY" /data/local/tmp/studysteady/runtime.pam
adb pull /data/local/tmp/studysteady/runtime.pam artifacts/motion-runtime.pam
```

Append `3` to the probe invocation to exercise an ES3 parent, matching the APK
capture; default is ES2. The worker inherits the parent's client version. Draws
use a real streamed VBO in both versions, so no ES2-only client vertex arrays
are required. After checking shared pixels, the probe plays three real metadata
timelines through nine rendered/read-back frames and checks changing output.

The Xiaomi 10 Pro run passed with 124,871 nontransparent pixels, CRC32
`054db89a`, capability 31 and identical worker/parent pixels. The log is
`artifacts/motion-runtime-gles-arm64.txt`.

## Clone and persistent state

`study_motion_clone_player` follows the original EmoteObject clone sequence:
construct a new same-project Engine, serialize the source Engine, then restore
that genuine state into the new Engine. No live TJS objects cross the opaque API.
The caller advances the clone before drawing, as it does the original actor.
The D3D shell's user/base scale fields begin at 1, while the Engine's restored
scale controller contains the saved scale. A subsequent SetScale replaces the
user scale; it does not multiply the restored scale again.

The runtime probe starts the real `腕切替A` timeline, clones its frame-5 state,
compares actual timeline cursors, verifies clone scale 2→3 and unchanged source
geometry, then advances the clone another 120 frames. Timeline metadata includes
decorative separators; those are deliberately excluded from this test.
`timeline_info` retains the native total-frame query semantics: lazy state can
report zero and non-loop entries report zero. Completion must use
`is_timeline_playing`, not a guessed duration comparison.

The original state dictionary contains timeline, eye, eyebrow, mouth,
transition, selector, base and outerforce sections. It does not serialize every
arbitrary variable, every queued selector command or transient physics data.

`study_motion_save_state(runtime, actor, &bytes, &size)` now writes this genuine
dictionary through `tTJSArrayNI::SaveStructuredBinaryForObject`. The version-1
envelope adds the exact loaded project path and D3D shell base/user scale values.
The allocated bytes remain valid independently of TJS or the runtime; free with
`study_motion_free_buffer`, including after runtime destruction. This is a new
APK envelope, not a promise to read the PC game's complete save-file format.

`study_motion_restore_state` first validates the complete binary before invoking
the actual `tTJSBinarySerializer::Read`: 8 MiB maximum, depth 32, 50,000 tree
nodes, 4,096 items per container, 65,536 UTF-16 units per string, well-formed
Unicode, no duplicate dictionary keys, no unsupported types, finite bounded
scalars and no trailing data. It validates exact section/field types, controller
sets and labels, channel-vector lengths, timeline labels/flags/cursors, wrapper
scales and project identity. Unknown selector labels therefore never reach the
original restore code's unchecked end-iterator dereference.

Restore constructs a fresh same-project Actor, applies the genuine restored
state and updates zero frames, then replaces the old Engine only after success.
The actor id, exported texture name and frame serial remain owned by the same
host actor. Rejected saves leave the old actor unchanged. The outer SDK wrapper
must separately save its project references, PostTimeline queue, curve and voice
state. The runtime probe covers the frame-5 timeline roundtrip, malformed input
rejection, failure atomicity and restoration into a new TJS owner after full
destruction. The GLES probe also checks restoration preserves its borrowed
output texture.

ARM64 exposed an idle-state serialization bug in the imported reconstruction:
native controller constructors leave unused interpolation members unwritten,
but its serializers read those members even while phase is zero. A deterministic
ASAN heap fill of `0xFF` reproduced `base/rotate/prev = NaN` from the unused
`EmoteAngleController::startRad` (`artifacts/motion-state-poison-before.txt`).
The generated EmoteEngine serializers now branch before reading idle Var,
Angle, Eye, Eyebrow and Mouth clock/exponent/previous-value fields and emit
canonical zero, one or the current value. Active fields and the live controller
algorithms remain unchanged. The same poisoned-heap test then passed, including
cross-owner restoration (`artifacts/motion-state-poison-after.txt`). The reader
still rejects nonfinite values; no NaN input has been accepted as a workaround.
