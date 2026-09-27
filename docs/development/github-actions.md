# GitHub Actions builds and releases

`.github/workflows/build-release.yml` builds Android ARM64 on Ubuntu, macOS Intel
on `macos-15-intel`, and macOS Apple Silicon plus an unsigned iOS ARM64 device IPA
in separate jobs on `macos-15`. Each build has its own checkout. No game directory
or local `.android-tools` directory is used.
Pinned inputs and clean-checkout preparation are described in [CI build inputs](ci-build-inputs.md).

## Triggers and artifacts

- Push to `main`, or run **Build and release** manually: publish a development
  prerelease named `dev-<run>.<attempt>-<commit>`.
- Push a `v*` tag: publish that version. Tags containing a hyphen remain
  prereleases (for example `v0.4.0-rc1`). Update the app manifests/version first.
- Publication happens only after all four packages and their checks pass.
  A failed build leaves diagnostics in Actions and does not publish a release.

Releases contain the signed APK, two app ZIPs, the unsigned device IPA,
build/native-test reports and `SHA256SUMS.txt`. macOS signing is ad-hoc, with archive extraction and signature
verification; this workflow does not perform Developer ID signing or notarization.
The native tests run the actual Cotopha interpreter with generated fixtures and
SDL dummy drivers. They do not require commercial game resources or a display.
Android packaging verifies the APK signature, ZIP alignment and ELF 16 KB alignment.
iOS packaging checks the IPA checksum, archive contents and ARM64 device Mach-O,
and rejects code signatures or provisioning data. The IPA must be signed locally
before installation; no Apple credentials are used by CI. See [iOS builds](ios-build.md).
Device gameplay still needs separate testing.

Build jobs have read-only repository access. Only the final release job receives
`contents: write`; no personal access token is needed by the workflow. Releases
are uploaded as drafts, then published after every asset upload succeeds.
Existing releases are never overwritten. If a version-tag run leaves a draft
after an upload failure, review/remove that draft before rerunning it.

## Manual iOS Simulator diagnostics

The separate **iOS Simulator diagnostics** workflow is defined in
[`.github/workflows/ios-simulator.yml`](../../.github/workflows/ios-simulator.yml).
It has only a `workflow_dispatch` trigger: use **Actions → iOS Simulator
diagnostics → Run workflow** when a Simulator check is needed. Pushes and tags do
not start it. It does not publish a release and is not a dependency of **Build
and release**.

The manual workflow builds an ARM64 Simulator app and checks the game library
and GLES presentation without commercial resources. The rendering check creates
a shared motion context and changes renderbuffer bindings, then verifies the
displayed green/red pattern and landscape orientation in a Simulator screenshot.
UIKit scene orientation and drawable dimensions determine how screenshot pixels
are sampled: simctl can return portrait panel coordinates for a landscape app.
The check does not infer orientation from PNG width alone or choose a rotation
just because it produces the expected colors.
The resizable game window uses the production orientation helper. Readiness markers and
explicit termination are used because SDL UIKit does not exit when SDL_main
returns. Build logs, screenshots and the smoke report remain in Actions artifacts.
These diagnostics do not establish physical-device gameplay compatibility.

The harness launches the app with separate stdout/stderr files and verifies the
returned Simulator process PID before accepting readiness or a screenshot. Its
report records each command and stage. Failures retain the original error,
available screenshots, app process samples and filtered system logs; termination
or device cleanup errors are reported separately and still fail the run.

The Simulator workflow saves its compiler cache after a successful build, before
runtime checks, so a Simulator startup failure does not discard compiled objects
needed by the next diagnostic run.

## Caches shared across workflow runs

Every build job restores and saves a bounded `ccache` directory under
`build/compiler-cache`. The cache is separated by platform, architecture and a
fingerprint of the runner image, ccache and actual Clang toolchain. Apple keys
also include the active Xcode and SDK versions. The write key contains the source
commit and run attempt; the restore prefix omits them so later commits can reuse
unchanged compilations. Compiler contents and compilation inputs still determine
individual ccache entries. Changing the SDK or compiler creates a separate cache.

Each cache is limited to 1 GB. Build steps clear the statistics before compiling,
then require at least one cache hit or miss to prove that compilation reached
ccache. Actions diagnostics include `build/ci/ccache.json` with hit/miss counts;
the first run normally has misses, and subsequent runs can reuse its objects.
Native tests, archive verification and signing still run on every build.

Download caches are independent of compiled objects:

- `build/downloads` stores pinned dependency verification archives. Its key
  includes the dependency setup scripts and manifest. Every run verifies archive
  hashes and committed source contents even when the cache is restored.
- Android caches the configured NDK, API 35 platform, build-tools 35.0.0 and
  platform-tools under the runner SDK directory. `sdkmanager` still checks that
  the required packages are installed after restoration.
- macOS/iOS cache only Homebrew's `~/Library/Caches/Homebrew/downloads` directory;
  Bison and ccache are installed normally from those downloads when available.

Caches never contain signing keys, profiles, Apple credentials, game resources
or whole build directories. A cache miss only makes the normal build/download
steps run again. The workflow pins `actions/cache` to a specific commit.

## Persistent Android signing

Configure these **repository Actions secrets** before the first run:

| Secret | Value |
| --- | --- |
| `ANDROID_KEYSTORE_BASE64` | Base64 of the existing Android keystore, with no line breaks |
| `ANDROID_KEY_ALIAS` | Signing key alias |
| `ANDROID_STORE_PASSWORD` | Keystore password |
| `ANDROID_KEY_PASSWORD` | Optional separate key password; defaults to the store password |

Reuse the existing local `.android-tools/development.keystore` if Actions APKs
must upgrade installations with the same application ID signed by this checkout.
The current ID is `io.entisgls.launcher`; it installs separately from the old
`io.studysteady.port` app and does not automatically migrate its data.
Switching the signing key prevents direct updates of same-ID installations. Keep a separate backup of the
keystore; GitHub Secrets are not a retrievable key backup.

An authenticated repository maintainer can upload the existing keystore without
printing it or writing its encoded form to a file:

```sh
python3 -c 'import base64,pathlib,sys; sys.stdout.write(base64.b64encode(pathlib.Path(".android-tools/development.keystore").read_bytes()).decode())' | gh secret set ANDROID_KEYSTORE_BASE64 --repo nL6DHZc3RhMnC/EntisGLS-SDL3
gh secret set ANDROID_KEY_ALIAS --repo nL6DHZc3RhMnC/EntisGLS-SDL3
gh secret set ANDROID_STORE_PASSWORD --repo nL6DHZc3RhMnC/EntisGLS-SDL3
```

The last two commands prompt for values. Passwords are passed to `apksigner`
through environment references, not command arguments. The key is decoded with
owner-only permissions under the runner's temporary directory and removed even
if compilation fails. CI refuses to generate a replacement temporary key.

## Local equivalents

```sh
python3 tools/ci/ci_prepare_dependencies.py
python3 -m unittest discover -s tests/unit/python -p 'test_*.py' -v
python3 tools/build/build_sdl_desktop.py --arch native
python3 tools/ci/ci_native_checks.py --build-dir build/macos-sdl3-<arch>
```

Use Bison 3.8.2 or newer (`brew install bison` on macOS; add its `bin` to `PATH`
or set `TJS_BISON`). Android builds accept `ANDROID_HOME`, `ANDROID_NDK_HOME`
and `JAVA_HOME`, or `--sdk-root`, `--ndk`, `--java-home`. Required versions are
SDK API 35, build-tools 35.0.0, NDK 27.2.12479018 and JDK 17.
