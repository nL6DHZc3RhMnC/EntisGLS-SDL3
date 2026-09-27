#!/usr/bin/env python3
"""Check the asset-free iOS library and real GLES presentation on a simulator."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import struct
import subprocess
import time
import zlib


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def tail(path, limit=6000):
    if not path.exists():
        return ''
    with path.open('rb') as stream:
        stream.seek(0, 2)
        stream.seek(max(0, stream.tell() - limit))
        return stream.read().decode('utf-8', errors='replace')


class Diagnostics:
    """Persist command output and progress before attempting any cleanup."""
    def __init__(self, output):
        self.output = Path(output)
        self.output.mkdir(parents=True, exist_ok=True)
        self.started_wall = time.time()
        self.started = time.monotonic()
        self.active_stage = None
        self.failure_collected = False
        self.result = {
            'passed': False, 'started_at': utc_now(), 'commands': [], 'stages': [],
            'cleanup_errors': [], 'diagnostic_errors': [], 'commercial_game_resources': False,
            'scope': 'library readiness and GLES drawable presentation; physical-device gameplay is not tested',
        }
        self.save()

    def save(self):
        self.result['elapsed_seconds'] = round(time.monotonic() - self.started, 3)
        temporary = self.output / 'smoke.json.tmp'
        temporary.write_text(json.dumps(self.result, indent=2) + '\n')
        temporary.replace(self.output / 'smoke.json')

    @contextmanager
    def stage(self, name):
        record = {'name': name, 'started_at': utc_now(), 'status': 'running'}
        self.result['stages'].append(record)
        previous, self.active_stage = self.active_stage, name
        started = time.monotonic()
        self.save()
        try:
            yield record
            record['status'] = 'passed'
        except Exception as error:
            record.update(status='failed', error=str(error))
            raise
        finally:
            record['elapsed_seconds'] = round(time.monotonic() - started, 3)
            self.active_stage = previous
            self.save()

    def command(self, label, arguments, *, timeout=180, check=True, read_output=True):
        arguments = list(map(str, arguments))
        number = len(self.result['commands']) + 1
        stem = f'{number:03d}-' + re.sub(r'[^A-Za-z0-9_.-]', '-', label)[:80]
        stdout, stderr = self.output / (stem + '.stdout.log'), self.output / (stem + '.stderr.log')
        record = {'name': label, 'stage': self.active_stage, 'argv': arguments,
                  'started_at': utc_now(), 'timeout_seconds': timeout, 'status': 'running',
                  'stdout': stdout.name, 'stderr': stderr.name, 'returncode': None}
        self.result['commands'].append(record)
        started = time.monotonic()
        self.save()
        failure = None
        try:
            # Files retain partial output even when subprocess.run kills a timed-out
            # simctl. They also avoid buffering an unbounded system log in memory.
            with stdout.open('w') as out, stderr.open('w') as err:
                completed = subprocess.run(arguments, stdout=out, stderr=err,
                                           text=True, timeout=timeout, check=False)
            record['returncode'] = completed.returncode
            record['status'] = 'passed' if completed.returncode == 0 else 'failed'
            if check and completed.returncode:
                failure = RuntimeError(f'{label} exited {completed.returncode}; see {stdout.name} / '
                                       f'{stderr.name}: {tail(stderr)}')
        except subprocess.TimeoutExpired:
            record['status'] = 'timeout'
            failure = RuntimeError(f'{label} timed out after {timeout}s; see {stdout.name} / '
                                   f'{stderr.name}: {tail(stderr)}')
        except Exception as error:
            record['status'] = 'error'
            failure = RuntimeError(f'{label} could not run: {error}')
        finally:
            record['elapsed_seconds'] = round(time.monotonic() - started, 3)
            if failure:
                record['error'] = str(failure)
            self.save()
        if failure:
            raise failure
        return subprocess.CompletedProcess(arguments, record['returncode'],
            stdout.read_text(errors='replace') if read_output else '',
            stderr.read_text(errors='replace') if read_output else '')

    def simctl(self, label, *arguments, **options):
        return self.command(label, ['xcrun', 'simctl', *arguments], **options)

    def attempt(self, category, name, operation):
        try:
            return operation()
        except Exception as error:
            self.result[category].append({'name': name, 'error': str(error), 'at': utc_now()})
            self.save()
            return None


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
    if width <= height:
        raise RuntimeError(f'UIKit did not rotate the landscape game window: {width}x{height}')
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
    return {'width': width, 'height': height, 'orientation': 'landscape',
            'green_left_ratio': ratios[0], 'red_right_ratio': ratios[1]}


def pid_alive(pid):
    try:
        os.kill(pid, 0)
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        return True


def launch_pid(text, bundle):
    match = re.search(r'^' + re.escape(bundle) + r':\s*([1-9][0-9]*)\s*$', text, re.MULTILINE)
    if not match:
        raise RuntimeError(f'simctl did not return the launched app PID: {text[-3000:]}')
    return int(match.group(1))


def app_text(paths):
    return '\n'.join(path.read_text(errors='replace') if path.exists() else '' for path in paths)


def wait_for_marker(pid, paths, marker, timeout=60):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        text = app_text(paths)
        if not pid_alive(pid):
            raise RuntimeError(f'The simulator app PID {pid} exited before {marker}: {text[-3000:]}')
        if marker in text:
            return
        time.sleep(0.2)
    raise RuntimeError(f'Timed out waiting for {marker}; app PID {pid} is still alive: {app_text(paths)[-3000:]}')


def copy_app_logs(diagnostics, name, paths):
    for source, suffix in zip(paths, ('stdout', 'stderr')):
        target = diagnostics.output / f'{name}.{suffix}.log'
        if source.exists():
            shutil.copyfile(source, target)
        else:
            target.write_text('')
    # Keep the familiar phase log as well as the unmerged, original streams.
    (diagnostics.output / f'{name}.log').write_text(
        '[stdout]\n' + app_text(paths[:1]) + '\n[stderr]\n' + app_text(paths[1:]))


def collect_crashes(diagnostics, device, executable):
    destinations = diagnostics.output / 'crashes'
    roots = [Path.home() / 'Library/Logs/DiagnosticReports',
             Path.home() / 'Library/Developer/CoreSimulator/Devices' / device / 'data/Library/Logs/CrashReporter',
             Path.home() / 'Library/Developer/CoreSimulator/Devices' / device / 'data/Library/Logs/DiagnosticReports']
    copied = diagnostics.result.setdefault('crash_reports', [])
    for root in roots:
        if not root.is_dir():
            continue
        for path in sorted(root.iterdir()):
            if (len(copied) >= 32 or path.is_symlink() or not path.is_file() or
                    not path.name.startswith(executable) or path.suffix not in ('.ips', '.crash') or
                    path.stat().st_mtime < diagnostics.started_wall - 2):
                continue
            if path.stat().st_size > 20 * 1024 * 1024:
                continue
            destinations.mkdir(exist_ok=True)
            target = destinations / f'{len(copied):02d}-{path.name}'
            shutil.copyfile(path, target)
            copied.append({'source': str(path), 'artifact': str(target.relative_to(diagnostics.output))})


def collect_failure(diagnostics, device, executable, name, pid=None):
    if diagnostics.failure_collected or not device:
        return
    diagnostics.failure_collected = True
    attempt = lambda label, call: diagnostics.attempt('diagnostic_errors', label, call)
    attempt('failure screenshot', lambda: diagnostics.simctl(name + '-failure-screenshot',
        'io', device, 'screenshot', diagnostics.output / f'{name}-failure.png', timeout=15))
    if pid is not None and pid_alive(pid):
        # Only sample the PID returned for our own launch, never an arbitrary app.
        attempt('app sample', lambda: diagnostics.command(name + '-sample',
            ['/usr/bin/sample', str(pid), '3', '1', '-file', str(diagnostics.output / f'{name}.sample.txt')],
            timeout=10, read_output=False))
    attempt('process inventory', lambda: diagnostics.command(name + '-processes',
        ['/bin/ps', '-axo', 'pid=,ppid=,stat=,etime=,comm='], timeout=10, read_output=False))
    predicate = ' OR '.join('process == ' + json.dumps(value)
                            for value in (executable, 'SpringBoard', 'runningboardd'))
    attempt('simulator unified log', lambda: diagnostics.simctl(name + '-system-log',
        'spawn', device, 'log', 'show', '--style', 'compact', '--last', '5m', '--info', '--debug',
        '--predicate', predicate, timeout=30, read_output=False))
    attempt('failure device inventory', lambda: diagnostics.simctl(name + '-device-inventory',
        'list', '--json', timeout=15, read_output=False))
    attempt('app crash reports', lambda: collect_crashes(diagnostics, device, executable))
    diagnostics.save()


def capture_phase(diagnostics, device, bundle, executable, container, name, arguments, marker):
    result = {'ready_marker': marker, 'log': name + '.log', 'stdout': name + '.stdout.log',
              'stderr': name + '.stderr.log', 'screenshot': name + '.png', 'pid': None, 'passed': False}
    diagnostics.result[name] = result
    # get_app_container returns a host path. Using its tmp directory avoids the
    # /tmp and /var path rewriting performed by CoreSimulator launch redirects.
    logdir = container / 'tmp'
    paths = [logdir / f'entis-smoke-{name}.{suffix}.log' for suffix in ('stdout', 'stderr')]
    primary = None
    try:
        logdir.mkdir(exist_ok=True)
        for path in paths:
            path.write_text('')
        with diagnostics.stage(name + '-launch'):
            # Detached launch separates the launch acknowledgement from app IO.
            # It is not proof of readiness: require its PID, liveness and marker.
            launched = diagnostics.simctl(name + '-launch', 'launch',
                '--stdout=' + str(paths[0]), '--stderr=' + str(paths[1]), device, bundle, *arguments,
                timeout=60)
            result['pid'] = launch_pid(launched.stdout, bundle)
        with diagnostics.stage(name + '-ready'):
            wait_for_marker(result['pid'], paths, marker)
        with diagnostics.stage(name + '-capture'):
            time.sleep(1)  # Let UIKit commit the layer after native readiness.
            if not pid_alive(result['pid']):
                raise RuntimeError('The simulator app exited before the screenshot')
            diagnostics.simctl(name + '-screenshot', 'io', device, 'screenshot',
                                diagnostics.output / result['screenshot'], timeout=30)
            if not pid_alive(result['pid']):
                raise RuntimeError('The simulator app exited during the screenshot')
            result['alive_after_capture'] = True
        result['passed'] = True
    except Exception as error:
        primary = error
        result['error'] = str(error)
        diagnostics.result.setdefault('error', str(error))
        diagnostics.save()
        collect_failure(diagnostics, device, executable, name, result['pid'])
    finally:
        diagnostics.attempt('diagnostic_errors', name + ' app logs',
                            lambda: copy_app_logs(diagnostics, name, paths))
        before = len(diagnostics.result['cleanup_errors'])
        diagnostics.attempt('cleanup_errors', name + ' terminate', lambda: diagnostics.simctl(
            name + '-terminate', 'terminate', device, bundle, timeout=30))
        if len(diagnostics.result['cleanup_errors']) != before and primary is None:
            primary = RuntimeError(diagnostics.result['cleanup_errors'][-1]['error'])
            result['error'] = str(primary)
            diagnostics.result.setdefault('error', str(primary))
            collect_failure(diagnostics, device, executable, name, result['pid'])
        if primary is not None:
            result['passed'] = False
        diagnostics.save()
    if primary is not None:
        raise primary
    return result


def run_smoke(app, output):
    diagnostics = Diagnostics(output)
    result = diagnostics.result
    device = None
    executable = 'EntisGLSLauncher'
    primary = None
    try:
        with diagnostics.stage('inspect-app-and-toolchain'):
            info = plistlib.loads((app / 'Info.plist').read_bytes())
            bundle = info['CFBundleIdentifier']
            if bundle != 'io.entisgls.launcher':
                raise RuntimeError('Unexpected simulator bundle ID')
            executable = info['CFBundleExecutable']
            if not executable or Path(executable).name != executable:
                raise RuntimeError('Invalid simulator executable name')
            result.update(bundle_id=bundle, executable=executable)
            inventory = json.loads(diagnostics.simctl('initial-inventory', 'list', '--json').stdout)
            (diagnostics.output / 'inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
            diagnostics.command('xcode-version', ['xcodebuild', '-version'], timeout=30)
            sdk_version = diagnostics.command('simulator-sdk',
                ['xcrun', '--sdk', 'iphonesimulator', '--show-sdk-version'], timeout=30).stdout.strip()
            help_result = diagnostics.simctl('launch-help', 'help', 'launch', timeout=30, check=False)
            if not all(option in help_result.stdout + help_result.stderr for option in ('--stdout', '--stderr')):
                raise RuntimeError('The selected simctl launch help does not advertise stdout/stderr redirection')
            runtime, phone = select_device(inventory, sdk_version)
            result.update(runtime=runtime['name'], device_type=phone['name'], sdk_version=sdk_version)
        with diagnostics.stage('create-and-boot'):
            # Only this freshly created device is ever shut down or deleted.
            device = diagnostics.simctl('create', 'create', 'EntisGLS CI startup',
                phone['identifier'], runtime['identifier']).stdout.strip()
            result['device_id'] = device
            diagnostics.simctl('boot', 'boot', device)
            diagnostics.simctl('bootstatus', 'bootstatus', device, '-b', timeout=300)
        with diagnostics.stage('install'):
            diagnostics.simctl('install', 'install', device, app)
            container = Path(diagnostics.simctl('app-data-container', 'get_app_container',
                device, bundle, 'data', timeout=30).stdout.strip())
            if not container.is_absolute() or not container.is_dir():
                raise RuntimeError(f'simctl returned an unavailable app data container: {container}')
            result['app_data_container'] = str(container)
        capture_phase(diagnostics, device, bundle, executable, container, 'library',
            ['--library-smoke', '--exit-after', '120'], 'IOS_LIBRARY_READY')
        result['library_ready'] = True
        capture_phase(diagnostics, device, bundle, executable, container, 'presentation',
            ['--ios-presentation-smoke'], 'IOS_PRESENTATION_READY')
        with diagnostics.stage('verify-presentation-pixels'):
            result['presentation']['pixels'] = verify_presented_pattern(diagnostics.output / 'presentation.png')
        result['presentation_verified'] = True
        result['checks_completed'] = True
    except Exception as error:
        primary = error
        result['passed'] = False
        result.setdefault('error', str(error))
        diagnostics.save()  # Preserve the first failure before diagnostics/cleanup.
        collect_failure(diagnostics, device, executable, 'failure')
    finally:
        if device:
            for operation in ('shutdown', 'delete'):
                diagnostics.attempt('cleanup_errors', operation, lambda operation=operation:
                    diagnostics.simctl(operation, operation, device, timeout=30))
        if primary is None and result['cleanup_errors']:
            primary = RuntimeError(result['cleanup_errors'][0]['error'])
            result['error'] = str(primary)
        # Do not publish PASS while cleanup is still running: cancellation at
        # that point must leave an incomplete/failed report, not a stale success.
        result['passed'] = primary is None and bool(result.get('checks_completed'))
        result['finished_at'] = utc_now()
        diagnostics.save()
    if primary is not None:
        raise primary
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--app', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(run_smoke(args.app.resolve(), args.output.resolve())))


if __name__ == '__main__':
    main()
