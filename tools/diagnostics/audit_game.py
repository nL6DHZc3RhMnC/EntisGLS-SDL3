#!/usr/bin/env python3
"""Verify NOA storage modes and actual game PSBs with the native parser."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
from collections import Counter
import json
from pathlib import Path
import subprocess
import tempfile
import noa
from psb_key_argument import parse_psb_key

GAME = ROOT / 'StudySteadyR18'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--psb-key', required=True, type=parse_psb_key, help='PSB header parameter supplied by the game owner')
    args = parser.parse_args()
    report = {'archives': [], 'motion_resources': [], 'complete_game_playable': False}
    with tempfile.TemporaryDirectory(prefix='studysteady-psb-') as temp:
        sample = Path(temp) / 'sample.psb'
        for archive in sorted(GAME.glob('*.noa')):
            entries = noa.entries(archive)
            report['archives'].append({
                'name': archive.name, 'bytes': archive.stat().st_size,
                'entries': len(entries),
                'encodings': {hex(k): v for k, v in Counter(e.encoding for e in entries).items()},
            })
            for entry in entries:
                if not entry.name.lower().endswith('.psb'):
                    continue
                noa.extract(archive, entry.name, sample)
                result = subprocess.run([str(ROOT / 'build/host/psb_probe'), str(sample),
                                         str(args.psb_key)], text=True, capture_output=True, timeout=60)
                report['motion_resources'].append({
                    'archive': archive.name, 'entry': entry.name, 'bytes': entry.size,
                    'passed': result.returncode == 0,
                    'diagnostic': result.stdout + result.stderr,
                })
                print(f'{archive.name}/{entry.name}: {"PASS" if result.returncode == 0 else "FAIL"}', flush=True)
    report['total_archive_bytes'] = sum(a['bytes'] for a in report['archives'])
    report['motion_passed'] = sum(m['passed'] for m in report['motion_resources'])
    report['motion_total'] = len(report['motion_resources'])
    target = ROOT / 'artifacts/game-audit.json'
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print(f'Motion parser: {report["motion_passed"]}/{report["motion_total"]} passed')
    print(target)

if __name__ == '__main__':
    main()
