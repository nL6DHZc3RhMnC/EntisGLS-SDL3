#!/usr/bin/env python3
"""Compile isolated ARM64 heap adapters/probe, without rebuilding the APK."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor
from official_build_flags import read_flags
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--build',type=Path,default=ROOT/'build/android-arm64')
p.add_argument('--output',type=Path,default=ROOT/'build/legacy_heap_sdk')
a=p.parse_args()
BASE=a.build.resolve()
try:
    target_flags={target:read_flags(BASE, targets=(target,))
                  for target in ('entisgls4', 'legacy_objects')}
except ValueError as error:
    p.error(str(error))
OUT=a.output.resolve();OUT.mkdir(parents=True,exist_ok=True)
NDK=ROOT/'.android-tools/ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64'

def compile_item(item):
    source,target=item
    flags=target_flags[target]
    output=OUT/(source.stem+'.o');log=OUT/(source.stem+'.compile.log')
    with log.open('w')as stream:
        result=subprocess.run([str(NDK/'bin/clang++'),'--target=aarch64-none-linux-android29','--sysroot='+str(NDK/'sysroot'),*flags,'-O0','-g0','-c',str(source),'-o',str(output)],stdout=stream,stderr=subprocess.STDOUT)
    print(source.name,'PASS' if not result.returncode else 'FAIL',flush=True)
    if result.returncode:
        for line in log.read_text().splitlines():
            if 'error:' in line:print(line,flush=True)
    return result.returncode
# Compile the exact sources emitted by the official build configuration. These
# already contain the heap patches; applying them again is not idempotent.
generated=BASE/'legacy_generated/GLS3/Source'
items=[(BASE/'legacy_heap_sdk/glscs_sakura2_obj_heap.cpp','entisgls4'),
       (generated/'glscs_execution_image.cpp','legacy_objects'),
       (generated/'glscsobj_buffer.cpp','legacy_objects'),
       (generated/'glscs_context.cpp','legacy_objects'),
       (ROOT/'tests/probes/cotopha/legacy_heap_probe.cpp','legacy_objects')]
with ThreadPoolExecutor(max_workers=3)as pool:results=list(pool.map(compile_item,items))
raise SystemExit(any(results))
