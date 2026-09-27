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
    runtimes = [item for item in inventory['runtimes']
                if item.get('isAvailable') and '.iOS-' in item['identifier']]
    if not runtimes:
        raise RuntimeError('No installed iOS Simulator runtime on this runner')
    runtime = max(runtimes, key=lambda item: tuple(map(int, item['version'].split('.'))))
    phones = [item for item in inventory['devicetypes']
              if item['name'].startswith('iPhone')]
    if not phones:
        raise RuntimeError('No iPhone simulator device type is available')
    # A recent iPhone device type with the newest installed iOS runtime. This
    # creates a disposable simulator; no existing simulator is reset or removed.
    phone = phones[-1]
    device = None
    process = None
    result = {'passed': False, 'bundle_id': bundle, 'runtime': runtime['name'],
              'device_type': phone['name'], 'commercial_game_resources': False,
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
