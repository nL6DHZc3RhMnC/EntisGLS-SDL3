# Cold actor first-frame publication

The first real two-actor save exposed a rendering issue that isolated actor
serialization tests did not cover. `artifacts/current-save0016.bmp` is a
70,924-byte BMP/save container. Its ERISAN `context ` record decodes to 93,697
bytes (`build/analysis/save0016-context.bin`). The actual original Emote wire
contains:

| File | Viewport | Target scale | Target coordinate | Active timeline / queue |
| --- | --- | --- | --- | --- |
| nak_d.psb | 1920 × 1380 | float32 0.95 | 0, 765 | empty / empty |
| haz_h.psb | 1920 × 1380 | 1 | 0, 744 | empty / empty |

The UTF-16 filenames begin at decoded context offsets 0x8138 and 0x8ad3.
Their three preceding float32 fields contain those real target values. The
saved vertical coordinates were therefore present, not omitted or reset.

The host regression `tools/motion_cold_transform_probe.cpp` loads both real
PSBs in both orders, applies the actual saved targets, explicitly publishes
with progress(0), then interleaves 120 actual frames. Controller and published
transforms remain correct in both orders. Log:
`build/analysis/cold-actor-transform-host.txt`.

The device trace gives the more important scheduling evidence:
`artifacts/game-actors-transform-trace.txt`, 08:28:40.728, actor 3 / haz_h.psb:

- target and controller: scale 1, coordinate 0, 744;
- published Player: scale 1, coordinate 0, 0, frame count 0;
- base Sprite position: 1392,1079, centre 960,1179, zoom 1;
- first draw bounds: -551,-1414,711,2169 (still the unshifted geometry).

The actor whose base Sprite is at x=1392 is the misplaced right-hand image.
The other actor had already received a timer tick and published its correct
y=765. No assumption about which filename corresponds to which visible
character is needed. Static/snapshot display can retain that first wrong frame.

## Fix and limits

Commit now publishes the restored targets/timeline through real Engine
progress(0). The public GLES render boundary also calls progress(0) when the
Engine is dirty, covering ordinary SetCoord/SetScale followed immediately by
drawing. This uses the same zero-time publication as the existing validated
clone and structured-state restore paths; it does not invent a time step or
change saved targets/timeline selection. The original ststeady drawing wrapper
calls native Player vtable slot +280; no claim is made here that its driver
internals have already been proved to perform the identical publication call.

A separate continuous-update defect was confirmed directly in SDK source:
`SGLSprite::Refresh` returns immediately when GetFrameBuffer is null
(`sglx_sprite.cpp`, line 2673). EmoteSprite owns an attached rendered SGLImage,
usually with no SDK framebuffer. Calling Refresh after progress or writing
pixels did not mark the cached parent scene dirty. OpenPlayer, AdvanceMotion,
and DrawMotion now call NotifyUpdate, which posts the actual dirty rectangle
to the parent. Bounded first/30th Advance and Draw logs record transforms,
parent identity, image alpha count, and rendered RGBA FNV hash to verify the
live game redraws instead of assuming the engine's CPU clock implies animation.

The new `study_motion_get_transform` diagnostic reads controller current values
and published Player values without stepping any controller or timeline.
The accompanying trace also computes geometry bounds for diagnosis.

## Original timeline completion behavior

Keeping the last timeline name was considered and rejected based on the actual
Windows executable. In AdvanceTime at 0x82E530, the completed/empty-queue branch
at 0x82E7D2 pushes the empty UTF-16 string at 0x95D2DC, loads the current timeline
string at adjusted-this+0x80C, then calls string assignment 0x40AE60 at
0x82E7DD. The queued branch assigns the next name to the same string at
0x82E7BD. Thus the port's timeline_.clear() matches the original behavior;
changing it would not address this first-frame bug.

## Verification status

The changed bridge, opaque runtime and GPU regression compile as ARM64 API 29
objects. `build/analysis/motion-cold-first-frame-gles-probe` links against the
real TJS/NCB/Player/GLES stack. Before the existing GPU/state suite, it creates
a fresh actor, sets scale 0.95 and y=765, draws without any timer advancement,
checks published values with frame count still zero, and compares a second
zero-time draw byte-for-byte. It also requires nonzero rendered alpha.

The independent regression passed on the actual Xiaomi 10 Pro, Android 13,
with an ES3 parent context. Full command, merged stdout/stderr, exit code and
elapsed time are in `artifacts/motion-cold-first-frame-gles-arm64.txt`:

- no-timer first draw: frame 0, published y=765, 710,239 nonzero alpha pixels;
- second zero-time draw: byte-identical output;
- original shared-parent image: 124,871 alpha pixels, CRC32 054db89a;
- three real timelines: nine rendered frames with nine distinct CRCs;
- active timeline cursor 5 restored to 5, inactive timeline remained inactive;
- borrowed output texture 2 remained valid, and worker shutdown preserved the
  parent context. Exit code 0.

The probe only pushed and ran its own executable against the existing
`/data/local/tmp/studysteady-haz_a.psb`. It did not install an APK, manipulate
the game Activity, or clear logcat. The usual shared-frame output is retained
as `artifacts/motion-cold-first-frame-gles.pam`.

The subsequent real-game cold load of slot 16 also passed, with both actors
visibly in the correct positions: `artifacts/game-actors-cold-fixed.txt/png`.
At 08:40:38.772 the right-hand actor's first draw already published y=744 while
its base motion cursor was still zero. Both actors then reached their 30th
Advance and Draw callbacks. Real output pixels changed:

| Actor | First RGBA FNV32 | 31st draw RGBA FNV32 |
| --- | --- | --- |
| nak_d | e6baf3d2 | 7cf7b480 |
| haz_h | b0059e33 | 0bf0e971 |

The logs report visible sprites, actual parent pointers, and null local
framebuffers. Together with those changed pixels, this directly covers the
NotifyUpdate integration; the independent GPU test alone did not prove it.
Subsequent dialogue advancement and new save/load cycles are separate checks.

## Why the diagnostic base-motion cursor stops near 61

`artifacts/game-actors-continued-fixed.txt` keeps printing 62.340 and 61.980
for the two actors even while named timelines, positions and geometry change.
This is expected for the field currently named `frame` in that diagnostic:
it is Player::getFrameTickCount, the cursor of the PSB base motion, not elapsed
Engine animation time or the selected metadata timeline cursor.

Both actual PSBs contain
`object.all_parts.motion["タイムライン構造"]` with `lastTime=61`, `loopTime=-1`,
and no Player-level parameterization. Player::frameProgress adds dt to the raw
cursor while playing (`PlayerFrameProgress.cpp:1064`). When it reaches/passes
61 with no loop, the function sets `_allplaying=false` while retaining the raw
overshoot; only the evaluation cursor is clamped. Later calls enter the idle
parameterized-node refresh branch. Thus the last crossing step produces a
stable value such as 62.340 or 61.980.

EmoteEngine::preProgress independently advances active metadata timeline
states on every nonzero delta (`EmoteEngine.cpp:3238`); controller evaluation,
parameter binding, Player::updateLayers and physics continue. The wrapper's
real `study_motion_progress_player` call occurs before its bounded diagnostic
counter. The counter merely saturates at 31 to stop logging, and never guards
or limits progression. No production animation change is required. Any future
diagnostic rename should call this field `baseMotionFrame`, and use
`study_motion_timeline_position` to inspect an actual named timeline.
