#!/usr/bin/env python3
"""Publish the verified artifacts of one successful GitHub Actions run."""

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
import re
import subprocess

from ci_ios_release import IPA_NAME, REPORT_NAME, verify_ipa


def prepare(directory, environment):
    required = ['EntisGLSLauncher-android-arm64.apk',
                'EntisGLSLauncher-android-arm64.build.json', IPA_NAME, REPORT_NAME]
    for arch in ('x86_64', 'arm64'):
        required += [f'EntisGLSLauncher-macos-{arch}{suffix}'
                     for suffix in ('.zip', '.build.json', '.tests.json')]
    actual = sorted(path.name for path in directory.iterdir())
    if actual != sorted(required):
        raise RuntimeError(f'Unexpected release asset set: {actual}; expected {sorted(required)}')
    for name in required:
        path = directory / name
        if path.is_symlink() or not path.is_file() or path.stat().st_size == 0:
            raise RuntimeError(f'Invalid release asset: {name}')

    for arch in ('x86_64', 'arm64'):
        prefix = f'EntisGLSLauncher-macos-{arch}'
        report = json.loads((directory / (prefix + '.build.json')).read_text())
        if (report['architectures'] != [arch] or report['archive_signature_verified'] is not True
                or report['archive_crc_verified'] is not True or report['game_resources_included'] is not False):
            raise RuntimeError(f'macOS package validation failed: {arch}')
        if report['archive_sha256'] != hashlib.sha256((directory / (prefix + '.zip')).read_bytes()).hexdigest():
            raise RuntimeError(f'macOS archive hash mismatch: {arch}')
        if json.loads((directory / (prefix + '.tests.json')).read_text())['passed'] is not True:
            raise RuntimeError(f'macOS native checks failed: {arch}')
    android = json.loads((directory / 'EntisGLSLauncher-android-arm64.build.json').read_text())
    if (android['native_packaged'] is not True or android['elf_16k_compatible'] is not True
            or android['game_resources_bundled'] is not False):
        raise RuntimeError('Android package validation failed')
    if android['apk_sha256'] != hashlib.sha256((directory / required[0]).read_bytes()).hexdigest():
        raise RuntimeError('Android APK hash mismatch')

    ios = json.loads((directory / REPORT_NAME).read_text())
    if (ios['platform'] != 'iOS' or ios['sdk'] != 'iphoneos'
            or ios['architectures'] != ['arm64'] or ios['unsigned'] is not True
            or ios['archive_crc_verified'] is not True
            or ios['archive_binary_verified'] is not True
            or ios['macho_platform_verified'] is not True
            or ios['game_resources_included'] is not False
            or ios['bundle_id'] != 'io.entisgls.launcher'):
        raise RuntimeError('iOS package validation failed')
    if ios['archive_sha256'] != hashlib.sha256((directory / IPA_NAME).read_bytes()).hexdigest():
        raise RuntimeError('iOS IPA hash mismatch')
    ios_info = verify_ipa(directory / IPA_NAME)

    sha = environment['GITHUB_SHA']
    if not re.fullmatch(r'[0-9a-f]{40}', sha):
        raise RuntimeError('Invalid source commit')
    tagged = environment['GITHUB_REF_TYPE'] == 'tag'
    tag = environment['GITHUB_REF_NAME'] if tagged else (
        f"dev-{environment['GITHUB_RUN_NUMBER']}.{environment['GITHUB_RUN_ATTEMPT']}-{sha[:8]}")
    if tagged and not re.fullmatch(r'v[0-9][A-Za-z0-9.+-]*', tag):
        raise RuntimeError('Version tags must start with v followed by a version number')
    prerelease = not tagged or '-' in tag
    run_url = (f"{environment['GITHUB_SERVER_URL']}/{environment['GITHUB_REPOSITORY']}"
               f"/actions/runs/{environment['GITHUB_RUN_ID']}")
    notes = '\n'.join([
        f'Built by [GitHub Actions]({run_url}) from commit `{sha}`.', '',
        '- Android: ARM64, Android 10 or newer. Add a game folder to access it directly with persistent read/write permission; resources are not copied.',
        '- Saves use savedata inside the selected game directory on every platform. Missing directories are created; old app-private saves are not migrated.',
        '- macOS: separate Intel (x86_64) and Apple Silicon (arm64) ZIPs, macOS 11 or newer.',
        '- macOS apps have ad-hoc signatures; they are not Developer ID signed or notarized.',
        f"- iOS: experimental ARM64 iPhone/iPad IPA, iOS {ios_info.get('MinimumOSVersion', ios.get('minimum_os', '13.0'))} or newer.",
        '- The iOS IPA is unsigned. Sign it locally with your own Apple development identity and device provisioning profile before installation.',
        '- iOS checks cover the device build, package contents, Mach-O platform, Simulator game-library startup and GLES presentation screenshots. Physical-device gameplay is not tested by this workflow.',
        '- No Apple account, signing certificate, or provisioning profile is used by the iOS build.',
        '- Game scripts and resource archives are not included. Supported games must be supplied separately.',
        '- Native checks use synthetic fixtures; they do not certify compatibility with every game.',
        '', 'Package verification, native test reports and SHA-256 checksums are attached.', '',
    ])
    checksums = ''.join(f'{hashlib.sha256((directory / name).read_bytes()).hexdigest()}  {name}\n'
                        for name in sorted(required))
    (directory / 'SHA256SUMS.txt').write_text(checksums)
    return tag, prerelease, notes, [directory / name for name in sorted(required)] + [directory / 'SHA256SUMS.txt']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts', type=Path, required=True)
    args = parser.parse_args()
    directory = args.artifacts.resolve()
    tag, prerelease, notes, assets = prepare(directory, os.environ)
    notes_path = directory.parent / 'release-notes.md'
    notes_path.write_text(notes)
    repository = os.environ['GITHUB_REPOSITORY']
    command = ['gh', 'release', 'create', tag, '--repo', repository, '--draft',
               '--target', os.environ['GITHUB_SHA'], '--title', f'EntisGLS Launcher {tag}',
               '--notes-file', str(notes_path), '--latest=false']
    if os.environ['GITHUB_REF_TYPE'] == 'tag':
        command.append('--verify-tag')
    if prerelease:
        command.append('--prerelease')
    # A draft keeps partial uploads out of the public release list. Existing
    # releases are never overwritten; rerun development builds get a new tag.
    subprocess.run(command + [str(path) for path in assets], check=True)
    subprocess.run(['gh', 'release', 'edit', tag, '--repo', repository,
                    '--draft=false', '--latest=false' if prerelease else '--latest'], check=True)


if __name__ == '__main__':
    main()
