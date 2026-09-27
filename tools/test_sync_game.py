#!/usr/bin/env python3
"""Exercise transfer/rollback logic locally; no adb process or phone is used."""
import contextlib
import io
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import sync_game as sync


REAL_RUN = subprocess.run


class LocalAdb:
    """Run the emitted shell commands against a temporary local filesystem."""
    def __init__(self):
        self.commands = []
        self.push_count = 0
        self.push_error = False
        self.corrupt_push = False
        self.move_error = None

    def __call__(self, command, **kwargs):
        assert command[0] == 'fake-adb', 'A real adb call is forbidden in this test'
        self.commands.append(command[1:])
        if command[1] == 'push':
            assert command[2] == '-Z'
            source, destination = map(Path, command[3:])
            self.push_count += 1
            if self.push_error:
                destination.write_bytes(b'partial transfer')
                raise subprocess.CalledProcessError(1, command)
            shutil.copyfile(source, destination)
            if self.corrupt_push:
                destination.write_bytes(b'incorrect transfer')
            return subprocess.CompletedProcess(command, 0)
        assert command[1] == 'shell'
        script = command[2]
        # A portable test substitute for Android toybox sha256sum.
        words = shlex.split(script)
        if words[0] == 'sha256sum':
            path = Path(words[1])
            stdout = sync.sha256(path) + '  ' + str(path) + '\n' if path.is_file() else ''
            return subprocess.CompletedProcess(command, 0 if stdout else 1, stdout, '')
        if 'mv -n ' in script and self.move_error:
            self.move_error(script)
        return REAL_RUN(['/bin/sh', '-c', script], **kwargs)


class SyncGameTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        # Exercise shell quoting without ever executing the metacharacters.
        self.game = self.root / "phone's game; directory"
        self.game.mkdir()
        self.source = self.root / 'original.noa'
        self.source.write_bytes(b'original game data')
        self.destination = self.game / "small's pack.noa"
        self.destination.write_bytes(b'previous content')
        self.old_inode = self.destination.stat().st_ino
        self.adb = LocalAdb()
        self.record = {}
        self.output = io.StringIO()
        self.stack = contextlib.ExitStack()
        self.addCleanup(self.stack.close)
        self.stack.enter_context(patch.object(sync.subprocess, 'run', self.adb))
        self.stack.enter_context(contextlib.redirect_stdout(self.output))

    def repair(self):
        sync.repair_copy(['fake-adb'], self.source, str(self.destination),
                         sync.sha256(self.source), self.record)

    def assert_old_unchanged(self):
        self.assertEqual(self.destination.read_bytes(), b'previous content')
        self.assertEqual(self.destination.stat().st_ino, self.old_inode)

    def test_success_fresh_inode_old_backup_and_no_staging_left(self):
        self.repair()
        self.assertEqual(self.destination.read_bytes(), self.source.read_bytes())
        self.assertNotEqual(self.destination.stat().st_ino, self.old_inode)
        backup = Path(self.record['backup_path'])
        self.assertEqual(backup.read_bytes(), b'previous content')
        self.assertEqual(backup.stat().st_ino, self.old_inode)
        self.assertFalse(Path(self.record['temporary_path']).exists())
        self.assertTrue(self.record['copied'])
        moves = [i for i, c in enumerate(self.adb.commands) if c[0] == 'shell' and 'mv -n ' in c[1]]
        pushes = [i for i, c in enumerate(self.adb.commands) if c[0] == 'push']
        hashes = [i for i, c in enumerate(self.adb.commands) if c[0] == 'shell' and c[1].startswith('sha256sum ')]
        self.assertLess(pushes[0], hashes[0])
        self.assertLess(hashes[0], moves[0])

    def test_equal_content_still_recreates_inode(self):
        self.destination.write_bytes(self.source.read_bytes())
        self.repair()
        self.assertEqual(self.adb.push_count, 1)
        self.assertNotEqual(self.destination.stat().st_ino, self.old_inode)

    def test_push_failure_does_not_move_original(self):
        self.adb.push_error = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.repair()
        self.assert_old_unchanged()
        self.assertIsNone(self.record['backup_path'])
        self.assertTrue(Path(self.record['temporary_path']).exists())

    def test_staging_checksum_failure_does_not_move_original(self):
        self.adb.corrupt_push = True
        with self.assertRaisesRegex(RuntimeError, 'staging'):
            self.repair()
        self.assert_old_unchanged()
        self.assertIsNone(self.record['backup_path'])

    def test_backup_move_failure_does_not_replace_original(self):
        def fail(_):
            raise subprocess.CalledProcessError(1, 'backup-move')
        self.adb.move_error = fail
        with self.assertRaises(subprocess.CalledProcessError):
            self.repair()
        self.assert_old_unchanged()
        self.assertTrue(Path(self.record['temporary_path']).exists())

    def test_publication_failure_restores_original(self):
        def fail(script):
            if '.import-' in script:
                raise subprocess.CalledProcessError(1, 'publication')
        self.adb.move_error = fail
        with self.assertRaises(subprocess.CalledProcessError):
            self.repair()
        self.assert_old_unchanged()
        self.assertTrue(self.record['original_restored'])
        self.assertTrue(Path(self.record['temporary_path']).exists())

    def test_recovery_failure_preserves_backup(self):
        moves = 0
        def fail(_):
            nonlocal moves
            moves += 1
            if moves > 1:
                raise subprocess.CalledProcessError(1, 'connection lost')
        self.adb.move_error = fail
        with self.assertRaises(subprocess.CalledProcessError):
            self.repair()
        self.assertFalse(self.destination.exists())
        self.assertEqual(Path(self.record['backup_path']).read_bytes(), b'previous content')
        self.assertIn('Recovery could not finish', self.record['recovery_note'])

    def test_unexpected_destination_is_not_overwritten(self):
        def inject(script):
            if '.import-' in script:
                self.destination.write_bytes(b'unexpected new data')
        self.adb.move_error = inject
        with self.assertRaises(subprocess.CalledProcessError):
            self.repair()
        self.assertEqual(self.destination.read_bytes(), b'unexpected new data')
        self.assertEqual(Path(self.record['backup_path']).read_bytes(), b'previous content')
        self.assertIn('not overwritten', self.record['recovery_note'])

    def test_adb_disconnect_is_not_interpreted_as_missing_file(self):
        failure = subprocess.CompletedProcess(['fake-adb'], 1, '', 'device disconnected')
        with patch.object(sync.subprocess, 'run', return_value=failure):
            with self.assertRaisesRegex(RuntimeError, 'device disconnected'):
                sync.remote_exists(['fake-adb'], str(self.destination))
        self.assert_old_unchanged()

    def run_main(self, extra=()):
        source_dir = self.root / 'StudySteadyR18'
        source_dir.mkdir(exist_ok=True)
        shutil.copyfile(self.source, source_dir / self.destination.name)
        report = self.root / 'custom reports' / 'sync.json'
        # Strip the tool's configured adb binary and serial before our fake.
        def routed(command, **kwargs):
            return self.adb(['fake-adb'] + command[3:], **kwargs)
        with patch.object(sync, 'ROOT', self.root), patch.object(sync, 'REMOTE', str(self.game)), \
             patch.object(sync.subprocess, 'run', routed), \
             patch('sys.argv', ['sync_game.py', '--serial', 'mock-device', '--report', str(report), *extra]):
            sync.main()
        return report

    def test_default_matching_hash_still_skips_and_custom_report_states_limit(self):
        self.destination.write_bytes(self.source.read_bytes())
        report = json.loads(self.run_main().read_text())
        self.assertEqual(self.adb.push_count, 0)
        self.assertEqual(self.destination.stat().st_ino, self.old_inode)
        self.assertFalse(report['files'][0]['copied'])
        self.assertFalse(report['app_read_access_verified'])
        self.assertEqual(report['status'], 'shell_hash_verified')

    def test_main_failure_writes_recovery_paths_to_requested_report(self):
        self.adb.corrupt_push = True
        with self.assertRaises(SystemExit):
            self.run_main(['--repair-permissions'])
        report = json.loads((self.root / 'custom reports' / 'sync.json').read_text())
        self.assertEqual(report['status'], 'failed')
        self.assertEqual(report['files'][0]['status'], 'failed')
        self.assertIn('.import-', report['files'][0]['temporary_path'])
        self.assert_old_unchanged()


if __name__ == '__main__':
    unittest.main()
