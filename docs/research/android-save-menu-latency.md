# Android save/load menu latency

Investigated on 2026-09-28 using a Mi 10 Pro with Android 13, the user's local
StudySteady resources accessed through SAF, and the existing CI APK
`dev-15.1-dc4e067e`. The installed APK SHA-256 was
`b02b08e9c238546aa4fb50c289f84c0bfca645b70bc435f980c93fa79a7d130c`.
No local build was used for this investigation.

## Device evidence

In-game SAVE and LOAD commands both first invoke the game's temporary save of
slot 9997, then load `wm_saveload.noa` and populate nine slots. The measurements
below used the first dialogue scene and an empty visible save page. No manual
save slot was selected or overwritten; the game itself created its temporary
`save9997.bmp`.

Two eight-second Simpleperf captures included CPU and off-CPU call stacks at
400 Hz. The exact APK libraries and Build IDs were used for symbolization.
Only the script thread's relevant intervals were compared; overlapping waits
from all the audio/render/worker threads were not added together. Input markers
and skin logs use the same monotonic clock as the profile.

| Stage | First LOAD | Subsequent SAVE |
| --- | ---: | ---: |
| Temporary save, including opening and thumbnail staging | about 1,084 ms | about 1,020 ms |
| Save/load skin lookup and decoding | 420 ms | 347 ms |
| Wait after skin loading until first slot query | about 538 ms | about 538 ms |
| Nine empty slot checks and their UI scheduling | about 726 ms | about 715 ms |

Temporary-save call stacks place approximately 190–230 ms in opening the
atomic path, 259 ms in thumbnail staging, and 521–546 ms in body replacement.
Actual serialization was about 23–25 ms; `fsync` accounted for only about
8–10 ms of sampled time. Most of the delay was provider/Binder work: repeated
path resolution, directory enumeration, creation, deletion and reopening.
This behavior persisted on the second opening.

Skin timings come from the application's phase logs. Other figures describe
sampled call-stack intervals, not exact function-entry/exit instrumentation.
The first capture lost 243 kernel samples and the second lost 9; neither lost
userspace samples. These intervals do not establish the precise first visible
or first interactive frame. The 538 ms wait includes command/main-thread
scheduling; the script also has interface animation, but the profile alone
does not classify all of this wait as animation.

## Targeted change

Previously, `Open(Create)` created an empty private file, `StagePrefix` created
another for the thumbnail, and `Replace` created a third for thumbnail plus
context. Each transition added provider calls and invalidated directory caches.
The common atomic-file layer now reuses the existing unpublished file for both
stages. It also resolves the save-directory boundary once per logical open,
passing the resulting candidate status and canonical parent through to opening.
It does not cache permission checks across separate operations.

The commit still requires `fsync`, closing the writer, reopening for complete
body read-back and length verification, and the provider's recoverable rename.
Already-published files still use a separate copy. Failed thumbnail writes or
failed body validation cannot be published by a later ordinary write, truncate,
prefix edit or close; complete validated retries are required.

CI regression sources assert a single temporary creation for thumbnail plus
body, and cover partial writes, corrupt read-back, failed opens/renames, retries,
scope failures and cleanup. The POSIX failure-injection executable is registered
in the native CI checks with assertions enabled in Release builds.

Nine-slot metadata checks remain a secondary cost: roughly 384–405 ms of their
sampled time was fresh root metadata queries despite directory-cache hits.
The original game's animations and skin decoding also remain. No reduction in
total device latency is claimed until the changed APK is built and measured;
this change's build/test validation is delegated to GitHub Actions.

Local raw profiles, screenshots, logs and detailed reports are under
`artifacts/save-ui-diagnosis/` and are excluded from version control.
