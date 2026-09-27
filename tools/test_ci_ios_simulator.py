"""Prevent unsupported model/runtime pairs on runners containing several Xcodes."""

import unittest
from pathlib import Path
import struct
import tempfile
import zlib
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
    def fixture(self, path, *, black=False, reversed_colors=False, alpha=True):
        width, height = 80, 100
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


if __name__ == '__main__':
    unittest.main()
