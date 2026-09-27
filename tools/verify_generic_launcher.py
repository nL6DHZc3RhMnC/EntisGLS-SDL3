#!/usr/bin/env python3
"""Run a real asset-free CSX through the launcher and verify configuration/save isolation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', required=True, type=Path)
    parser.add_argument('--fixture-generator', required=True, type=Path)
    parser.add_argument('--known-game-dir', type=Path)
    parser.add_argument('--output-dir', type=Path, default=ROOT/'artifacts/entisgls-launcher/generic-tests')
    args = parser.parse_args()
    binary = args.binary.resolve()
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    results = []
    with tempfile.TemporaryDirectory(prefix='entisgls-launcher-test-') as temporary:
        stage = Path(temporary)
        generated = subprocess.run([str(args.fixture_generator.resolve()), str(stage/'seed')], capture_output=True, text=True, check=True)
        (output/'fixture.log').write_text(generated.stdout + generated.stderr)
        game = stage/'遊戲 alpha'
        game.mkdir()
        for name in ['adventure.csx', 'cotopha.xml']:
            shutil.copyfile(stage/'seed'/name, game/name)
        original = (game/'cotopha.xml').read_text()
        empty_assets = stage/'no-assets'
        empty_assets.mkdir()
        local = stage/'local'

        def run(name, directory, extra=(), expected_code=0, expected=()):
            command = [str(binary), '--game-dir', str(directory), '--assets-dir', str(empty_assets),
                       '--local-dir', str(local), '--exit-after', '8', *extra]
            child = subprocess.run(command, capture_output=True, text=True, timeout=25)
            text = child.stdout + child.stderr
            (output/(name+'.log')).write_text(text)
            passed = child.returncode in (expected_code if isinstance(expected_code, tuple) else (expected_code,)) and all(value in text for value in expected)
            results.append({'name': name, 'passed': passed, 'exit_code': child.returncode})
            if not passed:
                raise RuntimeError(f'{name} failed; see {output/(name+".log")}')
            print(name, 'PASS', flush=True)
            return text

        main_text = run('asset-free-main', game, expected=('entry=adventure.csx;', 'profile=;', 'Legacy main returned without uncaught error'))
        if 'Noto' in main_text or 'MsgFont' in main_text:
            raise RuntimeError('Generic launch unexpectedly loaded a game-specific font')
        id_pattern = r'; id=([^;]+);'
        original_id = re.search(id_pattern, main_text).group(1)
        (game/'adventure.csx').rename(game/'冒險.csx')
        (game/'cotopha.xml').write_text(original.replace('adventure.csx', '冒險.csx'))
        run('unicode-entry', game, expected=('entry=冒險.csx;', 'Legacy main returned without uncaught error'))
        (game/'冒險.csx').rename(game/'adventure.csx')
        (game/'cotopha.xml').write_text(original)
        moved = stage/'renamed game'
        game.rename(moved)
        moved_text = run('directory-move', moved, expected=('Legacy main returned without uncaught error',))
        if re.search(id_pattern, moved_text).group(1) != original_id:
            raise RuntimeError('Moving the game changed save identity')
        (moved/'cotopha.xml').write_text(original.replace('<script ', '<script id="fixture-alpha" '))
        run('explicit-id-alpha', moved, expected=('id=fixture-alpha;', 'Legacy main returned without uncaught error'))
        (moved/'cotopha.xml').write_text(original.replace('<script ', '<script id="fixture-beta" '))
        run('explicit-id-beta', moved, expected=('id=fixture-beta;', 'Legacy main returned without uncaught error'))
        for identity in [original_id, 'fixture-alpha', 'fixture-beta']:
            if not (local/'games'/identity/'savedata').is_dir():
                raise RuntimeError('Missing isolated save directory: '+identity)
        (moved/'cotopha.xml').write_text(original.replace('adventure.csx', 'main.lqs'))
        run('unsupported-vm', moved, expected_code=1, expected=('traditional Cotopha .csx only',))
        (moved/'cotopha.xml').write_text(original.replace('adventure.csx', 'missing.csx'))
        run('missing-entry', moved, expected_code=3, expected=('Entry script not found: missing.csx',))
        (moved/'cotopha.xml').write_text(original)
        run('no-game-specific-probes', moved, ['--self-test'], expected_code=2, expected=('diagnostic probes require',))
        (moved/'cotopha.xml').unlink()
        run('no-guessed-default', moved, expected_code=1, expected=('configuration',))

        if args.known_game_dir:
            previous = local/'savedata'
            previous.mkdir()
            (previous/'migration-marker.bin').write_bytes(b'original-save')
            run('legacy-save-migration', args.known_game_dir.resolve(), ['--exit-after', '1', '--legacy-local-data'], expected_code=(0, 6),
                expected=('id=study-steady-r18;', 'profile=study-steady-r18;'))
            destination = local/'games/study-steady-r18/savedata/migration-marker.bin'
            if destination.read_bytes() != b'original-save' or (previous/'migration-marker.bin').read_bytes() != b'original-save':
                raise RuntimeError('Legacy save migration lost original data')
            destination.write_bytes(b'newer-save')
            run('migration-preserves-newer-save', args.known_game_dir.resolve(), ['--exit-after', '1', '--legacy-local-data'], expected_code=(0, 6))
            if destination.read_bytes() != b'newer-save':
                raise RuntimeError('Migration overwrote a newer save')
            known = stage/'known-with-explicit-id'
            known.mkdir()
            os.link(args.known_game_dir.resolve()/'script.noa', known/'script.noa')
            (known/'cotopha.xml').write_text(original.replace('<script ', '<script id="custom-study-mod" '))
            run('known-game-explicit-id', known, ['--inspect-game', '--legacy-local-data'], expected=('id=custom-study-mod;',))
            if (local/'games/custom-study-mod').exists():
                raise RuntimeError('Inspection unexpectedly created a save directory')

    report = {'status': 'PASS', 'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
              'tests': results, 'scope': 'Real traditional CSX execution, no commercial assets for generic cases; known game used read-only for migration checks.'}
    (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')


if __name__ == '__main__':
    main()
