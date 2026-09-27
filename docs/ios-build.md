# iOS builds

The iOS target packages the same SDL3 launcher, Cotopha interpreter, and GLES
renderer as the other platforms. SDL provides the application entry point,
window, input, audio, and lifecycle integration. A small UIKit launcher presents
the local game library and imports folders into the app's Documents directory.
Commercial game files are never bundled by the build.

## GitHub Actions

**Build and release** builds the ARM64 device IPA on a macOS runner with Xcode,
alongside Android and both macOS architectures. It runs on pushes to `main`,
`v*` tags, and manual dispatch. There is no separate iOS workflow.

After all builds pass, the shared release job validates the device IPA's Mach-O
platform, absence of code signatures/provisioning, and hashes. It publishes the
unsigned IPA and build report with the Android/macOS packages and shared
SHA-256 checksums in the same `dev-*` prerelease or version-tag release.
No Apple account or signing secrets are configured in CI.

iOS CI validation covers the device build and package, plus Simulator library
startup and GLES presentation checked against a captured image. A successful
release does not certify gameplay on a physical device.

## Build an unsigned device IPA

Requirements: macOS with full Xcode and its iPhoneOS SDK, CMake, Python 3.9 or
newer, and Bison 3.8.2 or newer. Apple's Command Line Tools alone do not contain
the iOS SDK. Use the active Xcode selected by `xcode-select`, or set
`DEVELOPER_DIR` to an installed Xcode's `Contents/Developer` directory.

Prepare the pinned dependencies as described in [CI build inputs](ci-build-inputs.md),
then build:

```sh
python3 tools/ci_prepare_dependencies.py
python3 tools/build_sdl_ios.py --jobs 3
```

The build uses the CMake Xcode generator, `iphoneos`, ARM64, iOS 13.0 or newer,
and static SDL3. Xcode signing is disabled at configuration and build time with
`CODE_SIGNING_ALLOWED=NO`, `CODE_SIGNING_REQUIRED=NO`, and an empty identity.
Packaging removes any linker-generated ad-hoc signature from a temporary copy.
No Apple account, certificate, private key, or provisioning profile is used.

Outputs under `artifacts/entisgls-launcher/ios-arm64/`:

- `EntisGLSLauncher-unsigned.ipa`, containing `Payload/EntisGLSLauncher.app/`.
- `EntisGLSLauncher.app`, the same unsigned app ready for local signing.
- `build.json`, recording the toolchain, ARM64 and Mach-O platform checks,
  minimum OS version, bundle identifier, unsigned state, hashes, and ZIP checks.

The default bundle identifier is `io.entisgls.launcher`. Use `--bundle-id` for a
different development identifier, `--deployment-target` for a higher minimum
iOS version, and `--build-dir` or `--output-dir` for separate generated outputs.
`--dry-run` prints the configure/build commands without changing files.
Build diagnostics are in `build/ios-arm64/logs/`.

Host test executables are excluded from the iOS configuration. Packaging checks
do not establish that a game runs on an iPhone; `build.json` explicitly records
that this command has not tested the runtime.

## Simulator

On an Apple Silicon Mac with an installed iOS Simulator runtime:

```sh
python3 tools/build_sdl_ios.py --sdk iphonesimulator --jobs 3
xcrun simctl install booted artifacts/entisgls-launcher/ios-simulator-arm64/EntisGLSLauncher.app
xcrun simctl launch booted io.entisgls.launcher
```

The Simulator build uses an ARM64 Simulator binary, stored in a separate output
directory with `EntisGLSLauncher-simulator.zip`. It receives a local ad-hoc
signature required by ARM64 execution; this does not use a development identity.
A Simulator build cannot be installed on an iPhone.

The diagnostic helper used by CI creates a compatible disposable Simulator,
captures the game library, and checks a displayed GLES color pattern:

```sh
python3 tools/ci_ios_simulator_smoke.py --app artifacts/entisgls-launcher/ios-simulator-arm64/EntisGLSLauncher.app --output artifacts/ios-smoke
```

This exercises the launcher library and drawable presentation after changing
renderbuffer bindings. It does not test game scripts, full game rendering or
audio. It waits for readiness logs and explicitly terminates each diagnostic;
SDL UIKit keeps the app alive after SDL_main returns.

## Sign and install locally

An unsigned IPA cannot be launched directly on an ordinary iPhone. A local
ad-hoc signature (`codesign --sign -`) does not satisfy iPhone installation.
Installation requires an Apple development certificate with its private key
and a provisioning profile covering the app identifier and device. Keep these
credentials local; the unsigned CI build does not need them.

The normal Xcode route is to open `build/ios-arm64/EntisGLSLauncher.xcodeproj`,
select the `studysteady_sdl` app target, choose a development team and a connected
iPhone, and enable signing for that target. Override the unsigned build's
`CODE_SIGNING_ALLOWED` and `CODE_SIGNING_REQUIRED` settings to `YES`, and choose
an Apple Development identity. Xcode can manage the development profile using
the signed-in account. Run the app from Xcode once provisioning is complete.

When signing an already downloaded app with existing local credentials, work on
a copy: embed the matching `embedded.mobileprovision`, use entitlements allowed
by that profile, then sign and verify the app before installing it with Xcode's
device tools. The certificate alone is insufficient without provisioning.
Do not publish the locally signed copy or its provisioning profile as the
unsigned release artifact.

After installation, use the launcher's import action to copy a complete game
folder from Files, or copy it into the app's `Documents/Games` directory through
Finder file sharing. Each game must retain its original resource files and
configuration (`cotopha.xml`, `entis-launcher.xml`, or the supported embedded
configuration in its original executable).

The shared launcher writes saves to `savedata` inside the active game folder.
On iOS this is the imported folder under `Documents/Games`, so it remains
accessible through Files/Finder. It creates the directory on first launch;
existing saves there are retained. Changing this save policy does not change
the iOS folder import mechanism.
