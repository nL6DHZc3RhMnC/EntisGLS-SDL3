#!/usr/bin/env python3
"""Rebuild the native Android development APK (macOS Intel build host)."""
from pathlib import Path
import json
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
HOST_CONFIG = ROOT / '.android-tools/build-host.json'
HOST = json.loads(HOST_CONFIG.read_text()) if HOST_CONFIG.exists() else {}
CMAKE = os.environ.get('STUDYSTEADY_CMAKE', HOST.get('cmake', 'cmake'))
GENERATOR = 'Unix Makefiles'

def run(*args):
    subprocess.run([str(a) for a in args], cwd=ROOT, check=True)

def configure(directory, *options):
    cache = ROOT / directory / 'CMakeCache.txt'
    fresh = []
    if cache.exists() and f'CMAKE_GENERATOR:INTERNAL={GENERATOR}\n' not in cache.read_text():
        fresh = ['--fresh']
    run(CMAKE, *fresh, '-S', '.', '-B', directory, '-G', GENERATOR,
        '-DCMAKE_MAKE_PROGRAM=/usr/bin/make', *options)

def main():
    ndk = ROOT / '.android-tools/ndk/android-ndk-r27c'
    if not ndk.exists():
        raise SystemExit('First run: python3 tools/setup_android.py')
    configure('build/host')
    run(CMAKE, '--build', 'build/host', '--parallel', '6')
    run(sys.executable, 'tools/prepare_game.py')
    run(sys.executable, 'tools/prepare_legacy.py')
    configure('build/android-arm64',
        '-DCMAKE_TOOLCHAIN_FILE=' + str(ndk / 'build/cmake/android.toolchain.cmake'),
        '-DANDROID_ABI=arm64-v8a', '-DANDROID_PLATFORM=android-29',
        '-DLEGACY_COMPILE_OBJECTS=ON', '-DENTISGLS_LAUNCHER=OFF', '-DSTUDYSTEADY_SDL3=OFF',
        '-DANDROID_STL=c++_shared', '-DCMAKE_BUILD_TYPE=Release')
    run(CMAKE, '--build', 'build/android-arm64', '--parallel', '6')
    run(sys.executable, 'tools/build_apk.py')

if __name__ == '__main__':
    main()
