#!/usr/bin/env python3
"""Validate release assembly with synthetic packages; never access GitHub."""

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
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile

import ci_release as release
from ci_ios_release import BUNDLE, IPA_NAME, REPORT_NAME


class ReleaseTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.assets = self.root / "assets"
        self.assets.mkdir()
        self.environment = {
            "GITHUB_SHA": "a" * 40,
            "GITHUB_REF_TYPE": "branch",
            "GITHUB_REF_NAME": "main",
            "GITHUB_RUN_NUMBER": "27",
            "GITHUB_RUN_ATTEMPT": "1",
            "GITHUB_RUN_ID": "123456",
            "GITHUB_SERVER_URL": "https://github.com",
            "GITHUB_REPOSITORY": "example/launcher",
        }
        self.write_package("EntisGLSLauncher-android-arm64.apk", b"synthetic APK", {
            "native_packaged": True,
            "elf_16k_compatible": True,
            "game_resources_bundled": False,
        }, "apk_sha256")
        for arch in ("x86_64", "arm64"):
            self.write_package(f"EntisGLSLauncher-macos-{arch}.zip", arch.encode(), {
                "architectures": [arch],
                "archive_signature_verified": True,
                "archive_crc_verified": True,
                "game_resources_included": False,
            }, "archive_sha256")
            (self.assets / f"EntisGLSLauncher-macos-{arch}.tests.json").write_text(
                json.dumps({"passed": True}))
        self.write_ios_package()

    def write_ios_package(self, platform=2, signed=False, extra=None):
        commands = struct.pack('<6I', 0x32, 24, platform, 13 << 16, 18 << 16, 0)
        if signed:
            commands += struct.pack('<4I', 0x1d, 16, 0, 0)
        binary = struct.pack('<8I', 0xfeedfacf, 0x100000c, 0, 2, 2 if signed else 1,
                             len(commands), 0, 0) + commands
        ipa = self.assets / IPA_NAME
        with zipfile.ZipFile(ipa, 'w') as archive:
            archive.writestr(BUNDLE + 'Info.plist', plistlib.dumps({
                'CFBundleIdentifier': 'io.entisgls.launcher',
                'CFBundleExecutable': 'EntisGLSLauncher', 'MinimumOSVersion': '13.0'}))
            archive.writestr(BUNDLE + 'EntisGLSLauncher', binary)
            if extra:
                archive.writestr(extra, b'fixture')
        (self.assets / REPORT_NAME).write_text(json.dumps(dict(
            platform='iOS', sdk='iphoneos', architectures=['arm64'], unsigned=True,
            archive_crc_verified=True, archive_binary_verified=True, macho_platform_verified=True,
            game_resources_included=False, bundle_id='io.entisgls.launcher', minimum_os='13.0',
            archive_sha256=hashlib.sha256(ipa.read_bytes()).hexdigest())))

    def write_package(self, name, content, report, hash_field):
        package = self.assets / name
        package.write_bytes(content)
        report[hash_field] = hashlib.sha256(content).hexdigest()
        package.with_suffix(".build.json").write_text(json.dumps(report))

    def change_report(self, filename, key, value):
        path = self.assets / filename
        report = json.loads(path.read_text())
        report[key] = value
        path.write_text(json.dumps(report))

    def prepare(self):
        return release.prepare(self.assets, self.environment)

    def development_tag(self, run, attempt):
        self.environment['GITHUB_RUN_NUMBER'] = run
        self.environment['GITHUB_RUN_ATTEMPT'] = attempt
        tag = self.prepare()[0]
        (self.assets / 'SHA256SUMS.txt').unlink()
        return tag

    def test_development_release_has_unique_attempt_tag_and_verified_checksums(self):
        tag, prerelease, notes, assets = self.prepare()
        self.assertEqual(tag, "dev-000027.01-aaaaaaaa")
        self.assertIs(prerelease, True)
        self.assertIn("https://github.com/example/launcher/actions/runs/123456", notes)
        self.assertIn("ad-hoc signatures", notes)
        self.assertIn("The iOS IPA is unsigned", notes)
        self.assertIn("not tested by this workflow", notes)
        self.assertEqual(len(assets), 11)
        lines = (self.assets / "SHA256SUMS.txt").read_text().splitlines()
        self.assertEqual(len(lines), 10)
        for line in lines:
            checksum, filename = line.split("  ")
            self.assertEqual(checksum, hashlib.sha256((self.assets / filename).read_bytes()).hexdigest())
        (self.assets / "SHA256SUMS.txt").unlink()
        self.environment["GITHUB_RUN_ATTEMPT"] = "2"
        self.assertEqual(self.prepare()[0], "dev-000027.02-aaaaaaaa")

    def test_development_tags_sort_across_run_digit_boundaries(self):
        runs = (1, 9, 10, 99, 100, 999, 1000, 9999, 10000, 99999, 100000, 999999)
        tags = [self.development_tag(str(run), '1') for run in runs]
        self.assertEqual(tags, sorted(tags))
        self.assertEqual(tags[1:5], ['dev-000009.01-aaaaaaaa', 'dev-000010.01-aaaaaaaa',
                                    'dev-000099.01-aaaaaaaa', 'dev-000100.01-aaaaaaaa'])
        self.assertEqual(tags[-1], 'dev-999999.01-aaaaaaaa')

    def test_development_tags_sort_attempts_within_each_run(self):
        tags = [self.development_tag('27', str(attempt)) for attempt in (1, 9, 10, 99)]
        self.assertEqual(tags, sorted(tags))
        self.assertEqual(tags[1:], ['dev-000027.09-aaaaaaaa', 'dev-000027.10-aaaaaaaa',
                                    'dev-000027.99-aaaaaaaa'])
        self.assertLess(tags[-1], self.development_tag('28', '1'))

    def test_rejects_invalid_or_overflowing_development_counters(self):
        for name, overflow in (('GITHUB_RUN_NUMBER', '1000000'), ('GITHUB_RUN_ATTEMPT', '100')):
            original = self.environment[name]
            for invalid in ('', '0', '-1', '+1', '01', '1.0', '1e2', ' 1', '1 ', '1\n',
                            '١', '１', 'not-a-number', overflow, None, 1):
                with self.subTest(counter=name, invalid=invalid):
                    self.environment[name] = invalid
                    with self.assertRaisesRegex(RuntimeError, name):
                        self.prepare()
            del self.environment[name]
            with self.subTest(counter=name, missing=True):
                with self.assertRaisesRegex(RuntimeError, name):
                    self.prepare()
            self.environment[name] = original

    def test_version_and_prerelease_tags(self):
        self.environment["GITHUB_REF_TYPE"] = "tag"
        # Explicit version tags do not use the development numbering scheme.
        self.environment.pop('GITHUB_RUN_NUMBER')
        self.environment.pop('GITHUB_RUN_ATTEMPT')
        for tag, prerelease in (("v0.4.0", False), ("v0.4.0-rc1", True)):
            with self.subTest(tag=tag):
                self.environment["GITHUB_REF_NAME"] = tag
                actual, is_prerelease, _, _ = self.prepare()
                self.assertEqual(actual, tag)
                self.assertIs(is_prerelease, prerelease)
                (self.assets / "SHA256SUMS.txt").unlink()

    def test_rejects_invalid_version_tag_and_source_sha(self):
        self.environment["GITHUB_REF_TYPE"] = "tag"
        self.environment["GITHUB_REF_NAME"] = "--help"
        with self.assertRaisesRegex(RuntimeError, "Version tags"):
            self.prepare()
        self.environment["GITHUB_REF_NAME"] = "v1.0"
        self.environment["GITHUB_SHA"] = "not a source commit"
        with self.assertRaisesRegex(RuntimeError, "Invalid source commit"):
            self.prepare()

    def test_rejects_missing_or_extra_release_assets(self):
        extra = self.assets / "unintended-game.noa"
        extra.write_bytes(b"unexpected")
        with self.assertRaisesRegex(RuntimeError, "Unexpected release asset set"):
            self.prepare()
        extra.unlink()
        (self.assets / "EntisGLSLauncher-android-arm64.apk").unlink()
        with self.assertRaisesRegex(RuntimeError, "Unexpected release asset set"):
            self.prepare()

    def test_rejects_empty_or_symlinked_assets(self):
        package = self.assets / "EntisGLSLauncher-android-arm64.apk"
        package.write_bytes(b"")
        with self.assertRaisesRegex(RuntimeError, "Invalid release asset"):
            self.prepare()
        external = self.root / "external.apk"
        external.write_bytes(b"synthetic APK")
        package.unlink()
        package.symlink_to(external)
        with self.assertRaisesRegex(RuntimeError, "Invalid release asset"):
            self.prepare()

    def test_rejects_package_content_hash_mismatch(self):
        for filename in ("EntisGLSLauncher-android-arm64.apk", "EntisGLSLauncher-macos-arm64.zip", IPA_NAME):
            with self.subTest(filename=filename):
                path = self.assets / filename
                original = path.read_bytes()
                path.write_bytes(original + b"corrupt")
                with self.assertRaisesRegex(RuntimeError, "hash mismatch"):
                    self.prepare()
                path.write_bytes(original)

    def test_rejects_failed_mac_build_validation(self):
        filename = "EntisGLSLauncher-macos-x86_64.build.json"
        for key, invalid, valid in (
            ("architectures", ["arm64"], ["x86_64"]),
            ("archive_signature_verified", False, True),
            ("archive_crc_verified", False, True),
            ("game_resources_included", True, False),
        ):
            with self.subTest(field=key):
                self.change_report(filename, key, invalid)
                with self.assertRaisesRegex(RuntimeError, "macOS package validation failed"):
                    self.prepare()
                self.change_report(filename, key, valid)

    def test_requires_ios_package_and_report(self):
        for filename in (IPA_NAME, REPORT_NAME):
            with self.subTest(filename=filename):
                self.write_ios_package()
                (self.assets / filename).unlink()
                with self.assertRaisesRegex(RuntimeError, "Unexpected release asset set"):
                    self.prepare()

    def test_rejects_failed_ios_build_validation(self):
        for key, value in (
            ('platform', 'iOS Simulator'), ('sdk', 'iphonesimulator'),
            ('architectures', ['x86_64']), ('unsigned', False), ('unsigned', 'true'),
            ('archive_crc_verified', False), ('archive_binary_verified', False),
            ('macho_platform_verified', False), ('game_resources_included', True),
            ('bundle_id', 'unexpected.bundle'),
        ):
            with self.subTest(field=key, value=value):
                self.write_ios_package()
                self.change_report(REPORT_NAME, key, value)
                with self.assertRaisesRegex(RuntimeError, "iOS package validation failed"):
                    self.prepare()

    def test_rejects_ios_simulator_or_signed_executable_despite_success_report(self):
        for options in ({'platform': 7}, {'signed': True}):
            with self.subTest(options=options):
                self.write_ios_package(**options)
                with self.assertRaises(RuntimeError):
                    self.prepare()

    def test_rejects_ios_provisioning_and_unsafe_archive_entries(self):
        for extra in (BUNDLE + 'embedded.mobileprovision',
                      BUNDLE + '_CodeSignature/CodeResources',
                      BUNDLE + '../unexpected', '/outside'):
            with self.subTest(entry=extra):
                self.write_ios_package(extra=extra)
                with self.assertRaises(RuntimeError):
                    self.prepare()

    def test_rejects_non_boolean_success_and_failed_native_checks(self):
        for invalid in (False, "true", 1, None):
            with self.subTest(value=invalid):
                self.change_report("EntisGLSLauncher-macos-arm64.tests.json", "passed", invalid)
                with self.assertRaisesRegex(RuntimeError, "macOS native checks failed"):
                    self.prepare()

    def test_rejects_failed_android_validation(self):
        filename = "EntisGLSLauncher-android-arm64.build.json"
        for key, invalid, valid in (
            ("native_packaged", False, True),
            ("elf_16k_compatible", False, True),
            ("game_resources_bundled", True, False),
        ):
            with self.subTest(field=key):
                self.change_report(filename, key, invalid)
                with self.assertRaisesRegex(RuntimeError, "Android package validation failed"):
                    self.prepare()
                self.change_report(filename, key, valid)

    def test_publish_uploads_draft_then_publishes(self):
        with patch.dict(release.os.environ, self.environment, clear=True), \
                patch("sys.argv", ["ci_release.py", "--artifacts", str(self.assets)]), \
                patch.object(release.subprocess, "run") as run:
            release.main()
        self.assertEqual(run.call_count, 2)
        create = run.call_args_list[0].args[0]
        publish = run.call_args_list[1].args[0]
        self.assertEqual(create[:4], ["gh", "release", "create", "dev-000027.01-aaaaaaaa"])
        self.assertIn("--draft", create)
        self.assertIn("--prerelease", create)
        self.assertIn("--latest=false", create)
        self.assertEqual(create[create.index("--target") + 1], "a" * 40)
        self.assertTrue(Path(create[create.index("--notes-file") + 1]).is_file())
        self.assertIn("--draft=false", publish)
        self.assertIn("--latest=false", publish)

    def test_failed_upload_never_publishes(self):
        with patch.dict(release.os.environ, self.environment, clear=True), \
                patch("sys.argv", ["ci_release.py", "--artifacts", str(self.assets)]), \
                patch.object(release.subprocess, "run",
                             side_effect=subprocess.CalledProcessError(1, "gh")) as run:
            with self.assertRaises(subprocess.CalledProcessError):
                release.main()
        self.assertEqual(run.call_count, 1)


if __name__ == "__main__":
    unittest.main()
