#!/usr/bin/env python3
"""iOS package validation reused by the unified release; legacy standalone CLI."""

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
from pathlib import Path, PurePosixPath
import plistlib
import re
import struct
import subprocess
import zipfile

IPA_NAME = 'EntisGLSLauncher-ios-arm64-unsigned.ipa'
REPORT_NAME = 'EntisGLSLauncher-ios-arm64-unsigned.build.json'
BUNDLE = 'Payload/EntisGLSLauncher.app/'


def verify_ipa(path):
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise RuntimeError('Duplicate IPA entries')
        for name in names:
            parts = PurePosixPath(name).parts
            if (name.startswith('/') or '..' in parts or '\\' in name
                    or not (name.startswith(BUNDLE) or name == 'Payload/')):
                raise RuntimeError(f'Unexpected IPA entry: {name}')
            if '_CodeSignature' in parts or name.endswith('/embedded.mobileprovision'):
                raise RuntimeError('The release IPA must not contain signing or provisioning data')
        if archive.testzip():
            raise RuntimeError('IPA CRC verification failed')
        info = plistlib.loads(archive.read(BUNDLE + 'Info.plist'))
        if info['CFBundleIdentifier'] != 'io.entisgls.launcher':
            raise RuntimeError('Unexpected iOS bundle ID')
        executable = info['CFBundleExecutable']
        if not executable or '/' in executable or executable in ('.', '..'):
            raise RuntimeError('Invalid bundle executable')
        with archive.open(BUNDLE + executable) as binary:
            header = binary.read(32)
            if len(header) != 32:
                raise RuntimeError('Missing Mach-O executable header')
            magic, cpu, _, kind, count, length, _, _ = struct.unpack('<8I', header)
            if (magic, cpu, kind) != (0xfeedfacf, 0x100000c, 2) or length > 1024 * 1024:
                raise RuntimeError('Expected an ARM64 Mach-O executable')
            commands = binary.read(length)
            offset, ios_device = 0, False
            for _ in range(count):
                if offset + 8 > len(commands):
                    raise RuntimeError('Truncated Mach-O load commands')
                command, size = struct.unpack_from('<II', commands, offset)
                if size < 8 or offset + size > len(commands):
                    raise RuntimeError('Invalid Mach-O load command size')
                if command == 0x1d:
                    raise RuntimeError('The device executable still contains a code signature')
                if command == 0x32 and size >= 24:
                    ios_device = struct.unpack_from('<I', commands, offset + 8)[0] == 2
                    if not ios_device:
                        raise RuntimeError('IPA contains a non-device executable (such as a simulator build)')
                if command == 0x25:
                    ios_device = True
                offset += size
            if not ios_device or offset != length:
                raise RuntimeError('Missing or malformed iOS device load commands')
    return info


def prepare(directory, environment):
    expected = [IPA_NAME, REPORT_NAME]
    if sorted(path.name for path in directory.iterdir()) != sorted(expected):
        raise RuntimeError('Unexpected iOS release asset set')
    for name in expected:
        path = directory / name
        if path.is_symlink() or not path.is_file() or not path.stat().st_size:
            raise RuntimeError(f'Invalid release asset: {name}')
    report = json.loads((directory / REPORT_NAME).read_text())
    if (report['platform'] != 'iOS' or report['sdk'] != 'iphoneos'
            or report['architectures'] != ['arm64'] or report['unsigned'] is not True
            or report['archive_crc_verified'] is not True
            or report['game_resources_included'] is not False
            or report['bundle_id'] != 'io.entisgls.launcher'):
        raise RuntimeError('iOS package validation failed')
    if report['archive_sha256'] != hashlib.sha256((directory / IPA_NAME).read_bytes()).hexdigest():
        raise RuntimeError('iOS IPA hash mismatch')
    info = verify_ipa(directory / IPA_NAME)
    sha = environment['GITHUB_SHA']
    if not re.fullmatch('[0-9a-f]{40}', sha):
        raise RuntimeError('Invalid source commit')
    tag = f"ios-dev-{environment['GITHUB_RUN_NUMBER']}.{environment['GITHUB_RUN_ATTEMPT']}-{sha[:8]}"
    run_url = (f"{environment['GITHUB_SERVER_URL']}/{environment['GITHUB_REPOSITORY']}"
               f"/actions/runs/{environment['GITHUB_RUN_ID']}")
    notes = '\n'.join([
        f'Experimental unsigned iOS build from `{sha}`, built by [GitHub Actions]({run_url}).', '',
        f"- ARM64 iPhone/iPad, minimum iOS {info.get('MinimumOSVersion', report.get('minimum_os', '13.0'))}.",
        '- This IPA is unsigned. Sign it locally with your own Apple development identity and device provisioning profile before installation.',
        '- This packaging helper does not verify simulator startup or gameplay on physical devices.',
        '- Import a supported game folder through Files, or copy it into the app Documents/Games folder. Commercial game resources are not included.',
        '- No Apple account, signing certificate, or provisioning profile is used by this workflow.', '',
    ])
    checksum_file = directory / 'SHA256SUMS.txt'
    checksum_file.write_text(''.join(
        f'{hashlib.sha256((directory / name).read_bytes()).hexdigest()}  {name}\n'
        for name in sorted(expected)))
    return tag, notes, [directory / name for name in expected] + [checksum_file]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts', type=Path, required=True)
    args = parser.parse_args()
    directory = args.artifacts.resolve()
    tag, notes, assets = prepare(directory, os.environ)
    notes_path = directory.parent / 'ios-release-notes.md'
    notes_path.write_text(notes)
    repository = os.environ['GITHUB_REPOSITORY']
    subprocess.run(['gh', 'release', 'create', tag, '--repo', repository, '--draft',
                    '--prerelease', '--latest=false', '--target', os.environ['GITHUB_SHA'],
                    '--title', f'EntisGLS Launcher {tag}', '--notes-file', str(notes_path),
                    *map(str, assets)], check=True)
    subprocess.run(['gh', 'release', 'edit', tag, '--repo', repository,
                    '--draft=false', '--latest=false'], check=True)


if __name__ == '__main__':
    main()
