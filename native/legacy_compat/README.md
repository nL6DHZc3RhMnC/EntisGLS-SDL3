# Traditional Cotopha on Android

`cmake/legacy_runtime.cmake` builds the actual GLS3 interpreter and object
sources from `EntisGLS/EntisGLS3`. `prepare.py` writes
UTF-8/Clang-compatible copies into the build directory; vendor originals stay
unchanged. The generated code links to Android `libgls4.a`, now built from the
official SDK sources via `cmake/official_entis.cmake`, for
Sakura2, current file/NOA codecs, graphics and media.

The implementation notes below describe early porting milestones. Current
feature and device-validation status is maintained in the project README and
`analysis/native-port-status.md`; they are not a list of current missing features.

The port retains ESL strings and containers. They are not aliases for the
current `SSystem` types. `windows.h` exposes only the small platform boundary:
recursive locks, manual/automatic events, wait-any/wait-all, monotonic time,
memory information and native symbol lookup. These operations have real POSIX
implementations in `platform.cpp`.

The generated object runtime preserves the game file ABI: 32-bit serialized
fields, packed Windows class/function records, 16-bit UTF-16 strings, and
4-byte object-instruction immediates. Native pointers and errors carrying
pointers remain 64-bit. Separate patches implement UTF-16 buffer access for
naked string calls. The ARM64 port runs the portable interpreter; the old x86
JIT and Windows x86 native-call ABI are unavailable.

The library includes the original object types, `ECSContext`, execution-image
loader, class metadata, EMC records, Rosetta bridge and thread logic. File,
environment, thread, resource and sprite adapters live in `native/legacy_*.cpp`.
`Resource` owns real current-SDK images/audio players; `Sprite` owns a real
`SGLSprite`. Unported platform classes return null from an explicit registration
point and unported methods report errors. This is not a complete playable game
runtime: window/render integration, additional native classes, resource save
state and dynamic `String.Calculate` compilation still need implementation.

On the Xiaomi 10 Pro / Android 13, the media probe now decodes the original
`particle_light1.eri` to 6 × 6 / 32-bit, decodes `se517.mio` to 22,050 Hz / 1,504
samples, and completes muted audio play/stop. It also creates a real sprite
image and verifies its position, transparency, visibility and parent/child
links. This tests the native media/property operations, not an on-screen game
frame or audible output. The reference-lifetime regression, assignment opcode
`0xff`, 64-bit arithmetic, UTF-16 object serialization and naked UTF-16 memory
checks also pass on that device. See `artifacts/legacy-core-logcat.txt` for the
current run; the root task maintains the final test summary.

The current explicit unsupported boundaries are:

- Native classes including Window, Setup, ResourceManager, input filters,
  message/movie/particle/3D/render sprites, SuperSprite and ToneFilter.
- Resource MIDI loading, SaveImage, volume envelopes, pixel-rectangle/wave-data
  operations, and Resource save-state serialization/restoration.
- Sprite methods beyond the implemented image, property and hierarchy subset.
  An unimplemented method reports its name instead of returning success.
- Windows plugin DLL loading and their x86 calling convention; GLS3 x86 JIT;
  dynamic `String.Calculate` compilation; legacy multimedia plugin interfaces.

Native-class initialization is still eager. It can stop on a declared class
before the script actually uses it. `artifacts/legacy-native-binding-design.md`
describes a possible safe deferred-binding design; it has not been implemented.

Build the standalone port with the Android toolchain, ABI `arm64-v8a`, platform
`android-29`, and source directory `native/legacy_compat`. The optional
`legacy_resource_compile_probe` target compiles both media adapters. The root
APK build links `legacy_objects` plus `legacy_foundation`; its runner loads the
original `script.csx` through the current Android archive opener.

Do not compile the whole APK with the replacement `gls.h` first on its include
path. Only traditional-runtime sources should include it; JNI/current SDK code
must keep their original header. App bridges can explicitly include
`"legacy_compat/gls.h"`.

`String.Calculate` remains required by this game's later condition/dynamic
command paths. Its current explicit unsupported error is a pending port, not a
permanent substitute. Likewise, compiling or initializing native classes alone
does not demonstrate successful rendering, audio playback or gameplay.

All generated validity checks use `ECSObject::IsValidObject(pointer)`. Do not
restore calls through a nullable receiver: a non-static method checking `this`
is undefined C++, and defensive compiler flags on only some callers cannot make
header-inline weak definitions safe. The original media crash and the fix are
documented in `artifacts/legacy-null-reference-audit.md`.
