#!/usr/bin/env python3
"""Host-independent checks for Android CI tool and signing resolution."""

import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import android_toolchain as toolchain


class AndroidToolchainTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        self.environment = patch.dict(os.environ, {}, clear=True)
        self.environment.start()
        self.local = patch.object(toolchain, "LOCAL", self.root / "local")
        self.local.start()

    def tearDown(self):
        self.local.stop()
        self.environment.stop()
        self.temporary.cleanup()

    def create(self, relative, content=""):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
        return path

    def ndk(self, relative):
        self.create(relative + "/source.properties", "Pkg.Revision = 27.2.12479018\n")
        self.create(relative + "/build/cmake/android.toolchain.cmake")
        return self.root / relative

    def sdk(self):
        prefix = "sdk/build-tools/35.0.0/"
        self.create(prefix + "source.properties", "Pkg.Revision=35.0.0\n")
        for name in ("aapt2", "d8", "zipalign", "apksigner", "core-lambda-stubs.jar"):
            self.create(prefix + name)
        self.create("sdk/platforms/android-35/source.properties", "AndroidVersion.ApiLevel = 35\n")
        self.create("sdk/platforms/android-35/android.jar")
        return self.root / "sdk"

    def test_standard_sdk_layout_and_ndk_environment_override(self):
        sdk = self.sdk()
        ndk = self.ndk("sdk/ndk/27.2.12479018")
        os.environ["ANDROID_HOME"] = str(sdk)
        self.assertEqual(toolchain.sdk_root(), sdk)
        self.assertEqual(toolchain.resolve_ndk(sdk=sdk), ndk)
        self.assertEqual(toolchain.resolve_sdk_tools(sdk),
                         (sdk / "build-tools/35.0.0", sdk / "platforms/android-35/android.jar"))
        override = self.ndk("override")
        os.environ["ANDROID_NDK_HOME"] = str(override)
        self.assertEqual(toolchain.resolve_ndk(sdk=sdk), override)
        self.assertEqual(toolchain.resolve_ndk(ndk, sdk), ndk)

    def test_invalid_explicit_ndk_does_not_fall_back(self):
        self.ndk("local/ndk/android-ndk-r27c")
        invalid = self.ndk("invalid")
        (invalid / "source.properties").write_text("Pkg.Revision=28.0.0\n")
        with self.assertRaisesRegex(RuntimeError, "Expected NDK r27c"):
            toolchain.resolve_ndk(invalid)

    def test_wrong_sdk_version_is_rejected(self):
        sdk = self.sdk()
        (sdk / "build-tools/35.0.0/source.properties").write_text("Pkg.Revision=34.0.0\n")
        with self.assertRaisesRegex(RuntimeError, "Expected Android build tools"):
            toolchain.resolve_sdk_tools(sdk)

    def test_local_signing_reuses_old_keystore(self):
        config = toolchain.resolve_signing()
        self.assertEqual(config.keystore, self.root / "local/development.keystore")
        self.assertEqual(config.alias, "androiddebugkey")
        self.assertTrue(config.create_local_key)

    def test_ci_never_generates_ephemeral_key(self):
        self.create("local/development.keystore")
        os.environ["CI"] = "true"
        with self.assertRaisesRegex(RuntimeError, "persistent"):
            toolchain.resolve_signing()
        os.environ["CI"] = "false"
        os.environ["GITHUB_ACTIONS"] = "true"
        with self.assertRaisesRegex(RuntimeError, "persistent"):
            toolchain.resolve_signing()

    def test_external_passwords_remain_environment_references(self):
        key = self.create("release.keystore")
        os.environ.update({"CI": "true", "ENTISGLS_ANDROID_KEYSTORE": str(key),
                           "ENTISGLS_ANDROID_KEY_ALIAS": "release",
                           "ENTISGLS_ANDROID_STORE_PASSWORD": "secret-never-print-me"})
        config = toolchain.resolve_signing()
        self.assertEqual(config.key_password_env, "ENTISGLS_ANDROID_STORE_PASSWORD")
        self.assertNotIn("secret-never-print-me", repr(config))
        self.assertFalse(config.create_local_key)
        os.environ["ENTISGLS_ANDROID_KEY_PASSWORD"] = "second-secret"
        self.assertEqual(toolchain.resolve_signing().key_password_env, "ENTISGLS_ANDROID_KEY_PASSWORD")
        del os.environ["ENTISGLS_ANDROID_STORE_PASSWORD"]
        with self.assertRaisesRegex(RuntimeError, "requires ENTISGLS_ANDROID_STORE_PASSWORD"):
            toolchain.resolve_signing()


if __name__ == "__main__":
    unittest.main()
