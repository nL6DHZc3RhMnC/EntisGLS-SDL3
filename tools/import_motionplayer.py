#!/usr/bin/env python3
"""Copy the user-supplied MotionPlayer sources with hashes and provenance."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, help='kirikiroid2-web checkout')
    args = parser.parse_args()
    source = args.source.resolve()
    destination = ROOT / 'vendor/kirikiroid2'
    destination.mkdir(parents=True, exist_ok=True)
    files = {}
    for module in ('motionplayer', 'psbfile'):
        folder = source / 'cpp/plugins' / module
        for path in sorted(folder.rglob('*')):
            if not path.is_file():
                continue
            relative = Path(module) / path.relative_to(folder)
            target = destination / relative
            if target.exists() and target.read_bytes() != path.read_bytes():
                raise RuntimeError(f'Refusing to overwrite modified import: {target}')
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, target)
            files[str(relative)] = hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copy2(source / 'LICENSE', destination / 'LICENSE')
    commit = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    (destination / 'provenance.json').write_text(json.dumps({
        'source': str(source), 'source_commit': commit,
        'note': 'Copied working-tree files unchanged; SHA-256 records actual contents, including local edits.',
        'files': files,
    }, indent=2) + '\n')
    print(f'Imported {len(files)} files into {destination}')

if __name__ == '__main__':
    main()
