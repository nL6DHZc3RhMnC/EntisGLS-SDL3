# EntisGLS Launcher: Android SDL3 shell

This shell subclasses unmodified SDL 3.4.16 `SDLActivity`. SDL owns the window,
input, audio and application lifecycle. Java maintains the game library and
Android document-tree grants. Native code recognizes configurations and checks
engine compatibility; Windows EXE/DLL files are read as data and never executed.

The application ID is `io.entisgls.launcher`, version code 5 (`0.3.1-dev`). It is
independent of the previous `io.studysteady.port` app and cannot read that app's
private data automatically.

## Direct game folders

**添加游戏文件夹** opens Android's system directory picker. The launcher retains
read/write permission for each selected tree and stores only its URI, name and
library UUID in `<internal-files>/library/<UUID>/entry.json`. It does not copy the
game into app storage. Selecting the same URI again reuses its library entry and
settings, including after reauthorizing it. Adding another game does not revoke
previous games' grants. Existing copied entries in `<external-files>/library/`
and the legacy `<external-files>/game` remain selectable.

Select the root of one game, directly containing `cotopha.xml`,
`entis-launcher.xml`, an EXE, CSX or NOA file. This is only a basic selection
check, not a promise that every game is supported. The provider must support
read/write access, seekable file descriptors and rename operations for saving.
Local device storage is suitable; a provider offering only streamed cloud files
is rejected instead of silently copying the game. If permission is revoked or
the directory is moved/unavailable, select the folder again.

Directory selection uses Android's [Storage Access Framework](https://developer.android.com/training/data-storage/shared/documents-files).
No all-files permission is requested, and document IDs are not interpreted as
filesystem paths. Names are resolved through the authorized provider; traversal,
ambiguous case-only names and cyclic/deep paths are rejected.

## Native handoff and IO

| Native argument | Java value |
| --- | --- |
| `--storage-dir` | `getExternalFilesDir(null)` |
| `--game-dir` | virtual `/__entis_saf__/<UUID>` for a selected tree; actual path for old copied games |
| `--local-dir` | `getFilesDir()` for per-game launcher settings/cache |
| `--legacy-local-data` | supplied only for the old `game/` entry |
| `--psb-key` | supplied when this entry has a manual override |

`GameActivity.getDocumentTreeAccess()` supplies the selected provider to native
code before discovery. The shared game IO interface serves XML/EXE discovery,
resource identity, DLL inspection and the SDK's `storage://game` file opener.
Reads and writes use seekable descriptors directly: an archive read does not
cross JNI for every block and does not require an archive-sized temporary copy.
Enumeration, creation, removal and rename use scoped provider operations. Other
platforms retain their ordinary native filesystem implementation.

## Saves

The common launcher always maps saves to `$(CURRENT)\savedata`, represented
internally as `storage://game/savedata`. On Android this is a real `savedata`
child of the chosen directory. The directory is created when needed; existing
saves there are used directly. No old application-data saves are searched or
migrated. PSB settings and caches remain private to the launcher.

Serialized saves are staged and validated before replacement. Native files use
atomic rename. SAF replacement journals the operation in app-private storage,
retains the previous document during publication, and recovers interrupted
renames on next access. Because providers do not offer universal atomic replace,
interrupted completion may retain an `.entis-save-backup-*` file alongside the
save. Failed saves report an error; they do not truncate the previous slot first.

## E-mote settings

**设置 PSB 解码参数** accepts an unsigned 32-bit decimal or hexadecimal value.
Zero is explicit. Clearing the field restores XML/driver discovery. Manual
settings live in private SharedPreferences and take priority over XML `psb_key`.
Discovered parameters are verified against the actual encrypted PSB.

**补充 E-mote 驱动文件** validates a selected DLL's size and PE header. For linked
trees it adds a uniquely named `entis-emotedriver-<UUID>.dll` to the selected
folder without replacing an existing DLL. For old app-owned copies it retains
the previous import/backup behavior. No DLL code is executed.

## Checks

```sh
python3 tests/integration/android/test_import_discovery.py
python3 tests/integration/android/test_psb_settings.py
python3 tests/integration/android/test_document_tree.py
python3 tools/build/build_sdl_android.py
```

The host JVM provider harness covers opaque document IDs, permission failures,
path rejection, descriptor ownership, replacement failures and restart recovery.
Native tests cover virtual resource discovery and game-relative save creation
and serialization. These checks do not replace real Android provider/device tests.
