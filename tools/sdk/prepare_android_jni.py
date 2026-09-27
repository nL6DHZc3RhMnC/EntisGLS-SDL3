#!/usr/bin/env python3
"""Generate the project JNI draw gate without changing the supplied official SDK."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
from pathlib import Path

if __package__:
    from .entis_sdk import COTOPHA, validate
else:
    from entis_sdk import COTOPHA, validate

SOURCE = COTOPHA / 'Source/android/gls4jclass/VirtualWindow_java.cpp'


def generate():
    validate()
    source = SOURCE.read_bytes()
    text = source.decode('utf-8-sig').replace('\r\n', '\n')
    original = 'pGenWnd->OnDraw() ;'
    if text.count(original) != 1:
        raise ValueError('Official VirtualWindow JNI must contain exactly one expected OnDraw call')
    text = text.replace(original, 'StudySteadyDrawAndroidWindow( pGenWnd ) ;')
    return ('// Generated from official SDK SHA-256 ' + hashlib.sha256(source).hexdigest() + '\n'
            '#include "runtime/cotopha_port/legacy_window_draw.h"\n' + text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    text = generate()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text() != text:
        args.output.write_text(text)
    print(args.output)


if __name__ == '__main__':
    main()
