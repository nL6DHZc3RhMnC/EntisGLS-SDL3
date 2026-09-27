"""Paths to the supplied official SDK; never fall back to the previous bundle."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path

PACKAGE = ROOT / 'EntisGLS'
COTOPHA = PACKAGE / 'Cotopha'
LEGACY = PACKAGE / 'EntisGLS3'


def validate():
    for path in (COTOPHA / 'Include/common/sakura/ssys_file.h',
                 LEGACY / 'GLS3/Source/glscs_context.cpp'):
        if not path.is_file():
            raise FileNotFoundError(f'Official SDK input is missing: {path}')


if __name__ == '__main__':
    validate()
    print(PACKAGE)
