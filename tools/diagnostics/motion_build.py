#!/usr/bin/env python3
"""Reproducibly build the independent native motion probes (not the APK)."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
from pathlib import Path
import shutil
import subprocess
import sys

BUNDLED_CMAKE = Path('/Users/fenghengzhi/Developer/toolchains/krkr2/cmake-pkg/cmake/data/bin/cmake')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--platform', choices=['host','android','both'], default='both')
    parser.add_argument('--cmake', default=str(BUNDLED_CMAKE) if BUNDLED_CMAKE.exists() else shutil.which('cmake'))
    parser.add_argument('--ndk', type=Path, default=ROOT / '.android-tools/ndk/android-ndk-r27c')
    parser.add_argument('--generator', default='Unix Makefiles')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--sanitizers', action='store_true', help='Host-only address/undefined behavior validation')
    args = parser.parse_args()
    if not args.cmake: parser.error('Specify --cmake or install CMake')
    if args.jobs < 1: parser.error('--jobs must be positive')
    if args.sanitizers and args.platform != 'host': parser.error('--sanitizers requires --platform host')
    subprocess.run([sys.executable, str(ROOT / 'tools/ci/motion_verify_imports.py')], cwd=ROOT, check=True)
    platforms = ['host','android'] if args.platform == 'both' else [args.platform]
    for platform in platforms:
        for project, prefix, targets in [
            ('native/extensions/emote','motion-bridge',['motion_inspect','motion_texture_probe','motion_pose_probe']),
            ('native/extensions/emote/tjs_runtime','motion-tjs',['motion_tjs_probe','motion_ncb_probe','motion_psb_tjs_probe','motion_player_probe','motion_mesh_probe','motion_timeline_probe','motion_runtime_probe','motion_scene_probe']),
        ]:
            suffix = 'arm64' if platform == 'android' else 'host'
            if args.sanitizers: suffix += '-asan'
            build = ROOT / 'build' / (prefix + '-' + suffix)
            configure = [args.cmake, '-S', str(ROOT / project), '-B', str(build), '-G', args.generator,
                         '-DCMAKE_BUILD_TYPE=Release', '-DENTISGLS_BUILD_DIAGNOSTICS=ON', '-Wno-deprecated']
            if platform == 'android':
                toolchain = args.ndk.resolve() / 'build/cmake/android.toolchain.cmake'
                if not toolchain.is_file(): parser.error(f'Android NDK toolchain missing: {toolchain}')
                configure += [f'-DCMAKE_TOOLCHAIN_FILE={toolchain}', '-DANDROID_ABI=arm64-v8a',
                              '-DANDROID_PLATFORM=android-29', '-DANDROID_STL=c++_static']
                if prefix == 'motion-bridge': targets += ['motion_gles_probe']
                if prefix == 'motion-tjs': targets += ['motion_scene_gles_probe','motion_runtime_gles_probe']
            if args.sanitizers:
                flags = '-fsanitize=address,undefined -fno-omit-frame-pointer'
                configure += [f'-DCMAKE_C_FLAGS={flags}',f'-DCMAKE_CXX_FLAGS={flags}',
                              '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined']
            print(f'Building {project} for {platform}', flush=True)
            subprocess.run(configure, cwd=ROOT, check=True)
            subprocess.run([args.cmake,'--build',str(build),'--target',*targets,'--parallel',str(args.jobs)], cwd=ROOT, check=True)

if __name__ == '__main__': main()
