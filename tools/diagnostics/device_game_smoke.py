#!/usr/bin/env python3
"""Exercise the original ZHTW UI on the connected 2340x1080 Mi 10 Pro.

Loads an existing save on page 01 or 02; never writes a save. Stops before further input or
screenshots when the game's main has exited or Android reports a current ANR.
Logs are saved even on failure; active Android errors also write .health.txt.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
from pathlib import Path
import subprocess
import re
import sys
import time

from android_app import PACKAGE_ID as PACKAGE

def _dump_blocks(text, header_pattern):
    """Yield only live, indented dumpsys records, not historical summary text."""
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if not re.match(header_pattern, line):
            continue
        indent = len(line) - len(line.lstrip())
        end = index + 1
        while end < len(lines):
            following = lines[end]
            if following.strip() and len(following) - len(following.lstrip()) <= indent:
                break
            end += 1
        yield line, '\n'.join(lines[index:end])


def android_health_failure(pid, processes, windows, package=PACKAGE):
    """Return (reason, current evidence) for this app's active ANR, else None.

    Process flags must belong to this exact PID/package. Window evidence must
    be a currently displayed ANR window. DropBox/lastanr history is never used.
    """
    identity = re.compile(r'ProcessRecord\{[^}\n]*\s' + re.escape(str(pid)) +
                          ':' + re.escape(package) + r'/[^}]+\}')
    for header, block in _dump_blocks(processes, r'\s*\*APP\*.*ProcessRecord\{'):
        if identity.search(header) and re.search(r'\bmNotResponding=true\b', block):
            return 'Android reports ANR for current game process ' + str(pid), block

    title = re.compile(r'Application Not Responding:\s*' + re.escape(package) + r'(?=[\s}:])')
    for header, block in _dump_blocks(windows, r'\s*Window #\d+ Window\{'):
        if not title.search(header):
            continue
        # A removed/hidden dialog can remain in a dump briefly. Require its
        # real surface and visibility, rather than matching its title alone.
        if re.search(r'\b(?:mDestroying|mRemoved|mRemoveOnExit)=true\b', block):
            continue
        if (re.search(r'\bmHasSurface=true\b', block)
                and re.search(r'(?:Surface: shown=true\b|\bmSurfaceShown=true\b)', block)
                and re.search(r'\b(?:isVisible|isOnScreen)=true\b', block)):
            return 'Android ANR dialog is currently visible for ' + package, block
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--serial', required=True)
    parser.add_argument('--flow', choices=['cold-load', 'new-game', 'warm-load', 'advance', 'observe'], required=True)
    parser.add_argument('--steps', type=int, default=0, help='Additional dialogue taps after the flow')
    parser.add_argument('--interval', type=float, default=2,
                        help='Seconds between additional dialogue taps (default: 2)')
    parser.add_argument('--slot', type=int, choices=[*range(10, 19), *range(20, 29)], default=14,
                        help='Existing file slot on save page 01 or 02 (default: 14)')
    parser.add_argument('--initial-page', type=int, choices=[1, 2], default=1,
                        help='Page shown when opening the load UI, from the saved test settings (default: 1)')
    parser.add_argument('--output', type=Path, required=True, help='Output basename, without extension')
    parser.add_argument('--wait', type=float, default=0,
                        help='Observe healthy gameplay before capture, up to 55 seconds; preserves logs')
    args = parser.parse_args()
    if not 0 <= args.steps <= 100:
        parser.error('--steps must be between 0 and 100')
    if not .25 <= args.interval <= 5:
        parser.error('--interval must be between 0.25 and 5 seconds')
    if not 0 <= args.wait <= 55:
        parser.error('--wait must be between 0 and 55 seconds')
    adb = [str(ROOT / '.android-tools/platform-tools/platform-tools/adb'), '-s', args.serial]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    pid = None

    def command(*words):
        # logcat otherwise waits indefinitely when USB disappears mid-flow.
        return subprocess.check_output(adb + list(words), stderr=subprocess.STDOUT,
                                       timeout=15)

    def log():
        words = ['logcat', '-d', '-v', 'threadtime']
        if pid:
            words += ['--pid', pid]
        return command(*words, '-s', 'StudySteady')

    def healthy():
        current = command('shell', 'pidof', PACKAGE).decode().strip()
        if pid and current != pid:
            raise RuntimeError('Game process changed or exited')
        processes = command('shell', 'dumpsys', 'activity', 'processes', PACKAGE).decode(errors='replace')
        windows = command('shell', 'dumpsys', 'window', 'windows').decode(errors='replace')
        failure = android_health_failure(current, processes, windows)
        if failure:
            reason, evidence = failure
            args.output.with_suffix('.health.txt').write_text(
                reason + '\nPID: ' + current + '\n\n' + evidence + '\n', encoding='utf-8')
            raise RuntimeError(reason + '; inspect the saved .health.txt and log')
        output = log()
        if b'Legacy main failed:' in output or b'Legacy main returned' in output:
            raise RuntimeError('Game main exited; inspect the saved log')
        resumed = command('shell', 'dumpsys', 'activity', 'activities').decode(errors='replace')
        if not any(PACKAGE in line for line in resumed.splitlines()
                   if 'mResumedActivity:' in line or 'topResumedActivity=' in line):
            raise RuntimeError('Game is not the foreground activity')

    def tap(x, y, delay):
        healthy()
        command('shell', 'input', 'tap', str(x), str(y))
        time.sleep(delay)

    try:
        if args.flow not in ('advance', 'observe'):
            command('shell', 'am', 'force-stop', PACKAGE)
            command('logcat', '-c')
            command('shell', 'am', 'start', '-n', PACKAGE + '/.GameActivity')
            time.sleep(3)
        pid = command('shell', 'pidof', PACKAGE).decode().strip()
        if args.flow in ('advance', 'observe'):
            healthy()
            if args.flow == 'advance':
                command('logcat', '-c')
        else:
            tap(974, 540, 13)  # Traditional Chinese, then title animation.
            if args.flow == 'cold-load':
                tap(1830, 575, 3)
            else:
                tap(1830, 492, 2.5)
                tap(1830, 875, 2)
                tap(1080, 720, 10)
                if args.flow == 'warm-load':
                    healthy()
                    command('shell', 'input', 'keyevent', '133')
                    time.sleep(3)
            if args.flow in ('cold-load', 'warm-load'):
                target_page = args.slot // 10
                if target_page != args.initial_page:
                    tap(1380 if target_page > args.initial_page else 1048, 970, 3)
                row, column = divmod(args.slot % 10, 3)
                tap([635, 1220, 1790][column], [270, 520, 770][row], 2)
                tap(1065, 565, 5)
                restored = log()
                if (b'Legacy context load complete' not in restored
                        and b'Primary restore load-complete' not in restored):
                    raise RuntimeError('Load completion checkpoint was not reached')
        for index in range(args.steps):
            tap(1240, 760, args.interval)
            print(f'Dialogue tap {index + 1}/{args.steps}', flush=True)
        until = time.monotonic() + args.wait
        while time.monotonic() < until:
            healthy()
            time.sleep(min(1, max(0, until - time.monotonic())))
        healthy()
        args.output.with_suffix('.png').write_bytes(command('exec-out', 'screencap', '-p'))
    finally:
        primary_failure = sys.exc_info()[0] is not None
        try:
            captured = log()
        except (subprocess.SubprocessError, OSError) as error:
            args.output.with_suffix('.log-error.txt').write_text(
                'Could not collect the device log; existing log retained.\n'
                + str(error) + '\n', encoding='utf-8')
            if not primary_failure:
                raise
        else:
            if captured or not args.output.with_suffix('.txt').exists():
                args.output.with_suffix('.txt').write_bytes(captured)
    print(f'Flow completed; inspect {args.output.with_suffix(".png")} for visual correctness', flush=True)


if __name__ == '__main__':
    main()
