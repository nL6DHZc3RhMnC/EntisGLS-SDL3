#!/usr/bin/env python3
"""Check iOS publication boundaries with authored, synthetic Mach-O/IPA data."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


import hashlib
import json
from pathlib import Path
import plistlib
import struct
import tempfile
import unittest
import zipfile

import ci_ios_release as release


class IosReleaseTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.environment = dict(GITHUB_SHA='a' * 40, GITHUB_RUN_NUMBER='1', GITHUB_RUN_ATTEMPT='2',
                                GITHUB_SERVER_URL='https://github.com', GITHUB_REPOSITORY='example/engine',
                                GITHUB_RUN_ID='123')
        self.write_ipa()

    def write_ipa(self, platform=2, signed=False, extra=None):
        commands = struct.pack('<6I', 0x32, 24, platform, 13 << 16, 26 << 16, 0)
        if signed:
            commands += struct.pack('<4I', 0x1d, 16, 0, 0)
        binary = struct.pack('<8I', 0xfeedfacf, 0x100000c, 0, 2, 2 if signed else 1,
                             len(commands), 0, 0) + commands
        ipa = self.root / release.IPA_NAME
        with zipfile.ZipFile(ipa, 'w') as archive:
            archive.writestr(release.BUNDLE + 'Info.plist', plistlib.dumps({
                'CFBundleIdentifier': 'io.entisgls.launcher',
                'CFBundleExecutable': 'EntisGLSLauncher', 'MinimumOSVersion': '13.0'}))
            archive.writestr(release.BUNDLE + 'EntisGLSLauncher', binary)
            if extra:
                archive.writestr(extra, b'fixture')
        (self.root / release.REPORT_NAME).write_text(json.dumps(dict(
            platform='iOS', sdk='iphoneos', architectures=['arm64'], unsigned=True,
            archive_crc_verified=True, game_resources_included=False,
            bundle_id='io.entisgls.launcher', minimum_os='13.0',
            archive_sha256=hashlib.sha256(ipa.read_bytes()).hexdigest())))

    def test_unsigned_device_package_has_prerelease_tag_and_checksums(self):
        tag, notes, files = release.prepare(self.root, self.environment)
        self.assertEqual(tag, 'ios-dev-1.2-aaaaaaaa')
        self.assertIn('unsigned', notes)
        self.assertEqual(len(files), 3)
        for line in (self.root / 'SHA256SUMS.txt').read_text().splitlines():
            checksum, name = line.split('  ')
            self.assertEqual(hashlib.sha256((self.root / name).read_bytes()).hexdigest(), checksum)

    def test_rejects_simulator_or_signed_executable(self):
        for options in ({'platform': 7}, {'signed': True}):
            with self.subTest(options=options):
                self.write_ipa(**options)
                with self.assertRaises(RuntimeError):
                    release.prepare(self.root, self.environment)

    def test_rejects_provisioning_and_unsafe_archive_entries(self):
        for extra in (release.BUNDLE + 'embedded.mobileprovision',
                      release.BUNDLE + '_CodeSignature/CodeResources',
                      release.BUNDLE + '../unexpected', '/outside'):
            with self.subTest(entry=extra):
                self.write_ipa(extra=extra)
                with self.assertRaises(RuntimeError):
                    release.prepare(self.root, self.environment)

    def test_rejects_hash_mismatch_and_missing_report(self):
        ipa = self.root / release.IPA_NAME
        ipa.write_bytes(ipa.read_bytes() + b'changed')
        with self.assertRaisesRegex(RuntimeError, 'hash mismatch'):
            release.prepare(self.root, self.environment)
        (self.root / release.REPORT_NAME).unlink()
        with self.assertRaisesRegex(RuntimeError, 'asset set'):
            release.prepare(self.root, self.environment)

    def test_rejects_non_boolean_unsigned_and_included_game_resources(self):
        for key, value in (('unsigned', 'true'), ('unsigned', False), ('game_resources_included', True)):
            with self.subTest(field=key, value=value):
                self.write_ipa()
                path = self.root / release.REPORT_NAME
                report = json.loads(path.read_text())
                report[key] = value
                path.write_text(json.dumps(report))
                with self.assertRaisesRegex(RuntimeError, 'validation failed'):
                    release.prepare(self.root, self.environment)


if __name__ == '__main__':
    unittest.main()
