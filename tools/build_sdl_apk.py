#!/usr/bin/env python3
"""Compile the SDL3 Android Java shell and package already-built ARM64 libraries.

Default input: build/android-sdl3/{libmain.so,sdl3/libSDL3.so}.
This command does not configure or compile native code. For an end-to-end build,
run tools/build_sdl_android.py instead.
The launcher uses its own package ID. Release builds use an external persistent
keystore; local development defaults to the existing project-local key.
Use --check-shell to compile Java/DEX/manifest without claiming a runnable APK.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

from entis_sdk import COTOPHA, LEGACY, validate as validate_sdk
from android_app import PACKAGE_ID
from android_toolchain import (add_toolchain_arguments, resolve_java, resolve_sdk_tools,
                               resolve_signing, sdk_root, validate_ndk)

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/apk-sdl3"
SHELL = ROOT / "android/sdl"
SDL = ROOT / "vendor/sdl3"


def only_path(pattern, root, description):
    candidates = sorted(root.glob(pattern))
    if len(candidates) != 1:
        raise RuntimeError(f"Expected one {description} under {root}, found {candidates}")
    return candidates[0]


def digest(path):
    result = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def cache_values(path):
    result = {}
    for line in path.read_text().splitlines():
        if line.startswith(("#", "//")) or "=" not in line:
            continue
        key, value = line.split("=", 1)
        result[key.split(":", 1)[0]] = value
    return result


def inspect_elf(path, readelf, environment):
    with path.open("rb") as stream:
        header = stream.read(64)
        if len(header) != 64 or header[:6] != b"\x7fELF\x02\x01":
            raise RuntimeError(f"Not a 64-bit little-endian ELF library: {path}")
        if struct.unpack_from("<H", header, 18)[0] != 183:
            raise RuntimeError(f"Expected AArch64 native library: {path}")
        if struct.unpack_from("<H", header, 16)[0] != 3:
            raise RuntimeError(f"Expected a shared ELF object: {path}")
        phoff = struct.unpack_from("<Q", header, 32)[0]
        phsize, phcount = struct.unpack_from("<HH", header, 54)
        alignment = []
        for index in range(phcount):
            stream.seek(phoff + index * phsize)
            segment = stream.read(phsize)
            if struct.unpack_from("<I", segment)[0] == 1:
                alignment.append(struct.unpack_from("<Q", segment, 48)[0])
    dynamic = subprocess.run([str(readelf), "--dynamic", str(path)], check=True,
        capture_output=True, text=True, env=environment).stdout
    needed = [line.split("[", 1)[1].split("]", 1)[0]
              for line in dynamic.splitlines() if "(NEEDED)" in line]
    return {"file": path.name, "sha256": digest(path), "bytes": path.stat().st_size,
            "min_load_alignment": min(alignment), "needed": needed}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native-build", type=Path, default=ROOT / "build/android-sdl3")
    parser.add_argument("--output", type=Path, default=ROOT / "artifacts/entisgls-launcher-arm64-dev.apk")
    parser.add_argument("--check-shell", action="store_true", help="Compile only Java, DEX and the manifest; do not publish a game APK")
    parser.add_argument("--without-bundled-fonts", action="store_true", help="Omit optional launcher fonts; games must provide their own fonts")
    add_toolchain_arguments(parser)
    args = parser.parse_args()
    signing = None if args.check_shell else resolve_signing()
    preserved = (ROOT / "artifacts/studysteady-arm64-dev.apk", ROOT / "artifacts/studysteady-sdl3-arm64-dev.apk")
    if args.output.resolve() in {path.resolve() for path in preserved}:
        raise RuntimeError("Refusing to overwrite a previous StudySteady APK; choose a launcher artifact path")
    validate_sdk()
    subprocess.run([sys.executable, str(ROOT / "tools/setup_sdl_fonts.py"), "--verify", "--freetype-only"], check=True)
    manifest = ET.parse(SHELL / "AndroidManifest.xml").getroot()
    android = "{http://schemas.android.com/apk/res/android}"
    application = manifest.find("application")
    if manifest.attrib["package"] != PACKAGE_ID:
        raise RuntimeError(f"The Android launcher manifest must use package ID {PACKAGE_ID}")
    java_source = SDL / "android-project/app/src/main/java"
    if not (java_source / "org/libsdl/app/SDLActivity.java").is_file():
        raise RuntimeError("Pinned SDL3 source is missing; run tools/setup_sdl3.py first")
    provenance_path = ROOT / "vendor/sdl3-provenance.json"
    provenance = json.loads(provenance_path.read_text())
    if provenance["version"] != "3.4.16" or provenance["commit"] != "fa2c02bb6e21974a89ea9824bc53c9932abe5f9c":
        raise RuntimeError("Review the SDL Java/native version pin before changing the dependency")
    jdk = resolve_java(args.java_home)
    sdk, platform = resolve_sdk_tools(sdk_root(args.sdk_root))
    environment = dict(os.environ, JAVA_HOME=str(jdk))
    environment["PATH"] = str(jdk / "bin") + os.pathsep + environment.get("PATH", "")

    def run(arguments, **kwargs):
        return subprocess.run([str(item) for item in arguments], check=True, env=environment, **kwargs)

    native_files = []
    native_info = []
    if not args.check_shell:
        cache_file = args.native_build / "CMakeCache.txt"
        if not cache_file.is_file():
            raise RuntimeError(f"Configure and build the SDL Android target first: {cache_file}")
        cache = cache_values(cache_file)
        if cache.get("ENTISGLS_LAUNCHER") != "ON" and cache.get("STUDYSTEADY_SDL3") != "ON":
            raise RuntimeError("The native build must enable ENTISGLS_LAUNCHER (or its legacy STUDYSTEADY_SDL3 alias)")
        for key, expected in (("ENTIS_ROOT", COTOPHA), ("LEGACY_ROOT", LEGACY)):
            if not cache.get(key) or Path(cache[key]).resolve() != expected.resolve():
                raise RuntimeError(f"Native build {key} must point to the supplied official SDK: {expected}")
        if cache.get("ANDROID_ABI") != "arm64-v8a" or cache.get("SDL_SHARED") != "ON":
            raise RuntimeError("Expected an arm64-v8a build with shared SDL3")
        if cache.get("ANDROID_PLATFORM") not in ("29", "android-29"):
            raise RuntimeError("This APK declares minSdk 29 and requires an Android API 29 native build")
        if not cache.get("SDL3_SOURCE_DIR") or Path(cache["SDL3_SOURCE_DIR"]).resolve() != SDL.resolve():
            raise RuntimeError("Native SDL must come from the same pinned vendor/sdl3 tree as its Java bootstrap")
        ndk_value = cache.get("CMAKE_ANDROID_NDK") or cache.get("ANDROID_NDK")
        if not ndk_value and cache.get("CMAKE_TOOLCHAIN_FILE"):
            toolchain = Path(cache["CMAKE_TOOLCHAIN_FILE"]).resolve()
            if toolchain.name == "android.toolchain.cmake" and toolchain.parent.name == "cmake" and toolchain.parent.parent.name == "build":
                ndk_value = str(toolchain.parents[2])
        if not ndk_value:
            raise RuntimeError("Native build does not identify its Android NDK")
        ndk = validate_ndk(ndk_value)
        prebuilt = only_path("*", ndk / "toolchains/llvm/prebuilt", "NDK host toolchain")
        readelf = prebuilt / "bin/llvm-readelf"
        main_library = args.native_build / "libmain.so"
        sdl_library = args.native_build / "sdl3/libSDL3.so"
        if not sdl_library.is_file():
            sdl_library = args.native_build / "libSDL3.so"
        cpp_library = prebuilt / "sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
        native_files = [main_library, sdl_library, cpp_library]
        for path in native_files:
            if not path.is_file():
                raise RuntimeError(f"Required native library has not been built: {path}")
            native_info.append(inspect_elf(path, readelf, environment))
        symbols = run([readelf, "--dyn-syms", main_library], capture_output=True, text=True).stdout
        if not any(line.split()[-1:] == ["SDL_main"] and " UND " not in line for line in symbols.splitlines()):
            raise RuntimeError("libmain.so does not export SDL_main for the SDL Activity bootstrap")
        system_libraries = {"libc.so", "libm.so", "libdl.so", "liblog.so", "libandroid.so", "libz.so",
                            "libEGL.so", "libGLESv1_CM.so", "libGLESv2.so", "libGLESv3.so",
                            "libOpenSLES.so", "libvulkan.so", "libjnigraphics.so", "libmediandk.so", "libaaudio.so"}
        available = system_libraries | {path.name for path in native_files}
        missing = {dependency for item in native_info for dependency in item["needed"]} - available
        if missing:
            raise RuntimeError(f"Native dependencies are missing from the APK: {sorted(missing)}")
        if "libSDL3.so" not in native_info[0]["needed"]:
            raise RuntimeError("libmain.so must use the same shared SDL3 instance loaded by Java")
        if any(item["min_load_alignment"] < 16384 for item in native_info):
            raise RuntimeError("All packaged native libraries must support 16 KB page alignment")

    BUILD.mkdir(parents=True, exist_ok=True)
    for name in ("classes", "dex", "assets"):
        destination = BUILD / name
        if destination.exists():
            shutil.rmtree(destination)
        destination.mkdir()
    java_files = sorted(java_source.rglob("*.java")) + sorted((SHELL / "java").rglob("*.java"))
    # These are Android's official compile-time lambda stubs, not packaged
    # runtime classes. D8 performs the same desugaring as the Gradle toolchain.
    lambda_stubs = sdk / "core-lambda-stubs.jar"
    if not lambda_stubs.is_file():
        raise RuntimeError(f"Android Java lambda stubs are missing: {lambda_stubs}")
    boot_classpath = str(lambda_stubs) + os.pathsep + str(platform)
    run([jdk / "bin/javac", "-encoding", "UTF-8", "-source", "8", "-target", "8",
         "-bootclasspath", boot_classpath, "-d", BUILD / "classes", *java_files])
    run([jdk / "bin/jar", "cf", BUILD / "classes.jar", "-C", BUILD / "classes", "."])
    run([sdk / "d8", "--lib", platform, "--min-api", "29", "--output", BUILD / "dex", BUILD / "classes.jar"])
    # Profiles are explicitly namespaced; no game's default configuration,
    # archives, execution images or EXE is part of the generic launcher.
    asset_directories = ["compatibility", "licenses"]
    if not args.without_bundled_fonts:
        asset_directories.append("fonts")
    for name in asset_directories:
        source = ROOT / "assets" / name
        if source.is_dir():
            shutil.copytree(source, BUILD / "assets" / name)
    license_directory = BUILD / "assets/licenses"
    license_directory.mkdir(exist_ok=True)
    shutil.copy2(SDL / "LICENSE.txt", license_directory / "SDL3.txt")
    shutil.copy2(provenance_path, license_directory / "SDL3-provenance.json")
    unsigned = BUILD / "unsigned.apk"
    unsigned.unlink(missing_ok=True)
    run([sdk / "aapt2", "link", "-I", platform, "--manifest", SHELL / "AndroidManifest.xml",
         "-A", BUILD / "assets", "-o", unsigned])
    with zipfile.ZipFile(unsigned, "a", compression=zipfile.ZIP_DEFLATED) as archive:
        for dex in sorted((BUILD / "dex").glob("*.dex")):
            archive.write(dex, dex.name)
        for native in native_files:
            archive.write(native, "lib/arm64-v8a/" + native.name)
    bundled_fonts = []
    with zipfile.ZipFile(unsigned) as archive:
        entries = archive.namelist()
        if "assets/cotopha.xml" in entries or any(Path(name).suffix.lower() in (".noa", ".csx", ".exe") for name in entries):
            raise RuntimeError("A game-specific default configuration or game resource entered the generic APK")
        for name in entries:
            if name.startswith("assets/fonts/") and Path(name).suffix.lower() in (".otf", ".ttf", ".ttc"):
                with archive.open(name) as stream:
                    font_digest = hashlib.sha256()
                    for block in iter(lambda: stream.read(1024 * 1024), b""):
                        font_digest.update(block)
                bundled_fonts.append({"file": name, "sha256": font_digest.hexdigest(),
                                      "bytes": archive.getinfo(name).file_size, "verified_in_package": True})
        if "assets/fonts/provenance.json" in entries:
            font = json.loads(archive.read("assets/fonts/provenance.json"))
            matching = next((item for item in bundled_fonts if item["file"] == font["file"]), None)
            if matching is None or matching["sha256"] != font["sha256"] or matching["bytes"] != font["size_bytes"]:
                raise RuntimeError("Packaged optional font differs from its provenance record")
            if font["license_file"] not in entries:
                raise RuntimeError("Packaged optional font license is missing")
            matching.update({"family": font["family"], "license_file": font["license_file"]})
    report = {"package": manifest.attrib["package"], "version_code": int(manifest.attrib[android + "versionCode"]),
              "version_name": manifest.attrib[android + "versionName"], "application_label": application.attrib[android + "label"],
              "backend": "SDL3", "launcher": "EntisGLS Launcher",
              "min_sdk": 29, "target_sdk": 35, "sdl_version": provenance["version"],
              "sdl_commit": provenance["commit"], "java_sources": len(java_files),
              "shell_compiled": True, "native_packaged": not args.check_shell,
              "native_libraries": native_info, "device_tested": False,
              "game_resources_bundled": False, "bundled_fonts": bundled_fonts,
              "compatibility_assets": sorted(name for name in entries if name.startswith("assets/compatibility/"))}
    if args.check_shell:
        (BUILD / "shell-validation.json").write_text(json.dumps(report, indent=2) + "\n")
        print("SDL Android Java, DEX, assets and manifest compiled. Shell check only; no game APK published.")
        return
    run([sdk / "zipalign", "-P", "16", "-f", "4", unsigned, BUILD / "aligned.apk"])
    key = signing.keystore
    if signing.create_local_key:
        environment[signing.store_password_env] = "android"
    if not key.exists():
        if not signing.create_local_key:
            raise RuntimeError("The configured external signing keystore is no longer available")
        key.parent.mkdir(parents=True, exist_ok=True)
        run([jdk / "bin/keytool", "-genkeypair", "-keystore", key, "-storepass:env", signing.store_password_env,
             "-keypass:env", signing.key_password_env, "-alias", signing.alias, "-dname", "CN=EntisGLS Launcher Local Development",
             "-keyalg", "RSA", "-keysize", "2048", "-validity", "3650"])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    run([sdk / "apksigner", "sign", "--ks", key, "--ks-key-alias", signing.alias,
         "--ks-pass", "env:" + signing.store_password_env,
         "--key-pass", "env:" + signing.key_password_env, "--out", args.output, BUILD / "aligned.apk"])
    run([sdk / "apksigner", "verify", "--verbose", args.output])
    run([sdk / "zipalign", "-c", "-P", "16", "4", args.output])
    report.update({"apk": str(args.output.resolve()), "apk_sha256": digest(args.output), "apk_bytes": args.output.stat().st_size,
                   "elf_16k_compatible": all(item["min_load_alignment"] >= 16384 for item in native_info)})
    args.output.with_suffix(".build.json").write_text(json.dumps(report, indent=2) + "\n")
    print(args.output)


if __name__ == "__main__":
    main()
