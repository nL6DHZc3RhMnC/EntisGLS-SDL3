#!/usr/bin/env python3
"""Copy original NOA packs into the development app, without altering saves.

Requires the APK to have been installed and opened once. Copies only NOA packs;
Windows binaries are not used. Existing destination files are checked by SHA256.
--repair-permissions recreates selected packs through fresh files, retaining the
old files as uniquely named backups. Hash checks run as adb shell; they do not
prove that the game process can read the resources. Launch the app afterwards.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import uuid

ROOT = Path(__file__).resolve().parents[1]
REMOTE = '/sdcard/Android/data/io.studysteady.port/files/game'


def sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(8 * 1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def remote_hash(adb, destination):
    result = subprocess.run(adb + ['shell', 'sha256sum ' + shlex.quote(destination)],
                            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
    return result.stdout.split()[0] if result.returncode == 0 and result.stdout.strip() else None


def remote_exists(adb, path):
    quoted = shlex.quote(path)
    command = f'if [ -e {quoted} ] || [ -L {quoted} ]; then echo present; else echo absent; fi'
    result = subprocess.run(adb + ['shell', command],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    state = result.stdout.strip()
    if result.returncode != 0 or state not in ('present', 'absent'):
        raise RuntimeError(f'Cannot inspect remote path {path}: {result.stderr.strip()}')
    return state == 'present'


def remote_move_new(adb, source, destination):
    # mv -n alone may report success without moving anything. Check both ends;
    # never replace a destination unexpectedly created during a transfer.
    src, dst = shlex.quote(source), shlex.quote(destination)
    command = (f'[ ! -e {dst} ] && [ ! -L {dst} ] && '
               f'mv -n {src} {dst} && '
               f'[ ! -e {src} ] && [ ! -L {src} ] && '
               f'( [ -e {dst} ] || [ -L {dst} ] )')
    subprocess.run(adb + ['shell', command], check=True)


def repair_copy(adb, path, destination, digest, record, checkpoint=lambda: None):
    token = uuid.uuid4().hex
    parent, name = destination.rsplit('/', 1)
    backup = f'{parent}/.{name}.backup-{token}'
    temporary = f'{parent}/.{name}.import-{token}'
    record.update(backup_path=None, temporary_path=temporary,
                  original_restored=False, copied=False)
    checkpoint()
    # A collision must abort before touching any file.
    if remote_exists(adb, backup) or remote_exists(adb, temporary):
        raise RuntimeError(f'Repair staging path already exists for {path.name}')
    try:
        # Keep the current path intact during the long transfer and checksum.
        subprocess.run(adb + ['push', '-Z', str(path), temporary], check=True)
        if remote_hash(adb, temporary) != digest:
            raise RuntimeError(f'SHA256 verification failed for staging file: {path.name}')
        if remote_exists(adb, destination):
            # Record the location before moving: a lost adb response must not
            # hide a backup which was actually created on the device.
            record['backup_path'] = backup
            checkpoint()
            remote_move_new(adb, destination, backup)
            print(f'Original retained at {backup}', flush=True)
        remote_move_new(adb, temporary, destination)
        if remote_hash(adb, destination) != digest:
            raise RuntimeError(f'SHA256 verification failed after replacement: {path.name}')
        record['copied'] = True
    except (Exception, KeyboardInterrupt):
        if record['backup_path']:
            try:
                if remote_exists(adb, backup):
                    if not remote_exists(adb, destination):
                        remote_move_new(adb, backup, destination)
                        record['original_restored'] = True
                        print(f'Original restored: {destination}', flush=True)
                    else:
                        record['recovery_note'] = 'Original retained as backup; destination was not overwritten during recovery.'
                else:
                    record['recovery_note'] = 'No backup found; inspect the original destination.'
            except (Exception, KeyboardInterrupt) as recovery_error:
                record['recovery_note'] = f'Recovery could not finish; retain backup at {backup}: {recovery_error}'
        # Keep an incomplete staging file for diagnosis; never delete backups.
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--serial', required=True, help='Authorized adb device serial')
    parser.add_argument('--repair-permissions', action='store_true',
                        help='Force fresh files even when hashes match; keep old files as backups')
    parser.add_argument('--report', type=Path, default=ROOT / 'artifacts/game-sync.json',
                        help='JSON report path (default: artifacts/game-sync.json)')
    parser.add_argument('packs', nargs='*', help='Pack basenames; default: all NOA packs')
    args = parser.parse_args()
    adb = [str(ROOT / '.android-tools/platform-tools/platform-tools/adb'), '-s', args.serial]
    game = ROOT / 'StudySteadyR18'
    packs = [game / name for name in args.packs] if args.packs else list(game.glob('*.noa'))
    if not packs or any(p.parent != game or p.suffix != '.noa' or not p.is_file() for p in packs):
        parser.error('Only existing NOA pack basenames in StudySteadyR18 are allowed')
    # Make the small startup packs available first.
    packs.sort(key=lambda p: p.stat().st_size)
    records = []
    report = {'serial': args.serial, 'files': records,
              'repair_permissions': args.repair_permissions,
              'verification_scope': 'SHA256 via adb shell only; game-process access must be verified by launching the app.',
              'app_read_access_verified': False, 'status': 'in_progress'}
    output = args.report
    output.parent.mkdir(parents=True, exist_ok=True)
    def write_report():
        output.write_text(json.dumps(report, indent=2) + '\n')
    write_report()
    try:
        subprocess.run(adb + ['shell', 'mkdir -p ' + shlex.quote(REMOTE)], check=True)
        for index, path in enumerate(packs, 1):
            print(f'[{index}/{len(packs)}] {path.name}: checking SHA256', flush=True)
            digest = sha256(path)
            destination = REMOTE + '/' + path.name
            record = {'name': path.name, 'size': path.stat().st_size, 'sha256': digest,
                      'destination': destination, 'copied': False, 'status': 'in_progress'}
            records.append(record)
            if args.repair_permissions:
                repair_copy(adb, path, destination, digest, record, write_report)
            else:
                copied = remote_hash(adb, destination) != digest
                if copied:
                    print(f'Copying {path.stat().st_size:,} bytes', flush=True)
                    subprocess.run(adb + ['push', '-Z', str(path), destination], check=True)
                    if remote_hash(adb, destination) != digest:
                        raise RuntimeError(f'SHA256 verification failed: {path.name}')
                record['copied'] = copied
            record['status'] = 'shell_hash_verified'
            print(f'{path.name}: shell SHA256 verified' +
                  (' (already present)' if not record['copied'] else ''), flush=True)
            write_report()
        report['status'] = 'shell_hash_verified'
    except (Exception, KeyboardInterrupt) as error:
        report['status'] = 'failed'
        report['error'] = str(error) or type(error).__name__
        if records and records[-1]['status'] == 'in_progress':
            records[-1]['status'] = 'failed'
        raise SystemExit(f'Sync failed: {report["error"]}. Recovery details: {output}') from error
    finally:
        write_report()
    print(f'Verified {len(records)} packs via adb shell; report: {output}', flush=True)
    print('Launch the actual game to verify app access; shell SHA256 does not prove app readability.', flush=True)


if __name__ == '__main__':
    main()
