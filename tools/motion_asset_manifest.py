#!/usr/bin/env python3
"""Read original NOA PSBs and emit a checked manifest for the GLES batch probe."""
import argparse
import hashlib
import struct
import zlib
from pathlib import Path
from noa import entries

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, default=ROOT / 'StudySteadyR18')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/analysis/motion-assets/manifest.tsv')
    args = parser.parse_args()
    rows = []
    # The patch assets are the previously unrendered coverage priority.
    for archive in ['stst_patch_R18.noa', 'psb.noa']:
        path = args.game / archive
        with path.open('rb') as source:
            for entry in entries(path):
                if not entry.name.lower().endswith('.psb') or entry.attributes & 0x10:
                    continue
                if entry.encoding != 0:
                    raise ValueError(f'{archive}/{entry.name}: expected raw NOA entry')
                source.seek(64 + entry.offset)
                tag, size = struct.unpack('<8sQ', source.read(16))
                if size != entry.size or not 56 <= size <= 256 * 1024 * 1024:
                    raise ValueError(f'{archive}/{entry.name}: invalid stored size')
                sha = hashlib.sha256()
                crc = 0
                remaining = size
                while remaining:
                    block = source.read(min(1024 * 1024, remaining))
                    if not block:
                        raise ValueError('truncated NOA entry')
                    sha.update(block)
                    crc = zlib.crc32(block, crc)
                    remaining -= len(block)
                rows.append(f'{archive}\t{entry.name}\t{80 + entry.offset}\t{size}\t{crc:08x}\t{sha.hexdigest()}')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text('# archive asset payload_offset bytes crc32 sha256\n' + '\n'.join(rows) + '\n')
    print(f'{len(rows)} original raw PSBs: {args.output}')


if __name__ == '__main__':
    main()
