#!/usr/bin/env python3
"""Read script-name candidates from an existing original BMP/context save.

No device, game, or save is modified. Region-tagged strings distinguish current
global script names from older caller names on the stack and label history.
This reports evidence candidates, not a guessed current program counter.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
import json
import re
import struct
import subprocess
import tempfile
from pathlib import Path

SCRIPT = re.compile(r'(?:common|haz|nak|yuu|mai)[A-Za-z0-9_-]*(?:\.srcxml)?$')


def records(data, start):
    if data[start:start+6] != b'Entis\x1a':
        raise ValueError('Expected an Entis container')
    position = start + 64
    while position < len(data):
        if position + 16 > len(data):
            raise ValueError('Truncated record header')
        tag, size = struct.unpack_from('<8sQ', data, position)
        end = position + 16 + size
        if end > len(data):
            raise ValueError('Truncated record body')
        yield tag, position + 16, data[position+16:end]
        position = end


def locate(path):
    data = path.read_bytes()
    start = struct.unpack_from('<I', data, 2)[0] if data[:2] == b'BM' else 0
    context = None
    for tag, offset, body in records(data, start):
        if tag == b'ccontext':
            context = body
            break
        if tag == b'context ':
            if len(body) < 4:
                raise ValueError('Missing decoded size')
            size = struct.unpack_from('<I', body)[0]
            if size > 256*1024*1024:
                raise ValueError('Context exceeds decoder limit')
            with tempfile.TemporaryDirectory(prefix='study-save-location-') as folder:
                source, target = Path(folder)/'compressed', Path(folder)/'decoded'
                source.write_bytes(body[4:])
                subprocess.run([str(ROOT/'build/host/erisan_decode'), str(source),
                                str(target), str(size)], check=True,
                               stdout=subprocess.DEVNULL)
                context = target.read_bytes()
            break
    if context is None:
        raise ValueError('No script context record')
    hits = []
    for tag, start, body in records(context, 0):
        for match in re.finditer(rb'(?:[\x20-\x7e]\x00){5,}', body):
            text = match[0].decode('utf-16le')
            kind = 'script-name' if SCRIPT.fullmatch(text) else 'label-history' if '@' in text and SCRIPT.fullmatch(text.rsplit('@',1)[1]) else None
            if kind:
                hits.append({'region': tag.decode('ascii').strip(),
                             'offset': hex(start+match.start()), 'kind': kind, 'text': text})
    return {'file': str(path.resolve()), 'container_bytes': len(data),
            'container_sha256': hashlib.sha256(data).hexdigest(),
            'decoded_context_bytes': len(context),
            'decoded_context_sha256': hashlib.sha256(context).hexdigest(),
            'candidates': hits}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('save', type=Path)
    arguments = parser.parse_args()
    print(json.dumps(locate(arguments.save), ensure_ascii=False, indent=2))
