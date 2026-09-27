"""Resolve pinned Android tools on macOS/Linux, including hosted CI runners.

Explicit options and environment variables take precedence over the historical
project-local installation. Passwords stay in the environment, never argv.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


from dataclasses import dataclass
import os
from pathlib import Path
import re
import subprocess

LOCAL = ROOT / ".android-tools"
NDK_REVISION = "27.2.12479018"
BUILD_TOOLS_REVISION = "35.0.0"
PLATFORM_API = "35"


def add_toolchain_arguments(parser, *, native=False):
    parser.add_argument("--sdk-root", type=Path, help="Android SDK root (otherwise ANDROID_HOME/ANDROID_SDK_ROOT or project-local tools)")
    parser.add_argument("--java-home", type=Path, help="JDK 17 home (otherwise JAVA_HOME or project-local JDK)")
    if native:
        parser.add_argument("--ndk", type=Path, help="NDK r27c directory (otherwise ANDROID_NDK_HOME/ANDROID_NDK_ROOT, SDK root, or project-local NDK)")


def sdk_root(explicit=None):
    value = explicit or os.environ.get("ANDROID_HOME") or os.environ.get("ANDROID_SDK_ROOT")
    if value:
        path = Path(value).expanduser().resolve()
        if not path.is_dir():
            raise RuntimeError(f"Android SDK root does not exist: {path}")
        return path
    return None


def properties(path):
    if not path.is_file():
        raise RuntimeError(f"Android package metadata is missing: {path}")
    return {key.strip(): value.strip() for line in path.read_text().splitlines()
            if "=" in line for key, value in [line.split("=", 1)]}


def validate_ndk(path):
    path = Path(path).expanduser().resolve()
    revision = properties(path / "source.properties").get("Pkg.Revision")
    if revision != NDK_REVISION:
        raise RuntimeError(f"Expected NDK r27c ({NDK_REVISION}), found {revision} at {path}")
    if not (path / "build/cmake/android.toolchain.cmake").is_file():
        raise RuntimeError(f"NDK CMake toolchain is missing: {path}")
    return path


def resolve_ndk(explicit=None, sdk=None):
    value = explicit or os.environ.get("ANDROID_NDK_HOME") or os.environ.get("ANDROID_NDK_ROOT")
    return validate_ndk(value or (sdk / "ndk" / NDK_REVISION if sdk else LOCAL / "ndk/android-ndk-r27c"))


def resolve_java(explicit=None):
    value = explicit or os.environ.get("JAVA_HOME")
    if not value:
        candidates = sorted((LOCAL / "jdk").glob("*/Contents/Home"))
        candidates += sorted(path.parent.parent for path in (LOCAL / "jdk").glob("*/bin/javac"))
        # Linux archives have no Contents/Home component.
        if len(candidates) != 1:
            raise RuntimeError("Set JAVA_HOME or --java-home to a JDK 17 installation")
        value = candidates[0]
    path = Path(value).expanduser().resolve()
    for name in ("java", "javac", "jar", "keytool"):
        if not (path / "bin" / name).is_file():
            raise RuntimeError(f"JDK tool is missing: {path / 'bin' / name}")
    result = subprocess.run([str(path / "bin/javac"), "-version"], check=True, capture_output=True, text=True)
    if not re.search(r"\bjavac 17(?:\.|\s|$)", result.stdout + result.stderr):
        raise RuntimeError(f"Expected JDK 17: {path}")
    return path


def resolve_sdk_tools(sdk=None):
    tools = (sdk / "build-tools" if sdk else LOCAL / "build-tools") / BUILD_TOOLS_REVISION
    if not tools.is_dir() and sdk is None:
        candidates = [path.parent for path in (LOCAL / "build-tools").glob("*/aapt2")]
        if len(candidates) == 1:
            tools = candidates[0]
    revision = properties(tools / "source.properties").get("Pkg.Revision")
    if revision != BUILD_TOOLS_REVISION:
        raise RuntimeError(f"Expected Android build tools {BUILD_TOOLS_REVISION}, found {revision}")
    platform = (sdk / "platforms" if sdk else LOCAL / "platform") / ("android-" + PLATFORM_API)
    if not platform.is_dir() and sdk is None:
        candidates = [path.parent for path in (LOCAL / "platform").glob("*/android.jar")]
        if len(candidates) == 1:
            platform = candidates[0]
    if properties(platform / "source.properties").get("AndroidVersion.ApiLevel") != PLATFORM_API:
        raise RuntimeError(f"Expected Android platform API {PLATFORM_API}: {platform}")
    for name in ("aapt2", "d8", "zipalign", "apksigner", "core-lambda-stubs.jar"):
        if not (tools / name).is_file():
            raise RuntimeError(f"Android build tool is missing: {tools / name}")
    if not (platform / "android.jar").is_file():
        raise RuntimeError(f"Android platform jar is missing: {platform}")
    return tools, platform / "android.jar"


@dataclass
class SigningConfig:
    keystore: Path
    alias: str
    store_password_env: str
    key_password_env: str
    create_local_key: bool = False


def resolve_signing():
    value = os.environ.get("ENTISGLS_ANDROID_KEYSTORE")
    if value:
        key = Path(value).expanduser().resolve()
        if not key.is_file():
            raise RuntimeError(f"Signing keystore does not exist: {key}")
        for name in ("ENTISGLS_ANDROID_KEY_ALIAS", "ENTISGLS_ANDROID_STORE_PASSWORD"):
            if not os.environ.get(name):
                raise RuntimeError(f"External signing requires {name}")
        password_env = ("ENTISGLS_ANDROID_KEY_PASSWORD" if os.environ.get("ENTISGLS_ANDROID_KEY_PASSWORD")
                        else "ENTISGLS_ANDROID_STORE_PASSWORD")
        return SigningConfig(key, os.environ["ENTISGLS_ANDROID_KEY_ALIAS"],
                             "ENTISGLS_ANDROID_STORE_PASSWORD", password_env)
    if os.environ.get("CI", "").lower() not in ("", "false", "0") or os.environ.get("GITHUB_ACTIONS") == "true":
        raise RuntimeError("CI release builds require a persistent ENTISGLS_ANDROID_KEYSTORE and signing secrets; temporary development keys are not allowed")
    return SigningConfig(LOCAL / "development.keystore", "androiddebugkey",
                         "ENTISGLS_LOCAL_STORE_PASSWORD", "ENTISGLS_LOCAL_STORE_PASSWORD", True)
