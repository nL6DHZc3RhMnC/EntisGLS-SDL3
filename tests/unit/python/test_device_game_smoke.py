#!/usr/bin/env python3
"""Local regression tests; all adb commands are mocked, no phone is touched."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import contextlib
import io
from pathlib import Path
import tempfile
import unittest
import subprocess
from unittest.mock import patch

import device_game_smoke as smoke

# Based on Android 13 Mi 10 Pro live dumps, with the launcher package identity.
PROCESS = '''ACTIVITY MANAGER RUNNING PROCESSES (dumpsys activity processes)
  All known processes:
  *APP* UID 10391 ProcessRecord{213446b 32376:io.entisgls.launcher/u0a391}
    pid=32376
    foregroundActivities=true (rep=true)
     mCrashing=false null mNotResponding=true [com.android.server.am.AppNotRespondingDialog@2da5fdc] bad=false
  Process LRU list:
'''
WINDOW = '''WINDOW MANAGER WINDOWS (dumpsys window windows)
  Window #14 Window{cd781d4 u0 Application Not Responding: io.entisgls.launcher}:
    mOwnerUid=1000 showForAllUsers=true package=android appop=SYSTEM_ALERT_WINDOW
    mAttrs={(0,0)(fillxfill) ty=SYSTEM_ALERT}
    mHasSurface=true isReadyForDisplay()=true mWindowRemovalAllowed=false
    WindowStateAnimator{70b47b4 Application Not Responding: io.entisgls.launcher}:
      Surface: shown=true layer=0 alpha=1.0
    isOnScreen=true
    isVisible=true
    mRemoveOnExit=false
  Window #17 Window{b6687b8 u0 io.entisgls.launcher/io.entisgls.launcher.GameActivity}:
    isVisible=true
'''
HEALTHY = PROCESS.replace('mNotResponding=true', 'mNotResponding=false')
HISTORY = '''ACTIVITY MANAGER LAST ANR
Process: io.entisgls.launcher
PID: 32376
Subject: Input dispatching timed out
Application Not Responding: io.entisgls.launcher
mNotResponding=true
'''


class AndroidHealthTest(unittest.TestCase):
    def check(self, processes, windows='', pid='32376'):
        return smoke.android_health_failure(pid, processes, windows)

    def test_current_process_anr_despite_foreground(self):
        reason, evidence = self.check(PROCESS)
        self.assertIn('current game process 32376', reason)
        self.assertIn('mNotResponding=true', evidence)

    def test_visible_modal_even_without_process_flag(self):
        reason, evidence = self.check(HEALTHY, WINDOW)
        self.assertIn('dialog is currently visible', reason)
        self.assertIn('Surface: shown=true', evidence)

    def test_old_anr_and_previous_pid_do_not_fail_cold_start(self):
        self.assertIsNone(self.check(PROCESS + HISTORY, '', pid='40000'))
        self.assertIsNone(self.check(HEALTHY + HISTORY, HISTORY))

    def test_other_app_anr_cannot_leak_into_current_record(self):
        other = PROCESS.replace(smoke.PACKAGE, 'example.other')
        self.assertIsNone(self.check(other + HEALTHY, WINDOW.replace(smoke.PACKAGE, 'example.other')))
        self.assertIsNone(self.check(HEALTHY, WINDOW.replace(smoke.PACKAGE, smoke.PACKAGE + '.other')))

    def test_hidden_or_removed_old_window_is_not_active(self):
        for text in [WINDOW.replace('shown=true', 'shown=false'),
                     WINDOW.replace('mHasSurface=true', 'mHasSurface=false'),
                     WINDOW.replace('isVisible=true', 'isVisible=false').replace('isOnScreen=true', 'isOnScreen=false'),
                     WINDOW.replace('mRemoveOnExit=false', 'mRemoveOnExit=true')]:
            with self.subTest(window=text):
                self.assertIsNone(self.check(HEALTHY, text))

    def test_normal_in_app_dialog_is_not_anr(self):
        self.assertIsNone(self.check(HEALTHY, WINDOW.replace('Application Not Responding: ', 'Save file: ')))

    def run_observe(self, directory, processes, windows):
        calls = []
        def adb(command, **kwargs):
            words = command[3:]
            calls.append(words)
            if words == ['shell', 'pidof', smoke.PACKAGE]:
                return b'32376\n'
            if words == ['shell', 'dumpsys', 'activity', 'processes', smoke.PACKAGE]:
                return processes.encode()
            if words == ['shell', 'dumpsys', 'window', 'windows']:
                return windows.encode()
            if words == ['shell', 'dumpsys', 'activity', 'activities']:
                return ('mResumedActivity: ' + smoke.PACKAGE + '/.GameActivity').encode()
            if words[0] == 'logcat' and '-d' in words:
                return b'app log preserved\n'
            if words == ['exec-out', 'screencap', '-p']:
                return b'mocked screenshot'
            raise AssertionError('Unexpected or mutating adb command: ' + repr(words))
        output = Path(directory) / 'observe'
        arguments = ['device_game_smoke.py', '--serial', 'fixture', '--flow', 'observe', '--output', str(output)]
        console = io.StringIO()
        with patch.object(smoke.subprocess, 'check_output', side_effect=adb), patch('sys.argv', arguments), contextlib.redirect_stdout(console):
            try:
                smoke.main()
            except RuntimeError as error:
                return output, calls, console.getvalue(), str(error)
        return output, calls, console.getvalue(), None

    def test_observe_stops_without_input_capture_or_completed_on_anr(self):
        with tempfile.TemporaryDirectory() as folder:
            output, calls, console, failure = self.run_observe(folder, PROCESS, WINDOW)
            self.assertIn('ANR', failure)
            self.assertNotIn('Flow completed', console)
            self.assertFalse(output.with_suffix('.png').exists())
            self.assertIn('mNotResponding=true', output.with_suffix('.health.txt').read_text())
            self.assertEqual(output.with_suffix('.txt').read_bytes(), b'app log preserved\n')
            self.assertFalse(any('input' in call or '-c' in call or 'screencap' in call or 'am' in call for call in calls))

    def test_observe_healthy_after_historical_anr_can_complete(self):
        with tempfile.TemporaryDirectory() as folder:
            output, _, console, failure = self.run_observe(folder, HEALTHY + HISTORY, HISTORY)
            self.assertIsNone(failure)
            self.assertIn('Flow completed', console)
            self.assertTrue(output.with_suffix('.png').exists())
            self.assertFalse(output.with_suffix('.health.txt').exists())

    def test_disconnect_during_log_does_not_mask_primary_anr_or_replace_log(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / 'disconnected'
            output.with_suffix('.txt').write_text('previous diagnostic\n')
            def adb(command, **kwargs):
                self.assertGreater(kwargs['timeout'], 0)
                words = command[3:]
                if words == ['shell', 'pidof', smoke.PACKAGE]:
                    return b'32376\n'
                if words == ['shell', 'dumpsys', 'activity', 'processes', smoke.PACKAGE]:
                    return PROCESS.encode()
                if words == ['shell', 'dumpsys', 'window', 'windows']:
                    return WINDOW.encode()
                if words[0] == 'logcat':
                    raise subprocess.TimeoutExpired(command, kwargs['timeout'])
                raise AssertionError('Unexpected command: ' + repr(words))
            argv = ['device_game_smoke.py', '--serial', 'fixture', '--flow', 'observe',
                    '--output', str(output)]
            console = io.StringIO()
            with patch.object(smoke.subprocess, 'check_output', side_effect=adb), \
                 patch('sys.argv', argv), contextlib.redirect_stdout(console):
                with self.assertRaisesRegex(RuntimeError, 'ANR'):
                    smoke.main()
            self.assertEqual(output.with_suffix('.txt').read_text(), 'previous diagnostic\n')
            self.assertTrue(output.with_suffix('.log-error.txt').exists())
            self.assertTrue(output.with_suffix('.health.txt').exists())
            self.assertFalse(output.with_suffix('.png').exists())
            self.assertNotIn('Flow completed', console.getvalue())


if __name__ == '__main__':
    unittest.main()
