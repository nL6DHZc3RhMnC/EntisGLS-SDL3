#!/usr/bin/env python3
"""Validate the traditional runtime already expanded in the official SDK."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from entis_sdk import LEGACY, validate

if __name__ == '__main__':
    validate()
    print(f'Traditional Cotopha sources: {LEGACY}')
