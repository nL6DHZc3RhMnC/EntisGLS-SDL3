#!/usr/bin/env python3
"""Install or verify the pinned, unmodified upstream SDL3 source distribution.

The official release archive is cached under build/downloads. An existing source
tree is verified, never overwritten. No system SDL installation is used.
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
import shutil
import tarfile
import tempfile
import urllib.request


VERSION = "3.4.16"
TAG = f"release-{VERSION}"
COMMIT = "fa2c02bb6e21974a89ea9824bc53c9932abe5f9c"
ARCHIVE_NAME = f"SDL3-{VERSION}.tar.gz"
ARCHIVE_SHA256 = "7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68"
ARCHIVE_URL = f"https://github.com/libsdl-org/SDL/releases/download/{TAG}/{ARCHIVE_NAME}"
SOURCE = ROOT / "vendor/sdl3"
PROVENANCE = ROOT / "vendor/sdl3-provenance.json"
ARCHIVE = ROOT / "build/downloads" / ARCHIVE_NAME
METADATA = {
    "name": "SDL3",
    "version": VERSION,
    "repository": "https://github.com/libsdl-org/SDL",
    "release_url": f"https://github.com/libsdl-org/SDL/releases/tag/{TAG}",
    "release_published_at": "2026-09-02T16:33:19Z",
    "tag": TAG,
    "commit": COMMIT,
    "archive_url": ARCHIVE_URL,
    "archive_sha256": ARCHIVE_SHA256,
    "source_directory": "vendor/sdl3",
    "license": "Zlib",
    "license_file": "vendor/sdl3/LICENSE.txt",
    "upstream_modifications": [],
    "verification": "python3 tools/sdk/setup_sdl3.py --verify",
}


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def get_archive():
    ARCHIVE.parent.mkdir(parents=True, exist_ok=True)
    if not ARCHIVE.exists():
        with tempfile.NamedTemporaryFile(dir=ARCHIVE.parent, delete=False) as out:
            pending = Path(out.name)
            try:
                request = urllib.request.Request(ARCHIVE_URL, headers={"User-Agent": "StudySteady-SDL3-setup"})
                with urllib.request.urlopen(request, timeout=60) as response:
                    shutil.copyfileobj(response, out)
                out.flush()
                if digest(pending) != ARCHIVE_SHA256:
                    raise RuntimeError("Downloaded SDL3 archive checksum does not match the pinned official release")
                pending.replace(ARCHIVE)
            finally:
                pending.unlink(missing_ok=True)
    if digest(ARCHIVE) != ARCHIVE_SHA256:
        raise RuntimeError(f"SDL3 archive checksum mismatch; inspect {ARCHIVE}")


def archive_files(archive):
    for member in archive.getmembers():
        parts = PurePosixPath(member.name).parts
        if not parts or parts[0] != f"SDL3-{VERSION}" or ".." in parts:
            raise RuntimeError(f"Unexpected archive path: {member.name}")
        if member.isdir():
            continue
        if not member.isfile() or len(parts) < 2:
            raise RuntimeError(f"Unsupported archive entry: {member.name}")
        yield member, Path(*parts[1:])


def verify(archive):
    expected = set()
    for member, relative in archive_files(archive):
        expected.add(relative.as_posix())
        path = SOURCE / relative
        if path.is_symlink() or not path.is_file():
            raise RuntimeError(f"Missing or unexpected SDL3 source file: {path}")
        with archive.extractfile(member) as stream:
            upstream_hash = hashlib.sha256(stream.read()).hexdigest()
        if digest(path) != upstream_hash:
            raise RuntimeError(f"Modified SDL3 source file: {path}")
    actual = {path.relative_to(SOURCE).as_posix() for path in SOURCE.rglob("*") if path.is_file() or path.is_symlink()}
    extra = actual - expected
    if extra:
        raise RuntimeError(f"Unexpected files in SDL3 source tree: {sorted(extra)}")
    if not PROVENANCE.is_file() or json.loads(PROVENANCE.read_text()) != METADATA:
        raise RuntimeError(f"Missing or mismatched provenance: {PROVENANCE}")
    print(f"SDL {VERSION}: verified {len(expected)} unmodified upstream files; archive SHA256 {ARCHIVE_SHA256}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify", action="store_true", help="Verify only; do not create a missing source tree")
    args = parser.parse_args()
    get_archive()
    with tarfile.open(ARCHIVE, "r:gz") as archive:
        if not SOURCE.exists():
            if args.verify:
                raise RuntimeError(f"SDL3 sources are missing; run {Path(__file__).name} without --verify")
            SOURCE.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.TemporaryDirectory(prefix=".sdl3-", dir=SOURCE.parent) as temporary:
                staged = Path(temporary) / "source"
                staged.mkdir()
                for member, relative in archive_files(archive):
                    path = staged / relative
                    path.parent.mkdir(parents=True, exist_ok=True)
                    with archive.extractfile(member) as stream, path.open("wb") as out:
                        shutil.copyfileobj(stream, out)
                    path.chmod(member.mode & 0o777)
                staged.rename(SOURCE)
            PROVENANCE.write_text(json.dumps(METADATA, indent=2) + "\n")
        verify(archive)


if __name__ == "__main__":
    main()
