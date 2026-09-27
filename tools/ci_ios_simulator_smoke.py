#!/usr/bin/env python3
"""Check the asset-free iOS library and real GLES presentation on a simulator."""

import argparse
import json
from pathlib import Path
import plistlib
import struct
import subprocess
import time
import zlib


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


def png_rgb(path):
    """Decode simctl's non-interlaced RGB/RGBA PNG without extra dependencies."""
    source = Path(path).read_bytes()
    if not source.startswith(b'\x89PNG\r\n\x1a\n'):
        raise RuntimeError('The simulator screenshot is not a PNG')
    offset, compressed, header = 8, bytearray(), None
    while offset + 12 <= len(source):
        length, tag = struct.unpack_from('>I4s', source, offset)
        end = offset + 8 + length
        if end + 4 > len(source):
            raise RuntimeError('Truncated PNG chunk')
        data = source[offset + 8:end]
        if zlib.crc32(tag + data) & 0xffffffff != struct.unpack_from('>I', source, end)[0]:
            raise RuntimeError('Invalid PNG checksum')
        if tag == b'IHDR':
            header = struct.unpack('>IIBBBBB', data)
        elif tag == b'IDAT':
            compressed.extend(data)
        elif tag == b'IEND':
            break
        offset = end + 4
    if not header:
        raise RuntimeError('Missing PNG header')
    width, height, bits, color, compression, filtering, interlace = header
    if (not width or not height or width * height > 32 * 1024 * 1024 or
            bits != 8 or color not in (2, 6) or compression or filtering or interlace):
        raise RuntimeError(f'Unsupported simulator PNG format: {header}')
    channels = 3 if color == 2 else 4
    stride = width * channels
    decoder = zlib.decompressobj()
    raw = decoder.decompress(compressed, (stride + 1) * height + 1)
    if not decoder.eof or len(raw) != (stride + 1) * height:
        raise RuntimeError('Invalid PNG pixel data length')
    rows, previous = [], bytearray(stride)
    for y in range(height):
        start = y * (stride + 1)
        method = raw[start]
        row = bytearray(raw[start + 1:start + 1 + stride])
        if method > 4:
            raise RuntimeError('Unsupported PNG filter')
        for x in range(stride):
            left = row[x - channels] if x >= channels else 0
            up = previous[x]
            corner = previous[x - channels] if x >= channels else 0
            if method == 1:
                row[x] = (row[x] + left) & 255
            elif method == 2:
                row[x] = (row[x] + up) & 255
            elif method == 3:
                row[x] = (row[x] + (left + up) // 2) & 255
            elif method == 4:
                estimate = left + up - corner
                distances = (abs(estimate - left), abs(estimate - up), abs(estimate - corner))
                predictor = (left, up, corner)[distances.index(min(distances))]
                row[x] = (row[x] + predictor) & 255
        rows.append(row)
        previous = row
    return width, height, channels, rows


def verify_presented_pattern(path):
    width, height, channels, rows = png_rgb(path)
    matched = [0, 0]
    total = [0, 0]
    # Sample well inside each half, clear of the status bar, home indicator,
    # rounded display corners and the one-pixel division between the halves.
    for side, center in enumerate((0.25, 0.75)):
        for y in range(int(height * 0.35), int(height * 0.65), max(1, height // 100)):
            for x in range(int(width * (center - 0.1)), int(width * (center + 0.1)), max(1, width // 100)):
                red, green, blue = rows[y][x * channels:x * channels + 3]
                total[side] += 1
                correct = ((green > 200 and red < 50 and blue < 50) if side == 0
                           else (red > 200 and green < 50 and blue < 50))
                if correct:
                    matched[side] += 1
    ratios = [hits / count if count else 0 for hits, count in zip(matched, total)]
    if min(ratios) < 0.95:
        raise RuntimeError(f'UIKit did not present the expected green/red frame: ratios={ratios}')
    return {'width': width, 'height': height, 'green_left_ratio': ratios[0], 'red_right_ratio': ratios[1]}


def wait_for_marker(process, log, marker, timeout=60):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        text = log.read_text(errors='replace')
        if process.poll() is not None:
            raise RuntimeError(f'The simulator launch command exited before capture: {text[-3000:]}')
        if marker in text:
            return
        time.sleep(0.2)
    raise RuntimeError(f'Timed out waiting for {marker}: {log.read_text(errors="replace")[-3000:]}')


def capture_phase(device, bundle, output, name, arguments, marker):
    log = output / (name + '.log')
    screenshot = output / (name + '.png')
    with log.open('w') as stream:
        process = subprocess.Popen([
            'xcrun', 'simctl', 'launch', '--console', '--terminate-running-process',
            device, bundle, *arguments], stdout=stream, stderr=subprocess.STDOUT, text=True)
        try:
            wait_for_marker(process, log, marker)
            # Let UIKit commit the window/layer after the native readiness log.
            time.sleep(1)
            if process.poll() is not None:
                raise RuntimeError('The simulator app stopped before the screenshot')
            run('io', device, 'screenshot', screenshot)
            return {'ready_marker': marker, 'log': log.name, 'screenshot': screenshot.name}
        finally:
            # SDL UIKit deliberately keeps UIApplication alive when SDL_main
            # returns. A simctl process-exit timeout is not a startup failure.
            try:
                run('terminate', device, bundle, timeout=30, check=False)
            finally:
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=10)


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
    result = {'passed': False, 'bundle_id': bundle, 'runtime': runtime['name'],
              'device_type': phone['name'], 'sdk_version': sdk_version, 'commercial_game_resources': False,
              'scope': 'library UI and GLES drawable presentation; physical-device gameplay is not tested'}
    try:
        device = run('create', 'EntisGLS CI startup', phone['identifier'], runtime['identifier']).stdout.strip()
        run('boot', device)
        run('bootstatus', device, '-b', timeout=300)
        run('install', device, app)
        result['library'] = capture_phase(device, bundle, output, 'library',
            ['--library-smoke', '--exit-after', '120'], 'IOS_LIBRARY_READY')
        result['presentation'] = capture_phase(device, bundle, output, 'presentation',
            ['--ios-presentation-smoke'], 'IOS_PRESENTATION_READY')
        result['presentation']['pixels'] = verify_presented_pattern(output / 'presentation.png')
        result['library_ready'] = True
        result['presentation_verified'] = True
        result['passed'] = True
    except Exception as error:
        result['error'] = str(error)
        raise
    finally:
        if device:
            run('shutdown', device, check=False)
            run('delete', device, check=False)
        (output / 'smoke.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
