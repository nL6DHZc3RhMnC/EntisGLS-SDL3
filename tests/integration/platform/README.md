# SDL PCM output checks

This standalone target needs SDL3 but does not need the EntisGLS platform
overlay. Point `SDL3_DIR` at an installed or existing build-tree SDL3 config:

```sh
cmake -S native/platform/sdl/tests -B build/sdl-audio-check/cmake \
  -DSDL3_DIR="$PWD/build/sdl3-host-probe/build/sdl3"
cmake --build build/sdl-audio-check/cmake
ctest --test-dir build/sdl-audio-check/cmake --output-on-failure
```

The test explicitly selects SDL's dummy audio device. It validates actual
AudioStream conversion, postmix PCM values, channel gains, queue consumption,
natural static completion, nested pause/restart, byte-offset seeking, looping,
streaming backpressure, decoder EOF calling Stop from inside the refill
callback, listener quiescence, and propagation of device-open failure. It does
not verify physical speaker output or game-level MI0/MEI integration.

The SDK wrapper is `../sdl_sound_player.cpp`. Register it using
`SakuraGL::RegisterSDLSoundPlayer()` after `SSystem::Initialize()` and before
creating the first player. Its portable core uses no Android or Apple APIs.

`GetPlayingPosition()` counts source sample frames consumed from SDL's queue,
excluding bytes merely accepted by Write. It is an output-queue position,
subject to the audio device's hardware latency, rather than a fabricated
wall-clock estimate. Gain changes apply to subsequently submitted PCM; the
default source queue is capped at 50 ms. `PrepareStream` can explicitly request
a larger queue (up to two seconds). Streaming loops remain decoder-owned.

Refill listeners run on a separate worker without any player or SDL stream
lock. The SDK decoder can call Write, GetPlayingPosition, and Stop there.
Close and SetListener wait for an already-running callback, so their caller
must release any SDK/UI lock that the callback needs. Do not destroy or Close
a player from its own callback; Close reports failure in that case. The object
owner must serialize Open/Close/destruction. Unsupported PCM formats, more
than two input channels, stream seeks, unknown playback flags, and output
device failures are not silently accepted.

The legacy `ptfDevice` control uses `../device_volume.cpp` and the common
`native/runtime/cotopha_port/legacy_device_volume.h` declarations in SDL builds. It is an application
master gain, multiplied by each player's own channel gain, not an OS-wide
volume change. The registry applies `SDL_SetAudioStreamGain` to every live
stream and initializes future streams with the same value. The getter verifies
the gain on all registered streams; setters report an SDL failure and roll back
already-updated streams rather than publishing a partially applied value.
Independent tests inspect real postmix PCM for both an already-playing stream
and a player created after a master gain update. Link `device_volume.cpp` in the
SDL target instead of `legacy_android_audio.cpp`; the original Android JNI
backend continues to implement its original media-stream volume behavior.

## Image codecs

The same CMake project builds `image_codec_test`. Its genuine encode/decode
round trips verify every PNG RGBA byte, including nonzero RGB under zero alpha,
odd image widths, embedded stream offsets, JPEG color tolerance and opaque
alpha at quality 0/75/100, and propagation of short writes and invalid input.
Run only these checks with `ctest --test-dir build/sdl-audio-check/cmake -R
image_codec --output-on-failure`.

`../image_codec.cpp` uses SDL 3.4's PNG IO APIs and a private, static stb JPEG
implementation from `vendor/official-tinygltf`. Add that directory to the target
include path. Static stb linkage prevents collisions with the SDK's tinygltf
translation unit. SDL 3.4.16's PNG writer treats a nonzero short write as success;
the adapter therefore encodes to SDL memory first and validates the exact byte
count written to the caller's actual output stream.

`../sdk_image_codec.cpp` bridges borrowed `SFileInterface` streams to SDL_IO and
registers PNG/JPEG using `SakuraGL::RegisterSDLImageDecoder()`. It keeps original
ERI/BMP/TGA/PSD decoders. The encoder requires straight ARGB32 snapshots, the
format already produced by `LegacyEncodeImage`; it explicitly rejects other
pixel layouts. JPEG drops alpha; PNG preserves it without premultiplication.
Codec image dimensions are limited to 32,768 per side and 64 Mi pixels, matching
the export pipeline's pixel ceiling; compressed input is limited to 256 MiB.
Unknown-length `SFileInterface` streams expose no separate EOF/error channel,
so zero reads must be treated as EOF. Known-length premature EOF and all short
writes fail.

The real NDK compiler syntax-checked both SDK codec/audio wrappers and both
the SDL and original Android branches of `legacy_image_export.cpp`. This does
not establish a full macOS SDK build or end-to-end screenshot export; those
remain integration checks for the complete SDL engine target. SDK source files
and bitmap font handling were not changed.

## Window presentation

`sdl_presentation_smoke` is a manual test requiring a real desktop display and
OpenGL 2.1; it is intentionally not part of headless CTest. After building the
target, run `build/sdl-audio-check/cmake/sdl_presentation_smoke`. It checks actual
window creation with immediate swaps, repeated hidden/shown transitions,
minimize/restore, and synchronous worker requests serviced by the SDL main
thread throughout those states. Hidden/minimized windows must not present;
shown/restored windows must really swap buffers. The log records the renderer,
actual frame counts, window flags, callbacks and longest observed swap time.

The application uses the same `presentation.h` policy. SDK tasks/events drain
before the per-window 60 Hz draw deadline, so frame pacing never suspends their
main-thread dispatch. It excludes occluded windows as well. Immediate swaps
avoid SDL Cocoa's unbounded display-link wait at positive swap intervals;
driver/GPU execution can still consume time, so this is not a promise that
every swap returns within a fixed number of milliseconds.
