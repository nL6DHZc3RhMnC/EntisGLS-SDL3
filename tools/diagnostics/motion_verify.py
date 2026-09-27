#!/usr/bin/env python3
"""Run actual TJS/NCB/PSB/Player probes and malformed-input rejection cases."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import json
from pathlib import Path
import subprocess
import tempfile
from psb_key_argument import parse_psb_key

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', type=Path, help='Probe build created by tools/diagnostics/motion_build.py (ENTISGLS_BUILD_DIAGNOSTICS=ON)', default=ROOT / 'build/motion-tjs-host')
    parser.add_argument('--psb', type=Path, default=ROOT / 'build/game/haz_a.psb')
    parser.add_argument('--output', type=Path, default=ROOT / 'artifacts/motion-verification-host.json')
    parser.add_argument('--psb-key', required=True, type=parse_psb_key, help='PSB header parameter supplied by the game owner')
    args = parser.parse_args()
    results = []

    def run(name, executable, params=(), code=0, tokens=()):
        binary = args.build_dir.resolve() / executable
        if not binary.is_file():
            raise RuntimeError(f'Missing diagnostic {binary}; build with tools/diagnostics/motion_build.py or -DENTISGLS_BUILD_DIAGNOSTICS=ON')
        command = [str(binary), *map(str,params)]
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
        output = result.stdout + result.stderr
        sanitizer_error = any(marker in output for marker in ('runtime error:', 'ERROR: AddressSanitizer', 'LeakSanitizer'))
        passed = result.returncode == code and all(token in output for token in tokens) and not sanitizer_error
        results.append(dict(name=name,passed=passed,exit_code=result.returncode,expected_exit_code=code,output=output))
        print(f'{name}: {"PASS" if passed else "FAIL"}',flush=True)

    run('real_tjs_execution','motion_tjs_probe',tokens=['StudySteady:5:4','PASS'])
    run('real_ncb_calls','motion_ncb_probe',tokens=['method=42: PASS'])
    run('real_psb_dispatch','motion_psb_tjs_probe',(args.psb.resolve(),args.psb_key),tokens=['win:4096:all_parts','67108864: PASS','384: PASS'])
    run('player_25_create_destroy_cycles','motion_player_probe',(args.psb.resolve(),args.psb_key,25),tokens=['nodes=26 child_players=13 parameters=3','cycles=25','move_UD=30','move_LR=30','body_slant=30'])
    run('mesh_combinator_and_rejections','motion_mesh_probe',(args.psb.resolve(),args.psb_key),tokens=['nodes=78 axes=278','rejection: PASS'])
    run('player_all_motion_timelines','motion_timeline_probe',(args.psb.resolve(),args.psb_key),tokens=['motions=45 nodes=379','combinators=78','neutral clears verified: PASS'])
    run('opaque_runtime_lifecycle','motion_runtime_probe',(args.psb.resolve(),args.psb_key),tokens=['cycles=3','capabilities=15','ResourceManager','PASS'])
    run('real_emote_scene_hierarchy','motion_scene_probe',(args.psb.resolve(),args.psb_key),tokens=['real_players=36 nodes=406','live texture bytes after ResourceManager release=0','PASS'])
    run('wrong_header_seed_rejected','motion_player_probe',(args.psb.resolve(),args.psb_key ^ 1),code=1)
    with tempfile.TemporaryDirectory(prefix='motion-rejection-') as directory:
        truncated = Path(directory) / 'truncated.psb'
        with args.psb.open('rb') as stream: truncated.write_bytes(stream.read(56))
        run('truncated_psb_rejected','motion_player_probe',(truncated,args.psb_key),code=1)
        run('missing_file_rejected','motion_player_probe',(Path(directory)/'missing.psb',args.psb_key),code=1)
    report = {'passed':all(row['passed'] for row in results),'tests':results,'full_game_playable':False,'rendering_tested_by_this_suite':False}
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    if not report['passed']: raise SystemExit(1)

if __name__ == '__main__': main()
