#!/usr/bin/env python3
"""Compile isolated Android audio_player sources using the official SDK build."""

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
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,default=ROOT/'build/android-arm64');a=p.parse_args()
try:
    flags=read_flags(a.build)
except ValueError as error:
    p.error(str(error))
clang=ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++'
for name, source in [
    ('legacy_resource', 'native/runtime/cotopha_port/legacy_resource.cpp'),
    ('legacy_audio_player', 'native/runtime/cotopha_port/legacy_audio_player.cpp'),
    ('legacy_audio_player_probe', 'tests/probes/cotopha/legacy_audio_player_probe.cpp'),
]:
    result=subprocess.run([str(clang),'--target=aarch64-none-linux-android29',
        '--sysroot='+str(clang.parent.parent/'sysroot'),*flags,'-c',
        str(ROOT / source),'-o',str(a.build/f'{name}-probe.o')],
        stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (a.build/f'{name}-compile.log').write_text(result.stdout)
    print(name+': '+('PASS' if result.returncode==0 else 'FAIL'))
    for line in result.stdout.splitlines():
        if 'error:' in line:print(line)
    if result.returncode:raise SystemExit(result.returncode)
