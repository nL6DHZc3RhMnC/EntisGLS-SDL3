#!/usr/bin/env python3
"""Launch the asset-free iOS library UI on an isolated iPhone simulator."""

import argparse
import json
from pathlib import Path
import plistlib
import subprocess
import time


def run(*arguments, timeout=180, check=True):
    return subprocess.run(['xcrun', 'simctl', *map(str, arguments)],
                          check=check, text=True, capture_output=True, timeout=timeout)


def version_tuple(value):
    parts = tuple(map(int, value.split('.')))
    return (parts + (0, 0, 0))[:3]


def select_device(inventory, sdk_version):
    sdk = version_tuple(sdk_version)
    runtimes = [item for item in inventory['runtimes']
                if item.get('isAvailable') and '.iOS-' in item['identifier']
                and version_tuple(item['version']) <= sdk]
    # Match the selected Xcode SDK rather than an unrelated newer Xcode's
    # installed runtime. A simulator model also has explicit OS version bounds.
    runtimes.sort(key=lambda item: version_tuple(item['version']), reverse=True)
    phones = {item['identifier']: item for item in inventory['devicetypes']
              if item['name'].startswith('iPhone')}
    for runtime in runtimes:
        major, minor, patch = version_tuple(runtime['version'])
        encoded = (major << 16) | (minor << 8) | patch
        paired = {item.get('deviceTypeIdentifier')
                  for item in inventory.get('devices', {}).get(runtime['identifier'], [])
                  if item.get('isAvailable')}
        supported = [phone for identifier, phone in phones.items()
                     if phone.get('minRuntimeVersion', 0) <= encoded <= phone.get('maxRuntimeVersion', 0xffffffff)
                     and (identifier in paired or ('minRuntimeVersion' in phone and 'maxRuntimeVersion' in phone))]
        if supported:
            phone = max(supported, key=lambda item: (
                item['identifier'] in paired, item.get('minRuntimeVersion', 0), item['identifier']))
            return runtime, phone
    raise RuntimeError(f'No compatible iPhone simulator/runtime for selected SDK {sdk_version}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--app', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    app, output = args.app.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    info = plistlib.loads((app / 'Info.plist').read_bytes())
    bundle = info['CFBundleIdentifier']
    if bundle != 'io.entisgls.launcher':
        raise RuntimeError('Unexpected simulator bundle ID')
    inventory = json.loads(run('list', '--json').stdout)
    sdk_version = subprocess.check_output(
        ['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-version'], text=True).strip()
    runtime, phone = select_device(inventory, sdk_version)
    # Create our own simulator with a supported model/runtime pair. Existing
    # user/runner devices are never reset or removed.
    device = None
    process = None
    result = {'passed': False, 'bundle_id': bundle, 'runtime': runtime['name'],
              'device_type': phone['name'], 'sdk_version': sdk_version, 'commercial_game_resources': False,
              'scope': 'library UI startup only; physical-device gameplay is not tested'}
    try:
        device = run('create', 'EntisGLS CI startup', phone['identifier'], runtime['identifier']).stdout.strip()
        run('boot', device)
        run('bootstatus', device, '-b', timeout=300)
        run('install', device, app)
        log = output / 'launcher.log'
        with log.open('w') as stream:
            process = subprocess.Popen([
                'xcrun', 'simctl', 'launch', '--console', '--terminate-running-process',
                device, bundle, '--library-smoke', '--exit-after', '15'],
                stdout=stream, stderr=subprocess.STDOUT, text=True)
            time.sleep(5)
            if process.poll() is not None:
                raise RuntimeError('The simulator launcher exited before the UI screenshot')
            run('io', device, 'screenshot', output / 'library.png')
            returncode = process.wait(timeout=60)
        text = log.read_text(errors='replace')
        result['launcher_exit_code'] = returncode
        result['library_ready'] = 'IOS_LIBRARY_READY' in text
        if returncode or not result['library_ready']:
            raise RuntimeError(f'iOS library smoke failed: {text[-3000:]}')
        result['passed'] = True
    except Exception as error:
        result['error'] = str(error)
        raise
    finally:
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        if device:
            run('shutdown', device, check=False)
            run('delete', device, check=False)
        (output / 'smoke.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
