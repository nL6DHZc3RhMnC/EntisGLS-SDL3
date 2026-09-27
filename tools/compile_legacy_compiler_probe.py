#!/usr/bin/env python3
"""Compile the actual GLS3 expression compiler in an isolated directory."""
from pathlib import Path
import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor
from official_build_flags import read_flags
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--build',type=Path,default=ROOT/'build/android-arm64')
p.add_argument('--output',type=Path,default=ROOT/'build/legacy-compiler-probe')
a=p.parse_args()
BASE=a.build.resolve()
try:
    flags=read_flags(BASE)
except ValueError as error:
    p.error(str(error))
BUILD=a.output.resolve();BUILD.mkdir(parents=True,exist_ok=True)
clang=ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang++'
def compile(name):
    source=BASE/f'legacy_generated/GLS3/Source/{name}.cpp'
    # prepare.py already applied every compiler patch to this generated source.
    result=subprocess.run([str(clang),'--target=aarch64-none-linux-android29',
        '--sysroot='+str(clang.parent.parent/'sysroot'),*flags,'-ferror-limit=0','-c',str(source),'-o',str(BUILD/(name+'.o'))],
        stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (BUILD/(name+'.log')).write_text(result.stdout)
    return name,result.returncode,[line for line in result.stdout.splitlines() if 'error:' in line]
with ThreadPoolExecutor(max_workers=4) as pool:
    results=list(pool.map(compile,['glscs_compiler','glscs_assembler','glscs_execution_image_compiler','glscs_execution_image_linker']))
    for name,code,errors in results:
        print(name, 'PASS' if not code else 'FAIL');print('\n'.join(errors))
raise SystemExit(any(code for _,code,_ in results))
