#!/usr/bin/env python3
"""Exercise the packaged Mac game's Traditional Chinese path on a real display.

Uses isolated saves and a directory containing NOA hard links and the original E-mote driver, so a missing
packaged font cannot be concealed by the game's external SETUP_ZHTW directory.
Screenshots still require visual review; this is not a full-game test.
"""
import argparse
import hashlib
import json
import os
import plistlib
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, default=ROOT/'artifacts/entisgls-launcher/macos-x86_64/EntisGLSLauncher.zip')
    parser.add_argument('--case', choices=('all', 'selftest', 'name', 'dialogue'), default='all')
    parser.add_argument('--game-dir', type=Path, default=ROOT/'StudySteadyR18')
    parser.add_argument('--output-dir', type=Path, default=ROOT/'artifacts/sdl3/zhtw-fix')
    args = parser.parse_args()
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    archive = args.archive.resolve()
    report = {'archive_sha256': hashlib.sha256(archive.read_bytes()).hexdigest(),
              'platform': 'macOS x86_64', 'tests': [], 'external_game_font_available': False,
              'screenshot_review': 'pending', 'scope': 'Font, name screen, first three dialogue messages; no full route or device testing.'}
    previous = output/'runtime-verification.json'
    if args.case != 'all' and previous.is_file():
        earlier = json.loads(previous.read_text())
        if earlier.get('archive_sha256') == report['archive_sha256']:
            report['tests'] = [test for test in earlier.get('tests', []) if test['name'] != args.case]
    startup = ['--click-at', '10,0.37,0.52', '--key-at', '18,escape',
               '--key-at', '22,escape', '--click-at', '28,0.85,0.47']
    cases = [
        ('selftest', ['--self-test', '--exit-after', '45'], None,
         ['SDL OpenType probe PASS', 'Legacy standalone self-tests PASS']),
        ('name', [*startup, '--capture-after', '36', '--exit-after', '40'], 'name.png',
         ['Legacy Window dequeue id=ID_LANGPICKER_ZHTW', 'Legacy Window dequeue id=ID_START']),
        ('dialogue', [*startup, '--click-at', '38,0.831,0.84', '--click-at', '44,0.421,0.695',
                      '--key-at', '60,enter', '--key-at', '64,enter',
                      '--capture-after', '69', '--exit-after', '74'], 'dialogue.png',
         ['Legacy Window dequeue id=ID_NAME_OK', 'Legacy Window dequeue id=ID_YES',
          'Legacy main returned without uncaught error']),
    ]
    with tempfile.TemporaryDirectory(prefix='studysteady-zhtw-release-') as temporary:
        stage = Path(temporary)
        subprocess.run(['/usr/bin/ditto', '-x', '-k', str(archive), str(stage)], check=True)
        app = next(stage.glob('*.app'))
        subprocess.run(['/usr/bin/codesign', '--verify', '--strict', str(app)], check=True)
        metadata = plistlib.loads((app/'Contents/Info.plist').read_bytes())
        binary = app/'Contents/MacOS'/metadata['CFBundleExecutable']
        report['binary_sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
        game = stage/'game-noa-driver'
        game.mkdir()
        for source in args.game_dir.resolve().glob('*.noa'):
            os.link(source, game/source.name)
        os.link(args.game_dir.resolve()/'emotedriver.dll', game/'emotedriver.dll')
        if not (game/'script.noa').is_file():
            raise RuntimeError('The chosen game directory has no script.noa')
        for name, options, screenshot, expected in cases:
            if args.case != 'all' and args.case != name:
                continue
            log = output/(name+'.log')
            command = [str(binary), '--game-dir', str(game), '--local-dir', str(stage/name/'local'),
                       '--storage-dir', str(stage/name/'storage'), *options]
            if screenshot:
                (output/screenshot).unlink(missing_ok=True)
                command += ['--capture-frame', str(output/screenshot)]
            with log.open('w') as stream:
                run = subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT,
                                     env={**os.environ, 'SDL_LOGGING': 'app=debug'}, timeout=100)
            text = log.read_text()
            passed = run.returncode == 0 and all(value in text for value in expected)
            passed &= '24684480 bytes from assets://fonts/NotoSerifCJKtc-Bold.otf' in text
            passed &= 'Legacy main failed' not in text and 'Standalone subsystem failed' not in text
            if screenshot:
                passed &= (output/screenshot).is_file()
            if name == 'dialogue':
                glyph_runs = re.findall(r'Legacy Message output[^\n]*input=(\d+) consumed=(\d+) glyphs=(\d+)[^\n]*font=Noto Serif CJK TC/56', text)
                passed &= len(glyph_runs) >= 6 and all(a == b and int(c) > 0 for a, b, c in glyph_runs)
            report['tests'].append({'name': name, 'exit_code': run.returncode, 'passed': bool(passed),
                                    'log': str(log), 'capture': str(output/screenshot) if screenshot else None})
            print(name, 'PASS' if passed else 'FAIL', flush=True)
            report['status'] = 'PASS' if len(report['tests']) == 3 and all(t['passed'] for t in report['tests']) else 'INCOMPLETE'
            (output/'runtime-verification.json').write_text(json.dumps(report, indent=2)+'\n')
            if not passed:
                raise SystemExit(1)


if __name__ == '__main__':
    main()
