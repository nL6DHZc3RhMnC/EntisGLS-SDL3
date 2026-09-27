#!/usr/bin/env python3
"""Build the SDL iOS app with Xcode and package an unsigned device IPA.

Uses Apple's installed iPhoneOS SDK, static SDL3, and no signing credentials.
An optional ARM64 Simulator build produces an app ZIP instead of a device IPA.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import plistlib
import re
import shlex
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile

APP_NAME = "EntisGLSLauncher.app"


def run(*command, check=True):
    return subprocess.run([str(value) for value in command], cwd=ROOT,
                          text=True, capture_output=True, check=check)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def writable_output(path):
    path = path.resolve()
    protected = [ROOT / name for name in ("EntisGLS", "EntisGLS4.07.03", "StudySteadyR18", "vendor", "assets", "native", "tools", "cmake", "apps", "tests", "docs")]
    if path == ROOT or ROOT.is_relative_to(path) or any(path == item or item in path.parents for item in protected):
        raise ValueError("Use a build/output directory outside the source, game, and vendor/SDK inputs")
    return path


def build_commands(cmake, directory, sdk, deployment_target, bundle_id, jobs):
    configure = [cmake, "-S", ROOT, "-B", directory, "-G", "Xcode",
                 "-DENTISGLS_LAUNCHER=ON", "-DSTUDYSTEADY_SDL3_SHARED=OFF",
                 "-DCMAKE_SYSTEM_NAME=iOS", "-DCMAKE_OSX_SYSROOT=" + sdk,
                 "-DCMAKE_OSX_ARCHITECTURES=arm64",
                 "-DCMAKE_OSX_DEPLOYMENT_TARGET=" + deployment_target,
                 "-DENTISGLS_IOS_BUNDLE_IDENTIFIER=" + bundle_id,
                 "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO",
                 "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO",
                 "-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY=",
                 "-DCMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE=NO"]
    build = [cmake, "--build", directory, "--config", "Release", "--target", "studysteady_sdl",
             "--parallel", str(jobs), "--", "CODE_SIGNING_ALLOWED=NO", "CODE_SIGNING_REQUIRED=NO",
             "CODE_SIGN_IDENTITY=", "-destination", "generic/platform=" +
             ("iOS Simulator" if sdk == "iphonesimulator" else "iOS")]
    return [("configure", configure), ("native-build", build)]


def write_archive(bundle, archive, device):
    prefix = "Payload/" if device else ""
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as output:
        for path in [bundle, *sorted(bundle.rglob("*"))]:
            name = prefix + path.relative_to(bundle.parent).as_posix()
            if path.is_symlink():
                entry = zipfile.ZipInfo(name)
                entry.create_system = 3
                entry.external_attr = (stat.S_IFLNK | 0o777) << 16
                output.writestr(entry, os.readlink(path))
            else:
                output.write(path, name)
    with zipfile.ZipFile(archive) as package:
        bad = package.testzip()
        if bad:
            raise RuntimeError(f"Archive CRC verification failed: {bad}")
        executable = prefix + APP_NAME + "/EntisGLSLauncher"
        info = package.getinfo(executable)
        if not (info.external_attr >> 16) & 0o111:
            raise RuntimeError("Packaged app executable lost its executable permission")
        if hashlib.sha256(package.read(executable)).hexdigest() != digest(bundle / "EntisGLSLauncher"):
            raise RuntimeError("Archived executable differs from the verified app")
        if device and any("_CodeSignature" in item.split("/") or item.endswith(".mobileprovision") for item in package.namelist()):
            raise RuntimeError("Unsigned archive contains a signature or provisioning profile")


def package_app(source, destination, sdk, deployment_target, bundle_id, toolchain):
    device = sdk == "iphoneos"
    expected_platform = "iPhoneOS" if device else "iPhoneSimulator"
    destination.mkdir(parents=True, exist_ok=True)
    archive = destination / ("EntisGLSLauncher-unsigned.ipa" if device else "EntisGLSLauncher-simulator.zip")
    with tempfile.TemporaryDirectory(prefix="entisgls-ios-") as temporary:
        stage = Path(temporary) / APP_NAME
        shutil.copytree(source, stage, symlinks=True)
        binary = stage / "EntisGLSLauncher"
        info = plistlib.loads((stage / "Info.plist").read_bytes())
        if info.get("CFBundleIdentifier") != bundle_id or info.get("CFBundleExecutable") != binary.name:
            raise RuntimeError("Built app has an unexpected bundle identifier or executable")
        if info.get("CFBundleSupportedPlatforms") != [expected_platform]:
            raise RuntimeError(f"Built app is not a {expected_platform} bundle")
        if info.get("MinimumOSVersion") != deployment_target:
            raise RuntimeError("Built app deployment target differs from the requested target")
        architectures = run("/usr/bin/lipo", "-archs", binary).stdout.split()
        if architectures != ["arm64"]:
            raise RuntimeError(f"Expected ARM64, found {architectures}")
        load_commands = run("xcrun", "--sdk", sdk, "vtool", "-show-build", binary).stdout
        expected_macho_platform = "IOSSIMULATOR" if not device else "IOS"
        if not re.search(r"platform\s+" + expected_macho_platform + r"\b", load_commands):
            raise RuntimeError("Executable Mach-O platform does not match the selected SDK")
        if device:
            # Some Apple linkers emit an ad-hoc signature with Xcode signing
            # disabled. Remove it only from the generated packaging copy.
            if run("/usr/bin/codesign", "--display", stage, check=False).returncode == 0:
                run("/usr/bin/codesign", "--remove-signature", stage)
            signature = run("/usr/bin/codesign", "--display", stage, check=False)
            if signature.returncode == 0 or "not signed at all" not in signature.stderr:
                raise RuntimeError("Could not verify that the app is unsigned: " + signature.stderr)
        else:
            # ARM64 Simulator executables use local ad-hoc signatures. This
            # requires neither an Apple account nor a provisioning profile.
            run("/usr/bin/codesign", "--force", "--sign", "-", stage)
            run("/usr/bin/codesign", "--verify", "--strict", stage)
        if not (stage / "LaunchScreen.storyboardc").exists():
            raise RuntimeError("Compiled iOS launch screen is missing")
        font_metadata = ROOT / "assets/fonts/provenance.json"
        font = json.loads(font_metadata.read_text()) if font_metadata.is_file() else None
        if font and (stage / font["file"]).is_file():
            if digest(stage / font["file"]) != font["sha256"] or not (stage / font["license_file"]).is_file():
                raise RuntimeError("Optional compatibility font or its license failed verification")
        else:
            font = None
        write_archive(stage, archive, device)
        bundle = destination / APP_NAME
        if bundle.is_symlink():
            bundle.unlink()
        elif bundle.exists():
            shutil.rmtree(bundle)
        shutil.copytree(stage, bundle, symlinks=True)
        report = {
            "platform": "iOS" if device else "iOS Simulator", "sdk": sdk,
            "architectures": architectures, "minimum_os": deployment_target,
            "bundle_id": bundle_id, "bundle": str(bundle), "archive": str(archive),
            "unsigned": device, "signature": "unsigned" if device else "local ad-hoc",
            "requires_signing_for_device": device,
            "binary_sha256": digest(binary), "archive_sha256": digest(archive),
            "archive_crc_verified": True, "archive_binary_verified": True,
            "macho_platform_verified": True, "runtime_verified_by_this_command": False,
            "game_resources_included": False, "toolchain": toolchain,
            "bundled_font": {"family": font["family"], "file": font["file"], "sha256": font["sha256"]} if font else None,
        }
        (destination / "build.json").write_text(json.dumps(report, indent=2) + "\n")
    print(archive)
    print(destination / APP_NAME)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", choices=("iphoneos", "iphonesimulator"), default="iphoneos")
    parser.add_argument("--deployment-target", default="13.0")
    parser.add_argument("--bundle-id", default="io.entisgls.launcher")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--dry-run", action="store_true", help="Print commands without configuring, building, or packaging")
    args = parser.parse_args()
    if not 1 <= args.jobs <= 16:
        parser.error("--jobs must be between 1 and 16")
    if not re.fullmatch(r"\d+\.\d+(?:\.\d+)?", args.deployment_target) or int(args.deployment_target.split(".")[0]) < 13:
        parser.error("--deployment-target must be an iOS version of 13.0 or newer")
    if not re.fullmatch(r"[A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)+", args.bundle_id):
        parser.error("--bundle-id must be a reverse-DNS app identifier")
    suffix = "ios-arm64" if args.sdk == "iphoneos" else "ios-simulator-arm64"
    try:
        directory = writable_output(args.build_dir or ROOT / "build" / suffix)
        destination = writable_output(args.output_dir or ROOT / "artifacts/entisgls-launcher" / suffix)
    except ValueError as error:
        parser.error(str(error))
    cmake = os.environ.get("ENTISGLS_CMAKE", os.environ.get("STUDYSTEADY_CMAKE", "cmake"))
    stages = build_commands(cmake, directory, args.sdk, args.deployment_target, args.bundle_id, args.jobs)
    if args.dry_run:
        for name, command in stages:
            print(name + ": " + shlex.join(str(item) for item in command))
        print("package: " + str(destination))
        return
    if platform.system() != "Darwin":
        raise SystemExit("iOS builds require macOS with full Xcode and the iOS SDK installed.")
    if not shutil.which(cmake):
        raise SystemExit(f"CMake not found: {cmake}; set ENTISGLS_CMAKE")
    xcode = run("xcodebuild", "-version", check=False)
    sdk_path = run("xcrun", "--sdk", args.sdk, "--show-sdk-path", check=False)
    if xcode.returncode or sdk_path.returncode:
        raise SystemExit("Full Xcode with the selected iOS SDK is required; Command Line Tools alone cannot build iOS. Select Xcode with DEVELOPER_DIR or xcode-select.")
    toolchain = {"xcode": xcode.stdout.strip(), "sdk_path": sdk_path.stdout.strip(),
                 "sdk_version": run("xcrun", "--sdk", args.sdk, "--show-sdk-version").stdout.strip()}
    cache = directory / "CMakeCache.txt"
    if cache.exists() and "CMAKE_GENERATOR:INTERNAL=Xcode\n" not in cache.read_text():
        raise SystemExit("The build directory uses another CMake generator; choose a new --build-dir.")
    logs = directory / "logs"
    logs.mkdir(parents=True, exist_ok=True)
    for name, command in stages:
        log_path = logs / (name + ".log")
        print(f"SDL iOS {name}; log: {log_path}", flush=True)
        with log_path.open("w") as log:
            result = subprocess.run([str(item) for item in command], cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
        if result.returncode:
            print("\n".join(log_path.read_text(errors="replace").splitlines()[-60:]), file=sys.stderr)
            raise SystemExit(f"{name} failed ({result.returncode}); see {log_path}")
    source = directory / ("Release-" + args.sdk) / APP_NAME
    package_app(source, destination, args.sdk, args.deployment_target, args.bundle_id, toolchain)
    print("Build complete. " + ("Device installation requires local Apple development signing and provisioning."
                               if args.sdk == "iphoneos" else "The Simulator app is signed locally with an ad-hoc signature."))


if __name__ == "__main__":
    main()
