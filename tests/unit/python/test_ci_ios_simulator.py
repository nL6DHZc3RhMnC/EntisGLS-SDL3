"""Prevent unsupported model/runtime pairs on runners containing several Xcodes."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


import unittest
from unittest import mock
import json
from pathlib import Path
import plistlib
import struct
import subprocess
import tempfile
import zlib
import ci_ios_simulator_smoke as smoke
from ci_ios_simulator_smoke import select_device, verify_presented_pattern


class SimulatorSelectionTest(unittest.TestCase):
    def inventory(self):
        return {
            'runtimes': [
                dict(identifier='com.apple.CoreSimulator.SimRuntime.iOS-18-5', version='18.5', name='iOS 18.5', isAvailable=True),
                dict(identifier='com.apple.CoreSimulator.SimRuntime.iOS-26-2', version='26.2', name='iOS 26.2', isAvailable=True)],
            'devicetypes': [
                dict(identifier='phone16', name='iPhone 16', minRuntimeVersion=18 << 16, maxRuntimeVersion=0xffffffff),
                dict(identifier='phone6s', name='iPhone 6s Plus', minRuntimeVersion=9 << 16, maxRuntimeVersion=(15 << 16) | 0xffff)],
            'devices': {},
        }

    def test_matches_xcode_sdk_and_skips_old_last_model(self):
        runtime, model = select_device(self.inventory(), '18.5')
        self.assertEqual(runtime['version'], '18.5')
        self.assertEqual(model['name'], 'iPhone 16')

    def test_uses_available_existing_pair_without_assuming_list_order(self):
        inventory = self.inventory()
        inventory['devicetypes'].append(dict(identifier='paired', name='iPhone 15'))
        inventory['devices'][inventory['runtimes'][0]['identifier']] = [
            dict(deviceTypeIdentifier='paired', isAvailable=True)]
        self.assertEqual(select_device(inventory, '18.5.0')[1]['identifier'], 'paired')

    def test_rejects_inventory_with_no_supported_pair(self):
        inventory = self.inventory()
        inventory['devicetypes'] = [inventory['devicetypes'][-1]]
        with self.assertRaisesRegex(RuntimeError, 'No compatible'):
            select_device(inventory, '18.5')

    def test_ignores_unavailable_and_newer_runtimes(self):
        inventory = self.inventory()
        inventory['runtimes'][0]['isAvailable'] = False
        with self.assertRaisesRegex(RuntimeError, 'No compatible'):
            select_device(inventory, '18.5')


class PresentationScreenshotTest(unittest.TestCase):
    def fixture(self, path, *, black=False, reversed_colors=False, alpha=True, portrait=False):
        width, height = (80, 100) if portrait else (100, 80)
        channels = 4 if alpha else 3
        pixels = bytearray()
        previous = bytearray(width * channels)
        for y in range(height):
            row = bytearray()
            for x in range(width):
                green = (x < width // 2) != reversed_colors
                rgb = (0, 0, 0) if black else (0, 255, 0) if green else (255, 0, 0)
                row.extend((*rgb, 255) if alpha else rgb)
            method = y % 5
            pixels.append(method)
            for x, value in enumerate(row):
                left = row[x - channels] if x >= channels else 0
                up = previous[x]
                corner = previous[x - channels] if x >= channels else 0
                if method == 0:
                    prediction = 0
                elif method == 1:
                    prediction = left
                elif method == 2:
                    prediction = up
                elif method == 3:
                    prediction = (left + up) // 2
                else:
                    estimate = left + up - corner
                    prediction = min((left, up, corner), key=lambda candidate: abs(estimate - candidate))
                pixels.append((value - prediction) & 255)
            previous = row

        def chunk(tag, data):
            return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff)

        path.write_bytes(b'\x89PNG\r\n\x1a\n' +
            chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 6 if alpha else 2, 0, 0, 0)) +
            chunk(b'IDAT', zlib.compress(pixels)) + chunk(b'IEND', b''))

    def test_accepts_presented_frame_in_rgb_and_rgba_with_all_filters(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            for alpha in (True, False):
                self.fixture(path, alpha=alpha)
                self.assertEqual(verify_presented_pattern(path)['green_left_ratio'], 1)

    def test_rejects_black_screen(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            self.fixture(path, black=True)
            with self.assertRaisesRegex(RuntimeError, 'did not present'):
                verify_presented_pattern(path)

    def test_rejects_landscape_content_presented_in_portrait(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            self.fixture(path, portrait=True)
            with self.assertRaisesRegex(RuntimeError, 'did not rotate'):
                verify_presented_pattern(path)

    def test_rejects_reversed_frame(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            self.fixture(path, reversed_colors=True)
            with self.assertRaisesRegex(RuntimeError, 'did not present'):
                verify_presented_pattern(path)

    def test_rejects_corrupted_screenshot(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            self.fixture(path)
            damaged = bytearray(path.read_bytes())
            damaged[-1] ^= 1
            path.write_bytes(damaged)
            with self.assertRaisesRegex(RuntimeError, 'checksum'):
                verify_presented_pattern(path)


class SimulatorDiagnosticsTest(unittest.TestCase):
    bundle = 'io.entisgls.launcher'

    def test_command_timeout_keeps_partial_streams_and_timing(self):
        with tempfile.TemporaryDirectory() as directory:
            diagnostics = smoke.Diagnostics(Path(directory))

            def stalled(argv, **options):
                options['stdout'].write('partial launch acknowledgement\n')
                options['stderr'].write('CoreSimulator is waiting\n')
                raise subprocess.TimeoutExpired(argv, options['timeout'])

            with mock.patch.object(smoke.subprocess, 'run', side_effect=stalled):
                with self.assertRaisesRegex(RuntimeError, 'timed out after 2s'):
                    diagnostics.simctl('launch', 'launch', 'device', self.bundle, timeout=2)
            report = json.loads((Path(directory) / 'smoke.json').read_text())
            command = report['commands'][0]
            self.assertEqual(command['status'], 'timeout')
            self.assertEqual(command['timeout_seconds'], 2)
            self.assertGreaterEqual(command['elapsed_seconds'], 0)
            self.assertIn('partial launch', (Path(directory) / command['stdout']).read_text())
            self.assertIn('CoreSimulator', (Path(directory) / command['stderr']).read_text())

    def test_nonzero_command_keeps_stderr_in_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            diagnostics = smoke.Diagnostics(Path(directory))

            def failed(argv, **options):
                options['stderr'].write('device cannot be booted')
                return subprocess.CompletedProcess(argv, 7)

            with mock.patch.object(smoke.subprocess, 'run', side_effect=failed):
                with self.assertRaisesRegex(RuntimeError, 'device cannot be booted'):
                    diagnostics.simctl('boot', 'boot', 'device')
            self.assertEqual(diagnostics.result['commands'][0]['returncode'], 7)

    def test_launch_requires_pid_for_the_requested_bundle(self):
        self.assertEqual(smoke.launch_pid(self.bundle + ': 1234\n', self.bundle), 1234)
        for text in ('', self.bundle + ': 0', 'another.app: 1234', self.bundle + ': -1'):
            with self.assertRaisesRegex(RuntimeError, 'did not return'):
                smoke.launch_pid(text, self.bundle)

    def test_marker_does_not_hide_app_death(self):
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / 'app.log'
            log.write_text('IOS_LIBRARY_READY\n')
            with mock.patch.object(smoke, 'pid_alive', return_value=False):
                with self.assertRaisesRegex(RuntimeError, 'PID 1234 exited'):
                    smoke.wait_for_marker(1234, [log], 'IOS_LIBRARY_READY')
            with mock.patch.object(smoke, 'pid_alive', return_value=True):
                smoke.wait_for_marker(1234, [log], 'IOS_LIBRARY_READY')

    def test_phase_keeps_marker_failure_when_terminate_also_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            diagnostics = smoke.Diagnostics(root / 'out')
            container = root / 'container'; container.mkdir()

            def commands(label, *arguments, **options):
                if arguments[0] == 'launch':
                    self.assertNotIn('--console', arguments)
                    self.assertNotIn('--terminate-running-process', arguments)
                    self.assertTrue(any(str(arg).startswith('--stdout=' + str(container)) for arg in arguments))
                    return subprocess.CompletedProcess(arguments, 0, self.bundle + ': 1234\n', '')
                raise RuntimeError('terminate timed out after 30s')

            with mock.patch.object(diagnostics, 'simctl', side_effect=commands), \
                    mock.patch.object(smoke, 'wait_for_marker', side_effect=RuntimeError('original marker timeout')), \
                    mock.patch.object(smoke, 'collect_failure') as collect:
                with self.assertRaisesRegex(RuntimeError, '^original marker timeout$'):
                    smoke.capture_phase(diagnostics, 'device', self.bundle, 'EntisGLSLauncher',
                                        container, 'library', [], 'IOS_LIBRARY_READY')
            self.assertEqual(diagnostics.result['error'], 'original marker timeout')
            self.assertIn('terminate timed out', diagnostics.result['cleanup_errors'][0]['error'])
            self.assertFalse(diagnostics.result['library']['passed'])
            self.assertEqual(diagnostics.result['library']['pid'], 1234)
            self.assertTrue((diagnostics.output / 'library.stdout.log').exists())
            self.assertTrue((diagnostics.output / 'library.stderr.log').exists())
            self.assertEqual(collect.call_args.args[-1], 1234)

    def test_successful_capture_with_failed_cleanup_is_not_a_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            diagnostics = smoke.Diagnostics(root / 'out')
            container = root / 'container'; container.mkdir()

            def commands(label, *arguments, **options):
                if arguments[0] == 'terminate':
                    raise RuntimeError('cleanup failed')
                return subprocess.CompletedProcess(arguments, 0,
                    self.bundle + ': 1234\n' if arguments[0] == 'launch' else '', '')

            with mock.patch.object(diagnostics, 'simctl', side_effect=commands), \
                    mock.patch.object(smoke, 'wait_for_marker'), \
                    mock.patch.object(smoke, 'pid_alive', return_value=True), \
                    mock.patch.object(smoke.time, 'sleep'), mock.patch.object(smoke, 'collect_failure'):
                with self.assertRaisesRegex(RuntimeError, 'cleanup failed'):
                    smoke.capture_phase(diagnostics, 'device', self.bundle, 'EntisGLSLauncher',
                                        container, 'library', [], 'IOS_LIBRARY_READY')
            self.assertTrue(diagnostics.result['library']['alive_after_capture'])
            self.assertFalse(diagnostics.result['library']['passed'])

    def test_early_input_failure_still_writes_report(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaises(FileNotFoundError):
                smoke.run_smoke(root / 'missing.app', root / 'output')
            report = json.loads((root / 'output/smoke.json').read_text())
            self.assertFalse(report['passed'])
            self.assertIn('Info.plist', report['error'])
            self.assertIn('finished_at', report)

    def test_shutdown_failure_still_deletes_device_and_reports_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            app = root / 'test.app'; app.mkdir()
            (app / 'Info.plist').write_bytes(plistlib.dumps({
                'CFBundleIdentifier': self.bundle, 'CFBundleExecutable': 'EntisGLSLauncher'}))
            container = root / 'container'; container.mkdir()
            inventory = SimulatorSelectionTest().inventory()
            calls = []

            def simctl(diagnostics, label, *arguments, **options):
                calls.append(arguments[0])
                if arguments[0] in ('shutdown', 'delete'):
                    self.assertTrue(diagnostics.result['checks_completed'])
                    self.assertFalse(diagnostics.result['passed'], 'PASS must wait until all cleanup succeeds')
                if arguments[0] == 'shutdown':
                    raise RuntimeError('shutdown timeout')
                output = {'list': json.dumps(inventory), 'help': '--stdout --stderr',
                          'create': 'owned-device', 'get_app_container': str(container)}.get(arguments[0], '')
                return subprocess.CompletedProcess(arguments, 0, output, '')

            def capture(diagnostics, device, bundle, executable, data, name, arguments, marker):
                diagnostics.result[name] = {'passed': True}

            with mock.patch.object(smoke.Diagnostics, 'simctl', autospec=True, side_effect=simctl), \
                    mock.patch.object(smoke.Diagnostics, 'command', return_value=subprocess.CompletedProcess([], 0, '18.5', '')), \
                    mock.patch.object(smoke, 'capture_phase', side_effect=capture), \
                    mock.patch.object(smoke, 'verify_presented_pattern', return_value={'orientation': 'landscape'}):
                with self.assertRaisesRegex(RuntimeError, 'shutdown timeout'):
                    smoke.run_smoke(app, root / 'output')
            self.assertEqual(calls[-2:], ['shutdown', 'delete'])
            report = json.loads((root / 'output/smoke.json').read_text())
            self.assertFalse(report['passed'])
            self.assertEqual(report['error'], 'shutdown timeout')
            self.assertEqual(report['device_id'], 'owned-device')


if __name__ == '__main__':
    unittest.main()
