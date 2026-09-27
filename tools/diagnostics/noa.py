#!/usr/bin/env python3
"""Read NOA indices and extract raw/ERISAN entries without modifying the game."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
from dataclasses import dataclass, asdict
import json
from pathlib import Path
import struct
import subprocess
import tempfile

@dataclass
class Entry:
    name: str
    size: int
    attributes: int
    encoding: int
    offset: int

def entries(path):
    with Path(path).open('rb') as f:
        header = f.read(64)
        if header[:8] != b'Entis\x1a\0\0':
            raise ValueError('Not an Entis archive')
        tag, length = struct.unpack('<8sQ', f.read(16))
        if tag != b'DirEntry' or length > 64 * 1024 * 1024:
            raise ValueError('Invalid root directory')
        buf = f.read(length)
    count = struct.unpack_from('<I', buf)[0]
    p = 4
    result = []
    for _ in range(count):
        size, attr, encoding, offset, _, extra = struct.unpack_from('<QIIQ8sI', buf, p)
        p += 36 + extra
        name_len = struct.unpack_from('<I', buf, p)[0]
        p += 4
        if name_len < 1 or p + name_len > len(buf) or buf[p + name_len - 1] != 0:
            raise ValueError('Invalid entry filename')
        name = buf[p:p + name_len - 1].decode('utf-8' if attr & 0x01000000 else 'cp932')
        p += name_len
        result.append(Entry(name, size, attr, encoding, offset))
    if p != len(buf):
        raise ValueError('Unexpected directory trailer')
    return result

def extract(path, name, destination):
    entry = next(e for e in entries(path) if e.name == name)
    if entry.attributes & 0x10:
        raise ValueError('Directory extraction is not supported')
    if entry.size > 256 * 1024 * 1024:
        raise ValueError('Entry exceeds extraction limit (256 MiB)')
    with Path(path).open('rb') as f:
        f.seek(64 + entry.offset)
        tag, length = struct.unpack('<8sQ', f.read(16))
        if length > 256 * 1024 * 1024:
            raise ValueError('Stored entry exceeds extraction limit')
        data = f.read(length)
        if len(data) != length:
            raise ValueError('Truncated entry')
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    if entry.encoding == 0:
        if len(data) != entry.size:
            raise ValueError('Raw entry size mismatch')
        destination.write_bytes(data)
    elif entry.encoding == 0x80000010:
        with tempfile.NamedTemporaryFile() as tmp:
            tmp.write(data)
            tmp.flush()
            subprocess.run([str(ROOT / 'build/host/erisan_decode'), tmp.name,
                            str(destination), str(entry.size)], check=True)
    else:
        raise ValueError(f'Unsupported entry encoding: {entry.encoding:#x}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--extract', metavar='NAME')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.extract:
        if not args.output:
            parser.error('--extract requires --output')
        extract(args.archive, args.extract, args.output)
    else:
        print(json.dumps([asdict(e) for e in entries(args.archive)], ensure_ascii=False, indent=2))
