#!/usr/bin/env python3
"""Build and package the SDL desktop app without modifying the game or SDK."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import stat
import subprocess
import tempfile
import zipfile

def run(*command, capture=False, check=True):
    return subprocess.run([str(value) for value in command], cwd=ROOT, check=check,
                          text=True, capture_output=capture)


def clear_bundle_finder_attributes(bundle):
    """Remove only signature-incompatible Finder metadata from generated copies."""
    for path in [bundle, *bundle.rglob('*')]:
        names = run('/usr/bin/xattr', '-s', path, capture=True).stdout.splitlines()
        for attribute in ('com.apple.FinderInfo', 'com.apple.ResourceFork'):
            if attribute in names:
                run('/usr/bin/xattr', '-s', '-d', attribute, path)


def check_architectures(binary, expected):
    actual = run('/usr/bin/lipo', '-archs', binary, capture=True).stdout.split()
    if set(actual) != set(expected):
        raise RuntimeError(f'Expected architectures {expected}, found {actual} in {binary}')
    return actual


def write_bundle_zip(bundle, archive):
    # Python's ZIP writer stores file data and POSIX modes, without xattrs or
    # AppleDouble entries. Preserve framework symlinks if the bundle gains them.
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as output:
        for path in [bundle, *sorted(bundle.rglob('*'))]:
            name = path.relative_to(bundle.parent).as_posix()
            if path.is_symlink():
                entry = zipfile.ZipInfo(name)
                entry.create_system = 3
                entry.external_attr = (stat.S_IFLNK | 0o777) << 16
                output.writestr(entry, os.readlink(path))
            else:
                output.write(path, name)
    with zipfile.ZipFile(archive) as package:
        bad_entry = package.testzip()
        if bad_entry:
            raise RuntimeError(f'ZIP CRC verification failed: {bad_entry}')


def package_app(source, destination, architectures, deployment_target):
    check_architectures(source/'Contents/MacOS/EntisGLSLauncher', architectures)
    destination.mkdir(parents=True, exist_ok=True)
    bundle = destination/source.name
    archive = destination/'EntisGLSLauncher.zip'
    font_info = ROOT/'assets/fonts/provenance.json'
    font = json.loads(font_info.read_text()) if font_info.is_file() else None
    if font and not (source/'Contents/Resources'/font['file']).is_file(): font = None
    # Documents can be managed by FileProvider, which adds FinderInfo to .app
    # roots. Sign and validate on a system temporary filesystem instead.
    with tempfile.TemporaryDirectory(prefix='entisgls-package-') as temporary:
        stage = Path(temporary)/source.name
        run('/usr/bin/ditto', source, stage)
        font_hash = None
        if font:
            font_hash = hashlib.sha256((stage/'Contents/Resources'/font['file']).read_bytes()).hexdigest()
            if font_hash != font['sha256']:
                raise RuntimeError('Optional compatibility font differs from the verified input')
            if not (stage/'Contents/Resources'/font['license_file']).is_file():
                raise RuntimeError('Optional compatibility font license is missing')
        clear_bundle_finder_attributes(stage)
        run('/usr/bin/codesign', '--force', '--sign', '-', stage)
        run('/usr/bin/codesign', '--verify', '--strict', stage)
        signed_hash = hashlib.sha256((stage/'Contents/MacOS/EntisGLSLauncher').read_bytes()).hexdigest()
        write_bundle_zip(stage, archive)
        extracted = Path(temporary)/'extracted'
        run('/usr/bin/ditto', '-x', '-k', archive, extracted)
        extracted_app = extracted/source.name
        run('/usr/bin/codesign', '--verify', '--strict', extracted_app)
        actual_architectures = check_architectures(
            extracted_app/'Contents/MacOS/EntisGLSLauncher', architectures)
        extracted_hash = hashlib.sha256(
            (extracted_app/'Contents/MacOS/EntisGLSLauncher').read_bytes()).hexdigest()
        if extracted_hash != signed_hash:
            raise RuntimeError('Archived executable differs from the signed executable')
        archived_font_hash = None
        if font:
            archived_font_hash = hashlib.sha256(
                (extracted_app/'Contents/Resources'/font['file']).read_bytes()).hexdigest()
            if archived_font_hash != font_hash:
                raise RuntimeError('Archived compatibility font differs from the signed bundle')
        # Replace only this tool's generated output; never touch SDK/game files.
        if bundle.exists():
            shutil.rmtree(bundle)
        run('/usr/bin/ditto', stage, bundle)
        clear_bundle_finder_attributes(bundle)
        bundle_check = run('/usr/bin/codesign', '--verify', '--strict', bundle,
                           capture=True, check=False)
        report = {
            'platform': 'macOS', 'architectures': actual_architectures,
            'deployment_target': deployment_target, 'bundle': str(bundle),
            'archive': str(archive), 'binary_sha256': signed_hash,
            'archive_sha256': hashlib.sha256(archive.read_bytes()).hexdigest(),
            'signature': 'local ad-hoc', 'archive_crc_verified': True,
            'archive_signature_verified': True,
            'bundle_signature_verified': bundle_check.returncode == 0,
            'runtime_verified_by_this_command': False,
            'game_resources_included': False,
            'bundled_font': {'family': font['family'], 'file': font['file'],
                             'sha256': archived_font_hash, 'bytes': font['size_bytes'],
                             'verified_in_archive': True} if font else None,
            'first_launch': 'Choose a game directory containing cotopha.xml, entis-launcher.xml, an original EXE with IDR_COTOMI (including an appended PE), or script.noa containing a traditional Cotopha script.csx for inferred configuration.',
        }
        if bundle_check.returncode:
            report['bundle_verification_error'] = (bundle_check.stderr or bundle_check.stdout).strip()
            print('Copied .app verification failed; use the verified ZIP archive.\n' +
                  report['bundle_verification_error'])
        (destination/'build.json').write_text(json.dumps(report, indent=2) + '\n')
    print(archive)
    print(bundle)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--arch', choices=('native', 'x86_64', 'arm64', 'universal'), default='native')
    parser.add_argument('--jobs', type=int, default=6)
    parser.add_argument('--build-dir', type=Path)
    parser.add_argument('--deployment-target', default='11.0')
    args = parser.parse_args()
    host = platform.system()
    if host != 'Darwin':
        raise SystemExit('This packaging command currently targets macOS; use CMake for other SDL hosts.')
    architecture = platform.machine() if args.arch == 'native' else args.arch
    architectures = 'x86_64;arm64' if architecture == 'universal' else architecture
    directory = args.build_dir or ROOT/'build'/('macos-sdl3-' + architecture)
    config = ROOT/'.android-tools/build-host.json'
    configured = json.loads(config.read_text()) if config.exists() else {}
    cmake = os.environ.get('ENTISGLS_CMAKE', os.environ.get('STUDYSTEADY_CMAKE', configured.get('cmake', 'cmake')))
    run(cmake, '-S', ROOT, '-B', directory, '-DENTISGLS_LAUNCHER=ON',
        '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_OSX_ARCHITECTURES=' + architectures,
        '-DCMAKE_OSX_DEPLOYMENT_TARGET=' + args.deployment_target)
    run(cmake, '--build', directory, '--target', 'studysteady_sdl', '--parallel', args.jobs)
    source = directory/'EntisGLSLauncher.app'
    destination = ROOT/'artifacts/entisgls-launcher'/('macos-' + architecture)
    package_app(source, destination, architectures.split(';'), args.deployment_target)


if __name__ == '__main__':
    main()
