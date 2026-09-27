"""Find a host JDK without depending on this developer's bundled tool layout."""
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
