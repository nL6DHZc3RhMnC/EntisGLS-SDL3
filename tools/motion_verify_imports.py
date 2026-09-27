#!/usr/bin/env python3
"""Verify imported source/dependency bytes against their recorded SHA-256."""
import hashlib
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]


def main():
    original = json.loads((ROOT / 'vendor/kirikiroid2/provenance.json').read_text())
    checks = [(ROOT / 'vendor/kirikiroid2' / name, sha) for name, sha in original['files'].items()]
    extra = json.loads((ROOT / 'vendor/motion-deps/provenance.json').read_text())
    checks.extend((ROOT / name, value['sha256']) for name, value in extra['files'].items())
    mismatches = [str(path.relative_to(ROOT)) for path, expected in checks
                  if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != expected]
    if mismatches: raise SystemExit('Imported source hash mismatch: ' + ', '.join(mismatches))
    print(f'Imported source SHA-256: {len(checks)}/{len(checks)} PASS')

if __name__ == '__main__': main()
