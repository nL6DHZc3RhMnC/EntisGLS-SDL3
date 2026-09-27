#!/usr/bin/env python3
"""Exercise direct tool entry points and the minimal release checkout in CI."""

import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


class ToolLayoutTest(unittest.TestCase):
    def help_from(self, script, cwd):
        result = subprocess.run([sys.executable, str(script), '--help'], cwd=cwd,
                                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn('usage:', result.stdout)

    def test_direct_entry_points_work_outside_checkout(self):
        # Help must resolve cross-category imports without configuring, building,
        # downloading, opening game files, or accessing signing credentials.
        with tempfile.TemporaryDirectory() as directory:
            for entry in (
                'tools/build/build_sdl_android.py',
                'tools/build/build_sdl_desktop.py',
                'tools/build/build_sdl_ios.py',
                'tools/ci/ci_release.py',
                'tools/sdk/setup_sdl_fonts.py',
                'tools/diagnostics/csx_inspect.py',
                'tools/diagnostics/motion_build.py',
            ):
                with self.subTest(entry=entry):
                    self.help_from(ROOT / entry, directory)

    def test_release_imports_in_sparse_checkout(self):
        with tempfile.TemporaryDirectory() as directory:
            stage = Path(directory)
            checkout = stage / 'checkout'
            cwd = stage / 'unrelated-directory'
            cwd.mkdir()
            for relative in ('tools/_bootstrap.py', 'tools/ci/ci_release.py',
                             'tools/ci/ci_ios_release.py'):
                target = checkout / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / relative, target)
            self.help_from(checkout / 'tools/ci/ci_release.py', cwd)


if __name__ == '__main__':
    unittest.main()
