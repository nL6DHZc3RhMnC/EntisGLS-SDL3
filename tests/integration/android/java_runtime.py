"""Find a host JDK without depending on this developer's bundled tool layout."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import os
from pathlib import Path
import shutil


def find_jdk(root: Path) -> Path:
    candidates = []
    if os.environ.get('JAVA_HOME'):
        candidates.append(Path(os.environ['JAVA_HOME']))
    candidates.extend((root / '.android-tools/jdk').glob('*/Contents/Home'))
    compiler = shutil.which('javac')
    if compiler:
        candidates.append(Path(compiler).resolve().parent.parent)
    for candidate in candidates:
        if (candidate / 'bin/javac').is_file() and (candidate / 'bin/java').is_file():
            return candidate
    raise RuntimeError('These Android host tests require a JDK; set JAVA_HOME.')
