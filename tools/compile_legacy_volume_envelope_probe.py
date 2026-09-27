#!/usr/bin/env python3
"""Compile Resource envelope production/probe sources with Android build flags."""
import argparse
from pathlib import Path
import subprocess
from official_build_flags import read_flags
ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--build', type=Path, default=ROOT/'build/android-arm64')
args = parser.parse_args()
try:
    flags = read_flags(args.build, targets=('legacy_objects',))
except ValueError as error:
    parser.error(str(error))
clang = ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++'
for name in ['legacy_resource', 'legacy_audio_player', 'legacy_movie', 'legacy_volume_envelope_probe', 'legacy_resource_state_probe']:
    result = subprocess.run([str(clang), '--target=aarch64-none-linux-android29',
        '--sysroot='+str(clang.parent.parent/'sysroot'), *flags, '-c',
        str(ROOT/f'native/{name}.cpp'), '-o', str(args.build/f'{name}-probe.o')],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    (args.build/f'{name}-compile.log').write_text(result.stdout)
    print(name+': '+('PASS' if result.returncode == 0 else 'FAIL'))
    for line in result.stdout.splitlines():
        if 'error:' in line:
            print(line)
    if result.returncode:
        raise SystemExit(result.returncode)
