"""Paths to the supplied official SDK; never fall back to the previous bundle."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PACKAGE = ROOT / 'EntisGLS' / 'EntisGLS4.07.03'
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
