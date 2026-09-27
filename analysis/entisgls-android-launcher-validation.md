# EntisGLS Launcher Android 0.3.0-dev validation

The generic SDL Android launcher completed a full configure, native compile and
APK package build. No APK was installed and no phone was launched in this task.

## Artifact

- APK: `artifacts/entisgls-launcher-arm64-dev.apk`
- Size: 76,715,406 bytes
- SHA-256: `1e842e68287585d0dd106e7352ec1f09f3377b48e6ca6902a58dc90418dc89ae`
- Package: `io.studysteady.port`; visible label: `EntisGLS Launcher`
- Version: `0.3.0-dev`, code 4; ARM64; min API 29, target API 35
- Build report: `artifacts/entisgls-launcher-arm64-dev.build.json`
- Additional validation: `artifacts/entisgls-launcher-arm64-dev.validation.json`

The package ID, Activity names and development signing key remain compatible
with the previously installed SDL app. Signature verification passed and the
certificate SHA-256 matches the preceding SDL APK:
`92772bdaa340ad02b7c2b467fa967ff1a0f57350a540484fbe1456073d90578b`.
Both prior StudySteady APKs remained byte-identical.

## Checks completed

- Full native compile/link of `libmain.so` and required libraries passed.
- Java, DEX and manifest compilation passed.
- APK signatures, ZIP CRC and `zipalign -P 16` passed.
- All three packaged libraries are ARM64 and have 16 KB ELF LOAD alignment.
- Packaged native SHA-256 values match build inputs.
- Manifest binary confirms the expected version, label and ABI.
- The compatibility profile is byte-identical to
  `assets/compatibility/study-steady-r18.xml` (SHA-256
  `4c1c3981b05a4227a136bf2f7bccb3f7405a7f33fac18a4de6ccecdd6ec7d122`).
- Optional Noto Serif CJK TC font and OFL license are bundled and the font hash
  matches its provenance. No root `assets/cotopha.xml`, NOA, CSX or EXE is packaged.
- `--check-shell --without-bundled-fonts` also passed; its report contains an empty
  `bundled_fonts` list, proving shell packaging does not require the game font.
- Eighteen host tests exercise production Java discovery/path logic with a fake
  document provider: nested/Unicode paths, empty ancillary files, unknown source
  sizes, candidate selection, cancellation, traversal, case collisions, provider
  cycles, depth limits, app-readability and link-safe staging cleanup.

## Scope and pending device checks

The launcher now lists independent imported games, recursively imports complete
game directories and passes the selected path to native configuration detection.
It retains the old `game/` entry and supplies `--legacy-local-data` only for that
entry. New imports never replace other games or write to save directories. Native
code controls recognized compatibility profiles and per-game save identities.

The Java host fixtures do not emulate Android SAF permissions, JSON transaction
recovery, process death or Activity lifecycle. Those flows and gameplay through
this new launcher still require phone testing. Successful packaging does not
establish that arbitrary EntisGLS games are supported.

## Commands and evidence

- Full workflow: `python3 tools/build_sdl_android.py --jobs 3`
- Discovery/path tests: `python3 android/sdl/tests/test_import_discovery.py`
- Shell: `python3 tools/build_sdl_apk.py --check-shell`
- Font-free shell: `python3 tools/build_sdl_apk.py --check-shell --without-bundled-fonts`
- Full log: `build/android-entisgls-launcher-build.log`
- Stage logs: `build/android-sdl3/logs/{configure,native-build,package}.log`
- Extra validation: `build/android-entisgls-package-verification.log`
- Signer/manifest: `build/android-entisgls-apksigner.txt`, `build/android-entisgls-badging.txt`
- No-font report: `build/android-sdl-generic-shell-no-fonts.json`

The build driver now explicitly enables `ENTISGLS_LAUNCHER`; the old
`STUDYSTEADY_SDL3` option and internal target names remain accepted for existing
build-directory compatibility. The final incremental native build explicitly
enabled `ENTISGLS_LAUNCHER=ON`. It includes topologically ordered font aliases,
explicit game IDs, inspection without save-directory side effects, guarded legacy
save migration and independently named Regular/Bold OpenType faces.
