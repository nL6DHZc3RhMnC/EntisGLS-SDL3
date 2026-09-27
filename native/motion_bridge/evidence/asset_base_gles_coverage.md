# Complete PSB library: actual base GLES coverage

On 2026-09-23, the Xiaomi 10 Pro / Android 13 passed **86/86** original game
PSBs through real TJS, NCB, EmoteEngine metadata initialization, Player creation,
GLES drawing, and explicit release. This is new coverage: the earlier 86/86
claim covered PSB parsing, while actual GPU checks used only a small subset.

| Original archive | PSBs | GPU frames | Result |
| --- | ---: | ---: | --- |
| `stst_patch_R18.noa` | 18 | 36 | All pass |
| `psb.noa` | 68 | 136 | All pass |
| Total | 86 | 172 | No failure |

The standalone probe ran sequentially as a low-priority shell process. It did
not install an APK, change activities, clear logs, touch saves, or modify the
original archives. It opened the already-present NOAs through a read-only storage
callback and decoded the encrypted PSB header in the production reader.

For each asset it:

1. Created a new real TJS/NCB owner and standalone GLES context.
2. Checked the archive entry against the host manifest's CRC32. The manifest
   separately records the original entry's SHA-256, size, and payload offset.
3. Loaded `metadata.base`, constructed the actual EmoteEngine and Player, and
   required finite, nonempty bounds.
4. Rendered a fitted 512×512 base frame, advanced one original frame unit, and
   rendered again. Both readbacks required nonzero alpha and no GLES error.
5. Destroyed the Player and required its output GL texture to be deleted;
   unloaded the project and required texture bytes, project count, and Player
   count to be zero; checked balanced storage allocations/releases; destroyed
   the runtime owner successfully before proceeding to the next asset.

The 86 original entries total 5,971,501,184 bytes. Probe execution totaled
36,620 ms. Nonzero-alpha counts ranged from 29,001 to 48,775 per fitted frame.
The largest live texture allocation for one asset was 101,711,872 bytes, and
every asset returned that count to zero after release. The device reported
Adreno 650 / OpenGL ES 3.2.

This verifies **basic initialization, base drawing, and release only**. The two
base frame readbacks were identical for each asset, so this run does not add
evidence for animated timelines, selectors, eye/mouth movement, voice sync,
every blend combination, or complete gameplay. Nonempty output is also not an
independent pixel-fidelity comparison with the Windows driver.

## Evidence

- `artifacts/motion-assets-patch18-gles-arm64.txt`: all 18 patch results first.
- `artifacts/motion-assets-base68-gles-arm64.txt`: all 68 base results.
- `asset_base_gles_coverage.json`: combined per-file names, source hashes,
  metadata labels, bounds, alpha counts, frame CRC32s, allocation statistics,
  release results, and timing.
- Tested binary SHA-256:
  `cf764ec815ee3e1d49d9b4424039800cb35bba0ff550f170408e12c0183ab56d`.
- Manifest SHA-256:
  `4f9ee0a60dd212747ac579fb92b0f381c422c60d7c7b8caeca67243a4890e86d`.

## Reproduce without changing the game

The runtime static libraries must already be built for ARM64 Android. The build
helper uses their existing CMake flags/link recipe and the configured NDK. It
only compiles and links the standalone probe; it does not build or configure the
APK. Exact commands are retained in the output directory's `build-commands.json`.

```sh
python3 tools/motion_asset_manifest.py
python3 tools/motion_build_asset_probe.py
.android-tools/platform-tools/platform-tools/adb -s 3ffa35dd push build/analysis/motion-assets/motion_asset_batch_probe build/analysis/motion-assets/manifest.tsv /data/local/tmp/
.android-tools/platform-tools/platform-tools/adb -s 3ffa35dd shell chmod 755 /data/local/tmp/motion_asset_batch_probe
.android-tools/platform-tools/platform-tools/adb -s 3ffa35dd shell nice -n 10 /data/local/tmp/motion_asset_batch_probe /data/local/tmp/manifest.tsv /sdcard/Android/data/io.studysteady.port/files/game
```

The manifest orders the 18 patch entries first. Each result is emitted as one
JSON line, including a BEGIN record before initialization. The probe stops and
reports the exact stage on its first failure. No shared production code was
changed for this coverage check.
