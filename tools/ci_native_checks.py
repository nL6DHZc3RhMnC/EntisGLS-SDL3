#!/usr/bin/env python3
"""Run asset-free native launcher checks in a configured desktop build.

The CSX smoke program is authored in this repository and serialized by the
actual Cotopha SDK. SDL's dummy drivers prevent GUI or sound-device access.
These checks do not validate graphics, commercial games, or hardware audio.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
TARGETS = (
    "studysteady_sdl",
    "launcher_config_test",
    "psb_key_resolver_test",
    "psb_key_settings_test",
    "game_save_directory_test",
    "sdl_system_test",
    "make_csx_fixture",
)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--skip-build", action="store_true")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    build = args.build_dir.resolve()
    if not (build / "CMakeCache.txt").is_file():
        parser.error("--build-dir must contain a configured native desktop build")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    if not args.skip_build:
        subprocess.run(
            [args.cmake, "--build", str(build), "--parallel", str(args.jobs),
             "--target", *TARGETS],
            check=True,
            cwd=ROOT,
        )

    environment = os.environ.copy()
    environment.update(SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
    results: list[dict] = []

    def run(name: str, command: list[str | Path], expected: str,
            expected_code: int = 0) -> str:
        command = [str(part) for part in command]
        print(f"\n[{name}] {shlex.join(command)}", flush=True)
        completed = subprocess.run(
            command, cwd=ROOT, env=environment, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            errors="replace", timeout=60,
        )
        print(completed.stdout, end="", flush=True)
        result = {
            "name": name,
            "exit_code": completed.returncode,
            "passed": completed.returncode == expected_code and expected in completed.stdout,
            "output": completed.stdout,
        }
        results.append(result)
        if not result["passed"]:
            raise RuntimeError(
                f"{name} failed: expected exit {expected_code} and output {expected!r}"
            )
        return completed.stdout

    passed = False
    fixture_parent = ROOT / "build/ci-checks"
    fixture_parent.mkdir(parents=True, exist_ok=True)
    try:
        with tempfile.TemporaryDirectory(prefix="native-", dir=fixture_parent) as temp:
            fixtures = Path(temp)
            run("launcher configuration", [build / "launcher_config_test", fixtures],
                "Launcher configuration PASS:")
            run("PSB discovery and cache", [build / "psb_key_resolver_test"],
                "PSB key resolver PASS:")
            run("per-game PSB settings", [build / "psb_key_settings_test", fixtures],
                "PSB settings PASS:")
            run("game directory saves", [build / "game_save_directory_test"],
                "Game directory saves PASS:")
            run("native and document-tree SDK IO", [build / "sdl_system_test", fixtures],
                "SDL system services and unified path routing: PASS")
            game = fixtures / "original-game-fixture"
            run("real CSX serialization and execution", [build / "make_csx_fixture", game],
                "CSX fixture PASS:")

            launcher = build / "EntisGLSLauncher.app/Contents/MacOS/EntisGLSLauncher"
            if not launcher.is_file():
                launcher = build / "entisgls-launcher"
            if not launcher.is_file():
                raise RuntimeError("No native desktop launcher found in the build directory")
            assets = fixtures / "empty-assets"
            storage = fixtures / "storage"
            data = fixtures / "app-data"
            for directory in (assets, storage, data):
                directory.mkdir()
            arguments = [
                launcher, "--game-dir", game, "--assets-dir", assets,
                "--storage-dir", storage, "--local-dir", data,
                "--exit-after", "10",
            ]
            output = run("headless full launcher", arguments,
                         "Legacy main returned without uncaught error")
            if "entry=adventure.csx; profile=;" not in output:
                raise RuntimeError("The asset-free game did not use the generic profile/custom entry")
            if not (game / "savedata").is_dir() or list((data / "games").glob("*/savedata")):
                raise RuntimeError("The launcher did not use the selected game's savedata directory")

            (game / "cotopha.xml").write_text("<script src='unsupported.lqs'/>\n", encoding="utf-8")
            run("unsupported runtime rejection", [*arguments, "--inspect-game"],
                "traditional Cotopha .csx only", expected_code=1)
            passed = True
    finally:
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.write_text(json.dumps({
                "passed": passed,
                "commercial_game_resources": False,
                "sdl_video_driver": "dummy",
                "sdl_audio_driver": "dummy",
                "checks": results,
            }, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("\nAsset-free native CI checks PASS", flush=True)


if __name__ == "__main__":
    main()
