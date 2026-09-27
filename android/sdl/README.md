# EntisGLS Launcher: Android SDL3 shell

This shell subclasses unmodified SDL 3.4.16 `SDLActivity`. SDL owns the window,
input, audio and application lifecycle. Java manages the game library, SAF
imports and native path arguments; native code recognizes configurations and
checks engine compatibility. Importing an EXE copies its bytes for configuration
inspection; the launcher never executes that Windows program.

The visible app is **EntisGLS Launcher**, version code 5 (`0.3.1-dev`). Its
Android application ID is `io.entisgls.launcher`, and its Activities use the
`io.entisgls.launcher.sdl` Java package. Android treats this as a separate app
from the previous `io.studysteady.port` package. Existing installations and their
data remain in that old app; this launcher does not automatically migrate them.
Import game resources into the new app to populate its library.

## Library and native handoff

Each import creates a new `<external-files>/library/<UUID>/` entry:

- `game/`: the selected directory's complete recursive contents, preserving names.
- `entry.json`: library ID and display name from the selected directory.
- `import-receipt.json`: relative filenames, sizes and SHA-256 recorded while copying.

The launcher lists games and remembers the selected entry. Importing another
game never replaces an earlier entry or writes to save directories. Existing
`<external-files>/game` remains selectable as the legacy entry, without moving
its resources or deleting old saves. UUIDs identify imported library entries;
native configuration identity determines the game's independent save location.

| Native argument | Java value |
| --- | --- |
| `--storage-dir` | `getExternalFilesDir(null)` |
| `--game-dir` | selected `<external-files>/library/<UUID>/game`, or legacy `game` |
| `--local-dir` | `getFilesDir()`; native appends its per-game data path |
| `--legacy-local-data` | supplied only for the old `game/` entry |
| `--psb-key` | supplied only when this library entry has a saved manual PSB override |
| Packaged assets | empty native `assetsRoot` uses SDL's APK asset IO |

Native code alone decides whether legacy saves can be copied to the recognized
game's private data directory; Java does not assume all games share a format.
The source save directory stays intact.

## Per-game E-mote settings

**设置 PSB 解码参数** accepts an unsigned 32-bit decimal or `0x`/`0X`
hexadecimal value. Zero is an explicit value. Clearing the field or selecting
**恢复自动** removes the override. Each imported UUID and the old `legacy` entry
has its own setting in the application's private SharedPreferences; game resources
and XML files are not modified. A manual setting takes priority over XML `psb_key`;
without either, native code looks for a driver-supplied parameter and verifies it
against the PSB being loaded. Entering a value does not by itself validate it.

Existing NOA-only imports can use **补充 E-mote 驱动文件** to select the original
game's DLL without reimporting all archives. The launcher copies it to the selected
app-owned game directory as `emotedriver.dll`, limiting it to 256 MiB and checking
MZ/PE signatures plus the DLL flag. It never executes the DLL or changes the source
provider. Native code performs E-mote recognition and PSB validation at load time.
An existing imported driver is backed up as `.emotedriver-backup-*.bin` before the
new completed copy atomically replaces it. Parameter settings and saves are kept.
Keep the launcher page open for this short copy; Activity destruction cancels it.

## Import behavior

Select the root of one game, not a parent containing multiple games. Java checks
for at least one nonempty root-level configuration/resource candidate:
`cotopha.xml`, `entis-launcher.xml`, `.exe`, `.csx` or `.noa`. This is only a basic
selection check, **not** proof that the engine supports that game. Native code
validates the configuration and required execution image at launch. There is no
fixed archive count or list of StudySteady filenames.

SAF import recursively copies all files and directories, including loose scripts,
configuration and fonts. Copies are new app-owned ordinary files. Modes 0700 for
directories and 0600 for files are requested where supported; emulated storage
with fixed modes is checked using actual app IO. The importer streams large files,
checks declared lengths, synchronizes completed writes, checks app readability
and records SHA-256. Empty ancillary files are preserved. It rejects unsafe names,
case-insensitive duplicate names, cyclic directories and excessively deep trees.

A completed directory is atomically renamed into a new library entry. A journal
recovers an interruption during commit; an interrupted copy can be restarted and
its positively identified temporary directory is cleaned on the next import.
This is recovery/restart, not resuming at a partially copied byte offset. Old
single-game import journals remain recoverable. Imports never delete original
provider files, other library entries, backup directories or saves.

Import state survives Activity recreation in-process. Keep the import page open
for long copies: this is not a foreground service, so Android may stop background
work. Persistent URI grants are retained when supported. Gameplay reads the
app-owned copy and no longer depends on the provider. No network or broad storage
permission is declared.

## Build and diagnostics

`python3 tools/build_sdl_android.py` configures, compiles and packages the native
runtime. `python3 tools/build_sdl_apk.py --check-shell` validates Java, DEX and the
manifest without publishing an APK. A shell build is not a device/runtime test.
The default output is `artifacts/entisgls-launcher-arm64-dev.apk`; previous
StudySteady APKs are protected from overwrite. Assets contain compatibility
profiles, licenses and optional launcher fonts. There is no root game configuration
or bundled NOA/CSX/EXE. Both build scripts accept `--without-bundled-fonts` to omit
the optional fonts; game-supplied fonts remain supported by the native runtime.

`python3 android/sdl/tests/test_import_discovery.py` exercises the production Java
discovery and path checks against a host-side fake document provider, including
recursive and Unicode names, zero-byte ancillary files, cancellation, duplicate
names, unsafe paths, cycles, and staging cleanup with links. It does not simulate
Android permissions, JSON commit recovery or Activity lifecycle.

`python3 android/sdl/tests/test_psb_settings.py` checks the production setting
parser and DLL-copy helper on a host JVM with fake preferences/provider: unsigned
values, per-entry isolation, clearing overrides, failed commits, PE rejection,
replacement backups, cancellation, and symlink targets. It does not test Android
dialogs, permissions, process recreation, or PSB decoding itself.

The development launcher accepts `start_game=true`, optional `game_id` (a library
UUID or `legacy`), plus the bounded diagnostics `probe`, `exit_after` (1–300) and
`capture_after` (0–300). Supported probes are `self-test`, `window-probe`,
`emote-probe`, `image-export-probe`, and `opening-probe`. The screenshot destination
is fixed to internal `files/sdl-frame.png`. Unknown probe names, arbitrary command
lines and arbitrary output paths are not forwarded; diagnostics are ignored by
non-debuggable builds.
