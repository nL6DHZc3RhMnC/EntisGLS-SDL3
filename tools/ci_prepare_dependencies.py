#!/usr/bin/env python3
"""Verify a clean checkout using pinned upstream archives, without game inputs.

Only the small verification cache under build/downloads is downloaded. Source,
font and license files already committed to the repository are never rewritten.
"""

import argparse
import json
from pathlib import Path
import tarfile
import time
import urllib.error

import entis_sdk
import motion_verify_imports
import setup_sdl3
import setup_sdl_fonts


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--offline', action='store_true',
                        help='Require the verification archives to be cached already')
    args = parser.parse_args()
    # Fail on an incomplete checkout instead of installing or substituting any
    # game font or vendored source tree. All preparation writes stay in build/.
    for path in (setup_sdl3.SOURCE / 'CMakeLists.txt',
                 setup_sdl_fonts.SOURCE / 'CMakeLists.txt',
                 setup_sdl_fonts.FONT):
        if path.is_symlink() or not path.is_file():
            raise RuntimeError(f'Missing committed build input: {path}')

    inputs = (
        (setup_sdl3.ARCHIVE, setup_sdl3.ARCHIVE_URL, setup_sdl3.ARCHIVE_SHA256),
        (setup_sdl_fonts.ARCHIVE, setup_sdl_fonts.ARCHIVE_URL,
         setup_sdl_fonts.ARCHIVE_SHA256),
        (setup_sdl_fonts.OFL, setup_sdl_fonts.OFL_URL, setup_sdl_fonts.OFL_SHA256),
    )
    for path, url, checksum in inputs:
        if args.offline and not path.is_file():
            raise RuntimeError(f'Missing verification cache: {path}; run without --offline')
        for attempt in range(3):
            try:
                setup_sdl_fonts.download_pinned(path, url, checksum, args.offline)
                break
            except (urllib.error.URLError, TimeoutError) as error:
                if args.offline or attempt == 2:
                    raise
                print(f'Download failed ({error}); retrying {url}', flush=True)
                time.sleep(2 ** attempt)

    with tarfile.open(setup_sdl3.ARCHIVE, 'r:gz') as archive:
        setup_sdl3.verify(archive)
    setup_sdl_fonts.prepare_freetype(verify_only=True)
    # verify_only cannot read a game directory: it fails if the committed font
    # is absent. The argument is used only by the separate initial-copy mode.
    setup_sdl_fonts.prepare_font(game_dir=ROOT, verify_only=True)
    entis_sdk.validate()
    motion_verify_imports.main()

    report = ROOT / 'build/ci/dependencies.json'
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(json.dumps({
        'game_inputs_used': False,
        'verification': 'committed sources compared with pinned upstream inputs',
        'inputs': [{'file': path.relative_to(ROOT).as_posix(), 'url': url,
                    'sha256': checksum} for path, url, checksum in inputs],
    }, indent=2) + '\n')
    print(f'CI dependencies verified; report: {report}')


if __name__ == '__main__':
    main()
