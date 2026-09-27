# GitHub Actions builds and releases

`.github/workflows/build-release.yml` builds Android ARM64 on Ubuntu, macOS Intel
on `macos-15-intel`, and macOS Apple Silicon on `macos-15`. Each architecture has
its own checkout. No game directory or local `.android-tools` directory is used.
Pinned inputs and clean-checkout preparation are described in [CI build inputs](ci-build-inputs.md).

## Triggers and artifacts

- Push to `main`, or run **Build and release** manually: publish a development
  prerelease named `dev-<run>.<attempt>-<commit>`.
- Push a `v*` tag: publish that version. Tags containing a hyphen remain
  prereleases (for example `v0.4.0-rc1`). Update the app manifests/version first.
- Publication happens only after all three packages and their checks pass.
  A failed build leaves diagnostics in Actions and does not publish a release.

Releases contain the signed APK, two app ZIPs, build/native-test reports and
`SHA256SUMS.txt`. macOS signing is ad-hoc, with archive extraction and signature
verification; this workflow does not perform Developer ID signing or notarization.
The native tests run the actual Cotopha interpreter with generated fixtures and
SDL dummy drivers. They do not require commercial game resources or a display.
Android packaging verifies the APK signature, ZIP alignment and ELF 16 KB alignment.
Device gameplay still needs separate testing.

Build jobs have read-only repository access. Only the final release job receives
`contents: write`; no personal access token is needed by the workflow. Releases
are uploaded as drafts, then published after every asset upload succeeds.
Existing releases are never overwritten. If a version-tag run leaves a draft
after an upload failure, review/remove that draft before rerunning it.

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
python3 tools/ci_prepare_dependencies.py
python3 -m unittest discover -s tools -p 'test_*.py' -v
python3 tools/build_sdl_desktop.py --arch native
python3 tools/ci_native_checks.py --build-dir build/macos-sdl3-<arch>
```

Use Bison 3.8.2 or newer (`brew install bison` on macOS; add its `bin` to `PATH`
or set `TJS_BISON`). Android builds accept `ANDROID_HOME`, `ANDROID_NDK_HOME`
and `JAVA_HOME`, or `--sdk-root`, `--ndk`, `--java-home`. Required versions are
SDK API 35, build-tools 35.0.0, NDK 27.2.12479018 and JDK 17.
