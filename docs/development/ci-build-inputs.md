# Clean-checkout build inputs

The SDL launcher builds from the committed official EntisGLS SDK, SDL 3.4.16,
FreeType 2.14.3, motion runtime sources, and redistributable font assets. The
`StudySteadyR18/` game directory, original EXE/DLL/NOA files, local settings,
and previously generated build products are not build inputs and are not
included in release packages.

Before configuring either Android or macOS, run:

```sh
python3 tools/ci/ci_prepare_dependencies.py
```

This downloads only the upstream release archives and font license used to
verify the committed inputs. Their URLs and SHA-256 values are pinned in
`tools/sdk/setup_sdl3.py` and `tools/sdk/setup_sdl_fonts.py`. Existing source, font and
license files must match; preparation does not replace them. Downloads are
stored in `build/downloads/` and may be cached between runs. A corrupted cache
is rejected. `--offline` requires the cached verification inputs to exist.
`build/ci/dependencies.json` records the verified input hashes.

The build also requires Python 3, CMake 3.20 or newer, and **Bison 3.8.2 or
newer**. Set `TJS_BISON` to a host Bison executable or put it on `PATH`; on
macOS the system Bison is too old, so install the Homebrew `bison` formula.
`ENTISGLS_CMAKE` can select an explicit CMake executable. No private absolute
toolchain paths are required.

## macOS

Run on a macOS host with Xcode command-line tools:

```sh
brew install bison
export TJS_BISON="$(brew --prefix bison)/bin/bison"
python3 tools/ci/ci_prepare_dependencies.py
python3 tools/build/build_sdl_desktop.py --arch arm64 --jobs 3
# Intel variant:
python3 tools/build/build_sdl_desktop.py --arch x86_64 --jobs 3
```

Both architectures can be cross-compiled on either macOS host. The GitHub
workflow uses native runners for each architecture so native test executables
can run as well. Minimum deployment target defaults to macOS 11.0.

Archives are written under
`artifacts/entisgls-launcher/macos-{arm64,x86_64}/EntisGLSLauncher.zip`.
Packaging verifies the architecture, font bytes, ZIP CRC, and the signature
of a separately extracted application. `build.json` records these checks.
The signature is **ad hoc**, not Developer ID signing or Apple notarization;
automated compilation does not claim that Gatekeeper distribution requirements
have been satisfied. This pipeline does not remove quarantine attributes.

## Android

The same dependency preparation runs on a Linux runner. Use the SDK and NDK
paths passed by the workflow, not the ignored `.android-tools/` directory.
Android packaging and signing configuration are documented with the release
workflow. No game resource import is needed to produce the APK.
