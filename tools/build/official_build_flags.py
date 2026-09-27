"""Read probe flags only from a build configured against the official SDK."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import shlex

from entis_sdk import COTOPHA, LEGACY, ROOT, validate


def read_flags(build, targets=('legacy_objects', 'legacy_resource_compile_probe')):
    """Reject retired CMake caches before reusing their compiler arguments."""
    validate()
    build = Path(build).resolve()
    cache_path = build / 'CMakeCache.txt'
    if not cache_path.is_file():
        raise ValueError(f'Configure the official SDK build first: {cache_path}')
    cache = {}
    for line in cache_path.read_text().splitlines():
        if not line or line.startswith(('#', '//')) or '=' not in line:
            continue
        key, value = line.split('=', 1)
        cache[key.split(':', 1)[0]] = value
    for name, expected in (('ENTIS_ROOT', COTOPHA), ('LEGACY_ROOT', LEGACY)):
        actual = cache.get(name)
        if not actual or Path(actual).resolve() != expected.resolve():
            raise ValueError(
                f'{cache_path}: {name} must be {expected}; '
                f'found {actual!r}. Reconfigure using the official SDK.')
    flags_path = next((build / 'CMakeFiles' / (target + '.dir') / 'flags.make'
                       for target in targets
                       if (build / 'CMakeFiles' / (target + '.dir') / 'flags.make').is_file()), None)
    if flags_path is None:
        raise ValueError(f'No probe compile flags found in {build}; configure with Unix Makefiles.')
    flags = []
    for line in flags_path.read_text().splitlines():
        if line.startswith(('CXX_DEFINES =', 'CXX_INCLUDES =', 'CXX_FLAGS =')):
            flags.extend(shlex.split(line.split('=', 1)[1]))
    # A corrected cache is insufficient if flags.make was copied or not regenerated.
    retired = ((ROOT / 'EntisGLS4.07.03').resolve(),
               (ROOT / 'EntisGLS/EntisGLS4.07.03').resolve(),
               (ROOT / 'build/legacy').resolve())
    for flag in flags:
        if not flag.startswith('-I') or len(flag) == 2:
            continue
        include = Path(flag[2:]).resolve()
        if any(include == root or root in include.parents for root in retired):
            raise ValueError(f'{flags_path}: retired include path {include}; reconfigure the build.')
    return flags
