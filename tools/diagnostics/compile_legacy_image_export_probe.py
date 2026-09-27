#!/usr/bin/env python3
"""Compile image export production/probe sources with Android build flags."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
from pathlib import Path
import subprocess
from official_build_flags import read_flags
parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, default=ROOT/'build/android-arm64')
args = parser.parse_args()
try:
    flags = read_flags(args.build, targets=('legacy_objects',))
except ValueError as error:
    parser.error(str(error))
clang = ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++'
for name, source in [
    ('legacy_image_export', 'native/runtime/cotopha_port/legacy_image_export.cpp'),
    ('legacy_atomic_path', 'native/runtime/cotopha_port/legacy_atomic_path.cpp'),
    ('legacy_atomic_file', 'native/runtime/cotopha_port/legacy_atomic_file.cpp'),
    ('legacy_resource', 'native/runtime/cotopha_port/legacy_resource.cpp'),
    ('legacy_script_file', 'native/runtime/cotopha_port/legacy_script_file.cpp'),
    ('legacy_image_export_probe', 'tests/probes/cotopha/legacy_image_export_probe.cpp'),
    ('legacy_atomic_save_probe', 'tests/probes/cotopha/legacy_atomic_save_probe.cpp'),
]:
    result = subprocess.run([str(clang), '--target=aarch64-none-linux-android29',
        '--sysroot='+str(clang.parent.parent/'sysroot'), *flags, '-c',
        str(ROOT / source), '-o', str(args.build/f'{name}-probe.o')],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    (args.build/f'{name}-compile.log').write_text(result.stdout)
    print(name+': '+('PASS' if result.returncode == 0 else 'FAIL'))
    for line in result.stdout.splitlines():
        if 'error:' in line:
            print(line)
    if result.returncode:
        raise SystemExit(result.returncode)
