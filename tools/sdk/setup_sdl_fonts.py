#!/usr/bin/env python3
"""Prepare pinned FreeType and the game's unmodified Traditional Chinese font.

Source/font/license inputs are installed once and subsequently verified. Existing
files are never replaced. No host font installation or package manager is used.
The game original and other vendor trees remain read-only.
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
from pathlib import Path, PurePosixPath
import re
import shutil
import tarfile
import tempfile
import urllib.request

VERSION = "2.14.3"
TAG = "VER-2-14-3"
COMMIT = "0a0221a1347e2f1e07c395263540026e9a0aa7c7"
TAG_OBJECT = "c740f0fda4274d6ffd2e5b64a25b06ef69803a07"
ARCHIVE_SHA256 = "dc49de6b01a266eef4876a4dd34d9842c475d3e28ff2eff63bd2fb760ab56261"
ARCHIVE_URL = f"https://codeload.github.com/freetype/freetype/tar.gz/refs/tags/{TAG}"
ARCHIVE = ROOT / "build/downloads/freetype-2.14.3-VER-2-14-3.tar.gz"
SOURCE = ROOT / "vendor/freetype"
PROVENANCE = ROOT / "vendor/freetype-provenance.json"
FONT_NAME = "NotoSerifCJKtc-Bold.otf"
FONT_SHA256 = "d6bc09d324004b38207898f86deb298cc4eb0527bc40916f25ba9d3ba07226f9"
FONT = ROOT / "assets/fonts" / FONT_NAME
FONT_PROVENANCE = ROOT / "assets/fonts/provenance.json"
OFL_COMMIT = "f8d157532fbfaeda587e826d4cd5b21a49186f7c"
OFL_URL = f"https://raw.githubusercontent.com/notofonts/noto-cjk/{OFL_COMMIT}/Serif/LICENSE"
OFL_SHA256 = "6a73f9541c2de74158c0e7cf6b0a58ef774f5a780bf191f2d7ec9cc53efe2bf2"
OFL = ROOT / "build/downloads/NotoSerif-OFL-1.1.txt"


def digest(path):
    sha = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    return sha.hexdigest()


def download_pinned(path, url, expected, verify_only):
    if not path.exists():
        if verify_only:
            raise RuntimeError(f"Missing cached input: {path}; run without --verify to download")
        path.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as output:
            pending = Path(output.name)
            try:
                request = urllib.request.Request(url, headers={"User-Agent": "StudySteady-font-setup"})
                with urllib.request.urlopen(request, timeout=60) as response:
                    shutil.copyfileobj(response, output)
                output.flush()
                if digest(pending) != expected:
                    raise RuntimeError(f"Downloaded input checksum mismatch: {url}")
                pending.replace(path)
            finally:
                pending.unlink(missing_ok=True)
    if path.is_symlink() or not path.is_file() or digest(path) != expected:
        raise RuntimeError(f"Pinned input checksum mismatch: {path}")


def archive_files(archive):
    seen = set()
    for member in archive.getmembers():
        parts = PurePosixPath(member.name).parts
        if not parts or parts[0] != f"freetype-{TAG}" or ".." in parts:
            raise RuntimeError(f"Unexpected archive path: {member.name}")
        if member.isdir():
            continue
        if not member.isfile() or len(parts) < 2:
            raise RuntimeError(f"Unsupported archive entry: {member.name}")
        relative = Path(*parts[1:])
        if relative in seen:
            raise RuntimeError(f"Duplicate archive entry: {relative}")
        seen.add(relative)
        yield member, relative


def install_bytes(path, data, verify_only):
    if path.exists() or path.is_symlink():
        if path.is_symlink() or not path.is_file() or path.read_bytes() != data:
            raise RuntimeError(f"Existing file differs from pinned input; left unchanged: {path}")
        return
    if verify_only:
        raise RuntimeError(f"Missing generated input metadata/license: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    # Exclusive creation also protects against a concurrent setup overwriting a file.
    with path.open("xb") as output:
        output.write(data)


def json_bytes(value):
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def prepare_freetype(verify_only):
    download_pinned(ARCHIVE, ARCHIVE_URL, ARCHIVE_SHA256, verify_only)
    with tarfile.open(ARCHIVE, "r:gz") as archive:
        members = list(archive_files(archive))
        if not SOURCE.exists():
            if verify_only:
                raise RuntimeError(f"Missing FreeType source tree: {SOURCE}")
            SOURCE.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.TemporaryDirectory(prefix=".freetype-", dir=SOURCE.parent) as temporary:
                staged = Path(temporary) / "source"
                staged.mkdir()
                for member, relative in members:
                    path = staged / relative
                    path.parent.mkdir(parents=True, exist_ok=True)
                    with archive.extractfile(member) as stream, path.open("wb") as output:
                        shutil.copyfileobj(stream, output)
                    path.chmod(member.mode & 0o777)
                staged.rename(SOURCE)
        if SOURCE.is_symlink() or not SOURCE.is_dir():
            raise RuntimeError(f"Unexpected FreeType source root: {SOURCE}")
        hashes = {}
        for member, relative in members:
            path = SOURCE / relative
            if path.is_symlink() or not path.is_file():
                raise RuntimeError(f"Missing/unexpected FreeType file: {path}")
            with archive.extractfile(member) as stream:
                expected = hashlib.sha256(stream.read()).hexdigest()
            if digest(path) != expected:
                raise RuntimeError(f"Modified FreeType source, left unchanged: {path}")
            hashes[relative.as_posix()] = expected
        hashes = dict(sorted(hashes.items()))
        actual = {p.relative_to(SOURCE).as_posix() for p in SOURCE.rglob("*") if p.is_file() or p.is_symlink()}
        if actual != set(hashes):
            raise RuntimeError(f"Unexpected FreeType tree entries: {sorted(actual - set(hashes))}")
    tree_sha = hashlib.sha256("".join(f"{sha}  {name}\n" for name, sha in hashes.items()).encode()).hexdigest()
    metadata = {
        "name": "FreeType", "version": VERSION,
        "repository": "https://github.com/freetype/freetype",
        "upstream_homepage": "https://freetype.org/",
        "release_date": "2026-03-22", "tag": TAG, "tag_object": TAG_OBJECT, "commit": COMMIT,
        "archive_url": ARCHIVE_URL, "archive_sha256": ARCHIVE_SHA256,
        "source_directory": "vendor/freetype", "upstream_modifications": [],
        "selected_license": "FTL", "license_file": "vendor/freetype/docs/FTL.TXT",
        "file_count": len(hashes), "tree_sha256": tree_sha,
        "tree_hash_format": "UTF-8 lines sorted by path: sha256 + two spaces + relative POSIX path + newline",
        "files_sha256": hashes, "verification": "python3 tools/sdk/setup_sdl_fonts.py --verify",
    }
    install_bytes(PROVENANCE, json_bytes(metadata), verify_only)
    install_bytes(ROOT / "assets/licenses/FreeType-FTL.txt", (SOURCE / "docs/FTL.TXT").read_bytes(), verify_only)
    install_bytes(ROOT / "assets/licenses/FreeType-LICENSE.txt", (SOURCE / "LICENSE.TXT").read_bytes(), verify_only)
    # Preserve the permissive notices for contributed modules too. Including
    # these notices is harmless when a given module is disabled at build time.
    notices = []
    for relative in ("src/bdf/README", "src/pcf/README"):
        notices.append(f"FreeType {VERSION}: {relative}\n\n" + (SOURCE / relative).read_text())
    for relative in ("src/gzip/zlib.h", "src/base/fthash.c", "src/autofit/ft-hb-ft.c",
                     "src/autofit/hb-script-list.h"):
        content = (SOURCE / relative).read_text()
        notice = next((part for part in re.findall(r"/\*.*?\*/", content, re.S)
                       if "Copyright" in part and "Permission" in part), None)
        if not notice:
            raise RuntimeError(f"Expected contributed module license missing: {relative}")
        notices.append(f"FreeType {VERSION}: {relative}\n\n" + notice)
    install_bytes(ROOT / "assets/licenses/FreeType-THIRD-PARTY.txt",
                  ("\n\n".join(notices) + "\n").encode("utf-8"), verify_only)
    print(f"FreeType {VERSION}: verified {len(hashes)} unmodified source files; tree SHA256 {tree_sha}")


def prepare_font(game_dir, verify_only):
    if not FONT.exists():
        if verify_only:
            raise RuntimeError(f"Missing bundled font: {FONT}")
        original = game_dir / "SETUP_ZHTW" / FONT_NAME
        if original.is_symlink() or not original.is_file() or digest(original) != FONT_SHA256:
            raise RuntimeError(f"Game font missing or differs from the verified original: {original}")
        install_bytes(FONT, original.read_bytes(), False)
    if FONT.is_symlink() or not FONT.is_file() or digest(FONT) != FONT_SHA256:
        raise RuntimeError(f"Modified bundled font, left unchanged: {FONT}")
    download_pinned(OFL, OFL_URL, OFL_SHA256, verify_only)
    license_path = ROOT / "assets/licenses/NotoSerifCJKtc-OFL.txt"
    # Font copyright/trademark values are from this exact font's sfnt name table.
    notice = ("Noto Serif CJK TC Bold, version 1.001\n"
              "Copyright © 2017 Adobe Systems Incorporated (http://www.adobe.com/).\n"
              "Noto is a trademark of Google Inc.\n\n").encode("utf-8")
    license_bytes = notice + OFL.read_bytes()
    install_bytes(license_path, license_bytes, verify_only)
    metadata = {
        "family": "Noto Serif CJK TC", "style": "Bold", "version": "1.001", "format": "OpenType/CFF",
        "file": "assets/fonts/" + FONT_NAME, "sha256": FONT_SHA256, "size_bytes": FONT.stat().st_size,
        "source": "StudySteadyR18/SETUP_ZHTW/" + FONT_NAME,
        "source_description": "Exact font supplied with the user's game; copied unchanged; no host font installation",
        "modifications": [], "license": "OFL-1.1", "license_file": "assets/licenses/NotoSerifCJKtc-OFL.txt",
        "copyright": "Copyright © 2017 Adobe Systems Incorporated (http://www.adobe.com/).",
        "license_source": OFL_URL, "license_source_commit": OFL_COMMIT,
        "license_source_sha256": OFL_SHA256, "packaged_license_sha256": hashlib.sha256(license_bytes).hexdigest(),
        "verification": "python3 tools/sdk/setup_sdl_fonts.py --verify",
    }
    install_bytes(FONT_PROVENANCE, json_bytes(metadata), verify_only)
    credits = ("This software is based in part on the work of the FreeType Team.\n"
               "Portions of this software are copyright © 2026 The FreeType Project\n"
               "(https://freetype.org). All rights reserved.\n"
               "FreeType 2.14.3 is used under the FreeType License (FTL).\n"
               "See FreeType-FTL.txt and FreeType-LICENSE.txt.\n\n"
               "Noto Serif CJK TC Bold is bundled unchanged under SIL Open Font License 1.1.\n"
               "See NotoSerifCJKtc-OFL.txt and ../fonts/provenance.json.\n").encode()
    install_bytes(ROOT / "assets/licenses/FONT-CREDITS.txt", credits, verify_only)
    print(f"{FONT_NAME}: verified unmodified {FONT.stat().st_size:,}-byte font, SHA256 {FONT_SHA256}; OFL 1.1 included")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify", action="store_true", help="Verify existing inputs only; no downloads or writes")
    parser.add_argument("--freetype-only", action="store_true", help="Prepare the engine dependency without a game or optional compatibility font")
    parser.add_argument("--game-dir", type=Path, default=ROOT / "StudySteadyR18",
                        help="Original game directory, needed only for initial font copy")
    args = parser.parse_args()
    prepare_freetype(args.verify)
    if not args.freetype_only:
        prepare_font(args.game_dir, args.verify)


if __name__ == "__main__":
    main()
