"""Prevent unsupported model/runtime pairs on runners containing several Xcodes."""

import unittest
from ci_ios_simulator_smoke import select_device


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


if __name__ == '__main__':
    unittest.main()
