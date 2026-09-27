#!/usr/bin/env python3
"""Build and sign a local development APK using the project-local SDK."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import zipfile
from entis_sdk import COTOPHA, validate as validate_sdk
from official_build_flags import read_flags

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / '.android-tools'
BUILD = ROOT / 'build/apk'
JDK = next((TOOLS / 'jdk').glob('*/Contents/Home'))
SDK = next((TOOLS / 'build-tools').glob('*/aapt2')).parent
PLATFORM = next((TOOLS / 'platform').glob('*/android.jar'))
ENV = dict(os.environ, JAVA_HOME=str(JDK))
ENV['PATH'] = str(JDK / 'bin') + os.pathsep + ENV['PATH']

def run(args, **kw):
    return subprocess.run([str(a) for a in args], check=True, env=ENV, **kw)

def main():
    validate_sdk()
    read_flags(ROOT / 'build/android-arm64', targets=('entisgls4',))
    for name in ('java', 'classes', 'dex', 'assets'):
        (BUILD / name).mkdir(parents=True, exist_ok=True)
    source = COTOPHA / 'Source/android/java'
    # Recreate generated Java and class outputs so a changed SDK cannot leave
    # stale classes from the previous distribution in the APK.
    for name in ('java', 'classes', 'dex'):
        shutil.rmtree(BUILD / name)
        (BUILD / name).mkdir()
    for path in source.rglob('*.java'):
        # Unused unfinished camera sample is not part of the VN runtime.
        if path.name == 'CameraCapture.java':
            stale = BUILD / 'java' / path.relative_to(source)
            if stale.exists():
                stale.unlink()
            continue
        text = path.read_text(encoding='utf-8-sig')
        text = re.sub(r'(?m)^#elseif\b', '#elif', text)
        result = run(['clang', '-E', '-P', '-x', 'c', '-DANDROID_API_LEVEL=29',
                      '-DANDROID_BILLING=0', '-'], input=text,
                     text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        target = BUILD / 'java' / path.relative_to(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(result.stdout, encoding='utf-8')
    java_sources = list((BUILD / 'java').rglob('*.java')) + list((ROOT / 'android/java').rglob('*.java'))
    run([JDK / 'bin/javac', '-encoding', 'UTF-8', '-source', '8', '-target', '8',
         '-bootclasspath', PLATFORM, '-d', BUILD / 'classes', *java_sources])
    run([JDK / 'bin/jar', 'cf', BUILD / 'classes.jar', '-C', BUILD / 'classes', '.'])
    run([SDK / 'd8', '--lib', PLATFORM, '--min-api', '29', '--output', BUILD / 'dex', BUILD / 'classes.jar'])
    shutil.copytree(ROOT / 'android/assets', BUILD / 'assets', dirs_exist_ok=True)
    # The original archive opener now supplies script.csx, including patch priority.
    stale_script = BUILD / 'assets/script.csx'
    if stale_script.exists():
        stale_script.unlink()
    run([SDK / 'aapt2', 'link', '-I', PLATFORM, '--manifest', ROOT / 'android/AndroidManifest.xml',
         '-A', BUILD / 'assets', '-o', BUILD / 'unsigned.apk'])
    with zipfile.ZipFile(BUILD / 'unsigned.apk', 'a', compression=zipfile.ZIP_DEFLATED) as archive:
        archive.write(BUILD / 'dex/classes.dex', 'classes.dex')
        archive.write(ROOT / 'build/android-arm64/libentisgls4.so', 'lib/arm64-v8a/libentisgls4.so')
        libc = TOOLS / 'ndk/android-ndk-r27c/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so'
        archive.write(libc, 'lib/arm64-v8a/libc++_shared.so')
    run([SDK / 'zipalign', '-f', '4', BUILD / 'unsigned.apk', BUILD / 'aligned.apk'])
    key = TOOLS / 'development.keystore'
    if not key.exists():
        run([JDK / 'bin/keytool', '-genkeypair', '-keystore', key, '-storepass', 'android',
             '-keypass', 'android', '-alias', 'androiddebugkey', '-dname',
             'CN=StudySteady Local Development', '-keyalg', 'RSA', '-keysize', '2048',
             '-validity', '3650'])
    out = ROOT / 'artifacts/studysteady-arm64-dev.apk'
    out.parent.mkdir(parents=True, exist_ok=True)
    run([SDK / 'apksigner', 'sign', '--ks', key, '--ks-pass', 'pass:android',
         '--out', out, BUILD / 'aligned.apk'])
    run([SDK / 'apksigner', 'verify', '--verbose', out])
    print(out)

if __name__ == '__main__':
    main()
