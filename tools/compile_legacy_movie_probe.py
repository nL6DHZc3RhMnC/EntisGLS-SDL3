#!/usr/bin/env python3
"""Compile isolated Android movie sources using the official SDK build."""
import argparse
from pathlib import Path
import subprocess
from official_build_flags import read_flags
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,default=ROOT/'build/android-arm64');a=p.parse_args()
try:
    flags=read_flags(a.build)
except ValueError as error:
    p.error(str(error))
clang=ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++'
for name in ['legacy_movie','legacy_movie_probe']:
    result=subprocess.run([str(clang),'--target=aarch64-none-linux-android29',
        '--sysroot='+str(clang.parent.parent/'sysroot'),*flags,'-c',
        str(ROOT/f'native/{name}.cpp'),'-o',str(a.build/f'{name}-probe.o')],
        stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (a.build/f'{name}-compile.log').write_text(result.stdout)
    print(name+': '+('PASS' if result.returncode==0 else 'FAIL'))
    for line in result.stdout.splitlines():
        if 'error:' in line:print(line)
    if result.returncode:raise SystemExit(result.returncode)
