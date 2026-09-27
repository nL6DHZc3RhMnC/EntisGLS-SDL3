#!/usr/bin/env python3
"""Compile the independent asset probe against an already-built ARM64 runtime.

Does not configure/build the APK or modify generated/runtime sources.
"""
import argparse
import json
import shlex
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-build', type=Path, default=ROOT / 'build/android-arm64/motion-tjs')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/analysis/motion-assets')
    args = parser.parse_args()
    target = args.runtime_build.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    definitions = {}
    for line in (target / 'CMakeFiles/motion_apk_runtime.dir/flags.make').read_text().splitlines():
        if ' = ' in line:
            key, value = line.split(' = ', 1)
            definitions[key] = shlex.split(value)
    link = shlex.split((target / 'CMakeFiles/motion_runtime_gles_probe.dir/link.txt').read_text())
    obj = output / 'motion_asset_batch_probe.o'
    binary = output / 'motion_asset_batch_probe'
    compile_command = [link[0], '--target=aarch64-none-linux-android29',
                       *definitions['CXX_DEFINES'], *definitions['CXX_INCLUDES'], *definitions['CXX_FLAGS'],
                       '-c', str(ROOT / 'tools/motion_asset_batch_probe.cpp'), '-o', str(obj)]
    link_command = []
    index = 0
    while index < len(link):
        argument = link[index]
        if argument == '-Xlinker' and link[index + 1].startswith('--dependency-file='):
            index += 2
            continue
        if argument == '-o':
            link_command.extend(['-o', str(binary)])
            index += 2
            continue
        link_command.append(str(obj) if argument.endswith('runtime_gles_probe.cpp.o') else argument)
        index += 1
    if '-static-libstdc++' not in link_command:
        link_command.append('-static-libstdc++')
    (output / 'build-commands.json').write_text(json.dumps({
        'compile_cwd': str(ROOT), 'compile': compile_command,
        'link_cwd': str(target), 'link': link_command}, indent=2) + '\n')
    for name, command, cwd in [('compile', compile_command, ROOT), ('link', link_command, target)]:
        result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (output / f'{name}.txt').write_text(result.stdout)
        print(name, result.returncode)
        if result.returncode:
            print(result.stdout)
            raise SystemExit(result.returncode)
    print(binary)


if __name__ == '__main__':
    main()
