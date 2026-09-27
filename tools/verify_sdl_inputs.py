#!/usr/bin/env python3
"""Audit read-only SDL migration inputs against their existing SHA-256 baselines.

This reads local files only. It does not download dependencies, change originals,
or inspect/copy game resources. Reports go to artifacts/sdl3 by default.
"""

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import tarfile

import setup_sdl3

ROOT = Path(__file__).resolve().parents[1]


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def verify_files(name, baseline, entries):
    mismatches = []
    for relative, expected in entries:
        path = ROOT / relative
        if path.is_symlink() or not path.is_file() or sha256(path) != expected:
            mismatches.append(relative)
    return {"name": name, "status": "FAIL" if mismatches else "PASS", "files_checked": len(entries),
            "mismatches": mismatches, "baseline": baseline,
            "baseline_sha256": sha256(ROOT / baseline)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=ROOT / "artifacts/sdl3")
    args = parser.parse_args()
    rows = []
    baseline = "artifacts/official-sdk-migration/official-input-manifest.json"
    manifest = json.loads((ROOT / baseline).read_text())
    sdk = verify_files("EntisGLS supplied distribution", baseline,
        [(entry["path"], entry["sha256"]) for entry in manifest["files"]])
    recorded_paths = {entry["path"] for entry in manifest["files"]}
    actual_paths = {path.relative_to(ROOT).as_posix() for path in (ROOT / "EntisGLS").rglob("*") if path.is_file()}
    sdk["unrecorded_files"] = sorted(actual_paths - recorded_paths)
    if sdk["unrecorded_files"]:
        sdk["status"] = "FAIL"
    rows.append(sdk)

    sdl = {"name": "SDL3 3.4.16", "status": "PASS", "files_checked": 0,
           "baseline": "Official release archive and vendor/sdl3-provenance.json",
           "commit": setup_sdl3.COMMIT, "archive_sha256": setup_sdl3.ARCHIVE_SHA256}
    try:
        if not setup_sdl3.ARCHIVE.is_file():
            raise RuntimeError("Cached SDL release archive is missing; run tools/setup_sdl3.py before this offline audit")
        if sha256(setup_sdl3.ARCHIVE) != setup_sdl3.ARCHIVE_SHA256:
            raise RuntimeError("SDL release archive hash mismatch")
        with tarfile.open(setup_sdl3.ARCHIVE, "r:gz") as archive:
            setup_sdl3.verify(archive)
            sdl["files_checked"] = sum(1 for _ in setup_sdl3.archive_files(archive))
    except Exception as error:
        sdl.update(status="FAIL", error=str(error))
    rows.append(sdl)

    dependency_manifest = "vendor/official-entis-dependencies.json"
    dependencies = json.loads((ROOT / dependency_manifest).read_text())
    for dependency in dependencies:
        directory = ROOT / dependency["directory"]
        # The recorded baseline sorts pathlib Paths component-by-component,
        # which differs from sorting serialized path strings around slashes.
        paths = sorted(path for path in directory.rglob("*") if path.is_file())
        tree = hashlib.sha256()
        for path in paths:
            tree.update((sha256(path) + "  " + path.relative_to(directory).as_posix() + "\n").encode())
        tree_hash = tree.hexdigest()
        matches = tree_hash == dependency["source_tree_sha256"] and len(paths) == dependency["source_file_count"]
        rows.append({"name": "Official " + dependency["name"], "status": "PASS" if matches else "FAIL",
            "files_checked": len(paths), "expected_files": dependency["source_file_count"],
            "tree_sha256": tree_hash, "expected_tree_sha256": dependency["source_tree_sha256"],
            "tree_order": "pathlib component order, matching the recorded baseline",
            "commit": dependency["commit"], "baseline": dependency_manifest,
            "baseline_sha256": sha256(ROOT / dependency_manifest)})

    imported = "vendor/kirikiroid2/provenance.json"
    motion = json.loads((ROOT / imported).read_text())
    rows.append(verify_files("Imported MotionPlayer / PSB snapshot", imported,
        [("vendor/kirikiroid2/" + name, digest) for name, digest in motion["files"].items()]))
    extras = "vendor/motion-deps/provenance.json"
    motion_dependencies = json.loads((ROOT / extras).read_text())
    rows.append(verify_files("Imported TJS and motion dependency snapshot", extras,
        [(name, value["sha256"]) for name, value in motion_dependencies["files"].items()]))

    success = all(row["status"] == "PASS" for row in rows)
    result = {
        "verified_at_utc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS" if success else "FAIL",
        "total_file_checks": sum(row["files_checked"] for row in rows),
        "inputs": rows,
        "scope": "Byte integrity of the recorded SDK/vendor input snapshots; generated overlays and project adapters are intentionally outside these snapshots.",
        "provenance_note": "The supplied EntisGLS directory is compared with its migration baseline. Kirikiroid2 imports are compared with their recorded working-tree bytes, not asserted to be pristine upstream sources.",
        "game_resources_read": False,
        "apk_rebuilt": False,
    }
    args.output_dir.mkdir(parents=True, exist_ok=True)
    report = args.output_dir / "input-integrity.json"
    report.write_text(json.dumps(result, indent=2) + "\n")
    lines = ["# SDL3 迁移输入完整性", "", f"校验时间：{result['verified_at_utc']}。结果：**{result['status']}**。", "",
             "| 输入 | 校验文件数 | 结果 |", "| --- | ---: | --- |"]
    for row in rows:
        lines.append(f"| {row['name']} | {row['files_checked']} | {row['status']} |")
    lines.extend(["", f"共完成 {result['total_file_checks']} 项文件字节校验。详情及基线 SHA-256 见 `input-integrity.json`。", "",
        "EntisGLS 按官方包迁移前保存的 1805 文件快照比较；SDL3 按固定官方发布归档比较；Loquaty、TinyGLTF 按记录的完整源码树摘要比较。MotionPlayer/PSB/TJS 及其依赖按导入时字节快照比较，其来源本来就可能含提供者的本地修改。", "",
        "项目适配代码和 build 中的生成副本不属于这些只读输入。本次审计没有读取游戏资源、改写输入或重新生成 APK。", "",
        "复查命令：`python3 tools/verify_sdl_inputs.py`。"])
    (args.output_dir / "input-integrity.md").write_text("\n".join(lines) + "\n")
    print(f"Input integrity: {result['status']}; {result['total_file_checks']} file checks; {report}")
    if not success:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
