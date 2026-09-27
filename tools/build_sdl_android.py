#!/usr/bin/env python3
"""Configure, compile and package the SDL3 ARM64 Android development build.

Uses the project-local NDK r27c and build-host.json CMake override. The launcher
does not need a game directory for compilation. Previous APKs are preserved.
"""

import argparse
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys

from entis_sdk import validate as validate_sdk

ROOT = Path(__file__).resolve().parents[1]
HOST_CONFIG = ROOT / ".android-tools/build-host.json"
NDK = ROOT / ".android-tools/ndk/android-ndk-r27c"
GENERATOR = "Unix Makefiles"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=6, help="Parallel compilation jobs (1–8, default 6)")
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build/android-sdl3")
    parser.add_argument("--output", type=Path, default=ROOT / "artifacts/entisgls-launcher-arm64-dev.apk")
    parser.add_argument("--without-bundled-fonts", action="store_true", help="Package without optional launcher fonts; games must provide their own fonts")
    parser.add_argument("--no-package", action="store_true", help="Configure/compile native libraries without replacing the SDL APK")
    parser.add_argument("--dry-run", action="store_true", help="Validate local tools and print commands without building or packaging")
    args = parser.parse_args()
    if not 1 <= args.jobs <= 8:
        parser.error("--jobs must be between 1 and 8")
    validate_sdk()
    if not (NDK / "source.properties").is_file():
        raise SystemExit("Install the project-local Android tools first: python3 tools/setup_android.py")
    properties = dict(line.split("=", 1) for line in (NDK / "source.properties").read_text().splitlines() if "=" in line)
    properties = {key.strip(): value.strip() for key, value in properties.items()}
    if properties.get("Pkg.Revision") != "27.2.12479018":
        raise SystemExit(f"Expected NDK r27c (27.2.12479018), found {properties.get('Pkg.Revision')}")
    if not (ROOT / "vendor/sdl3/CMakeLists.txt").is_file():
        raise SystemExit("Prepare the pinned SDL source first: python3 tools/setup_sdl3.py")
    host = json.loads(HOST_CONFIG.read_text()) if HOST_CONFIG.is_file() else {}
    cmake = os.environ.get("ENTISGLS_CMAKE", os.environ.get("STUDYSTEADY_CMAKE", host.get("cmake", "cmake")))
    if not shutil.which(cmake):
        raise SystemExit(f"CMake not found: {cmake}; set ENTISGLS_CMAKE or {HOST_CONFIG}")
    make = shutil.which("make")
    if not make:
        raise SystemExit("Unix Makefiles requires make on the build host")

    directory = args.build_dir.resolve()
    forbidden = [ROOT / name for name in ("EntisGLS", "EntisGLS4.07.03", "StudySteadyR18", "vendor")]
    if directory == ROOT or any(path == directory or path in directory.parents for path in forbidden):
        raise SystemExit("Use an out-of-source build directory outside the game and vendor/SDK inputs")
    fresh = []
    cache = directory / "CMakeCache.txt"
    if cache.exists() and f"CMAKE_GENERATOR:INTERNAL={GENERATOR}\n" not in cache.read_text():
        fresh = ["--fresh"]
    configure = [cmake, *fresh, "-S", ROOT, "-B", directory, "-G", GENERATOR,
        "-DCMAKE_MAKE_PROGRAM=" + make,
        "-DCMAKE_TOOLCHAIN_FILE=" + str(NDK / "build/cmake/android.toolchain.cmake"),
        "-DANDROID_ABI=arm64-v8a", "-DANDROID_PLATFORM=android-29", "-DANDROID_STL=c++_shared",
        "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON", "-DCMAKE_BUILD_TYPE=Release",
        "-DENTISGLS_LAUNCHER=ON", "-DSTUDYSTEADY_SDL3_SHARED=ON",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384"]
    compile_command = [cmake, "--build", directory, "--target", "studysteady_sdl", "--parallel", str(args.jobs)]
    package = [sys.executable, ROOT / "tools/build_sdl_apk.py", "--native-build", directory, "--output", args.output.resolve()]
    if args.without_bundled_fonts:
        package.append("--without-bundled-fonts")
    stages = [("configure", configure), ("native-build", compile_command)]
    if not args.no_package:
        stages.append(("package", package))
    if args.dry_run:
        for name, command in stages:
            print(f"{name}: {shlex.join(str(item) for item in command)}")
        print("Dry run: no configuration, compilation or APK changes performed.")
        return

    logs = directory / "logs"
    logs.mkdir(parents=True, exist_ok=True)
    for name, command in stages:
        log_path = logs / (name + ".log")
        print(f"SDL Android {name}; log: {log_path}", flush=True)
        with log_path.open("w") as log:
            result = subprocess.run([str(item) for item in command], cwd=ROOT,
                stdout=log, stderr=subprocess.STDOUT)
        if result.returncode:
            print("\n".join(log_path.read_text(errors="replace").splitlines()[-40:]), file=sys.stderr)
            raise SystemExit(f"{name} failed ({result.returncode}); see {log_path}")
    print(directory / "libmain.so" if args.no_package else args.output.resolve())
    print("Build completed. This command does not install the APK or verify a device run.")


if __name__ == "__main__":
    main()
