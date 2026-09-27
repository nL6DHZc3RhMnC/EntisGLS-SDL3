#!/usr/bin/env python3
"""Exercise final launcher key discovery through real NOA and motion loading.

The test obtains its override value from the user's original driver at run time.
No game-specific key is supplied by source code. Every mutation is in a temporary
fixture; original archives are linked read-only and the DLL is copied separately.

The supplied archive must be built with ENTISGLS_BUILD_DIAGNOSTICS=ON for
--self-test checks. Production release packages omit these game probes.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, help='Archive from a build configured with -DENTISGLS_BUILD_DIAGNOSTICS=ON', default=ROOT/'artifacts/entisgls-launcher/macos-x86_64/EntisGLSLauncher.zip')
    parser.add_argument('--game-dir', type=Path, default=ROOT/'StudySteadyR18')
    parser.add_argument('--output-dir', type=Path, default=ROOT/'artifacts/entisgls-launcher/psb-discovery')
    args = parser.parse_args()
    archive, source, output = args.archive.resolve(), args.game_dir.resolve(), args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    values = set(int(value) for value in re.findall(rb'(?<![0-9])([0-9]{1,10})\x00#c#r#y#p#t#k#e#y#\x00', (source/'emotedriver.dll').read_bytes()))
    if len(values) != 1 or next(iter(values)) > 0xffffffff:
        raise RuntimeError('Regression fixture requires one driver marker candidate')
    key = next(iter(values))
    wrong = key ^ 1
    report = {'archive_sha256': hashlib.sha256(archive.read_bytes()).hexdigest(), 'tests': [], 'status': 'INCOMPLETE'}
    with tempfile.TemporaryDirectory(prefix='entis-psb-discovery-') as directory:
        stage = Path(directory)
        subprocess.run(['/usr/bin/ditto', '-x', '-k', str(archive), str(stage)], check=True)
        app = stage/'EntisGLSLauncher.app'
        subprocess.run(['/usr/bin/codesign', '--verify', '--strict', str(app)], check=True)
        binary = app/'Contents/MacOS/EntisGLSLauncher'
        report['binary_sha256'] = hashlib.sha256(binary.read_bytes()).hexdigest()
        game = stage/'game'
        game.mkdir()
        for noa in source.glob('*.noa'):
            os.link(noa, game/noa.name)
        local = stage/'local'
        ids = set()

        def run(name, extra=(), code=0, expected=(), probe=True):
            command = [str(binary), '--game-dir', str(game), '--local-dir', str(local), '--exit-after', '30']
            if probe:
                command += ['--self-test']
            command += list(extra)
            process = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=50)
            text = process.stdout
            (output/(name+'.log')).write_text(text)
            found = re.search(r'; id=([^;]+);', text)
            if found:
                ids.add(found.group(1))
            passed = process.returncode == code and all(token in text for token in expected)
            if code == 0 and probe:
                passed &= 'Legacy standalone self-tests PASS' in text
            report['tests'].append({'name': name, 'passed': bool(passed), 'exit_code': process.returncode})
            (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
            print(name, 'PASS' if passed else 'FAIL', flush=True)
            if not passed:
                raise RuntimeError('Regression failed: '+name+'; --self-test requires an archive built with -DENTISGLS_BUILD_DIAGNOSTICS=ON')

        run('missing-driver', code=40, expected=('no game DLL is present',))
        shutil.copyfile(source/'emotedriver.dll', game/'emotedriver.dll')
        run('auto-discovery', expected=('source=dll',))
        caches = list(local.glob('games/*/psb-key-cache/*.cache'))
        if len(caches) != 1:
            raise RuntimeError('Validated discovery did not write exactly one cache')
        run('validated-cache', expected=('source=cache',))
        with (game/'emotedriver.dll').open('ab') as driver:
            driver.write(b'fixture-only fingerprint change')
        run('changed-driver', expected=('source=dll',))
        if len(list(local.glob('games/*/psb-key-cache/*.cache'))) != 2:
            raise RuntimeError('Changed DLL content reused the old fingerprint')
        run('save-manual', ['--save-psb-key', str(key)], probe=False, expected=('override saved',))
        run('cli-overrides-saved', ['--psb-key', str(wrong)], code=40, expected=('configured psb_key did not validate',))
        (game/'emotedriver.dll').unlink()
        run('manual-without-driver', expected=('source=explicit',))
        xml = (ROOT/'assets/compatibility/study-steady-r18.xml').read_text()
        (game/'entis-launcher.xml').write_text(xml.replace('<script ', '<script psb_key="'+str(wrong)+'" '))
        run('saved-overrides-xml', expected=('source=explicit',))
        run('clear-manual', ['--save-psb-key', 'auto'], probe=False, expected=('override cleared',))
        run('xml-key-validates', code=40, expected=('configured psb_key did not validate',))
        (game/'entis-launcher.xml').unlink()
        run('cache-cannot-replace-missing-driver', code=40, expected=('no game DLL is present',))
        if len(ids) != 1:
            raise RuntimeError('Key settings or DLL changes modified save identity')
        report.update(status='PASS', save_identity_stable=True, original_files_unchanged=True,
                      key_source='User-provided DLL at test runtime; no key value recorded',
                      scope='Final Intel macOS package; original NOA, PSB dispatch and per-game settings/cache/error paths')
        (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')


if __name__ == '__main__':
    main()
