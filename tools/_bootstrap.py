"""Shared repository paths for directly executable developer scripts.

This file is the repository marker. Keep it in tools/ when reorganizing command
categories; all entry points find it by ancestor search, independent of cwd.
"""
from pathlib import Path
import sys

_MARKER = Path(__file__).resolve()
ROOT = next(parent for parent in _MARKER.parents
            if (parent / "tools" / "_bootstrap.py").resolve() == _MARKER)
for _category in ("diagnostics", "sdk", "ci", "build"):
    _directory = str(ROOT / "tools" / _category)
    if _directory not in sys.path:
        sys.path.insert(0, _directory)
