# First-scene E-mote interface audit

Read-only game/script/resource inspection plus a bounded opaque-runtime clone
fix, 2026-09-23. The subsequent device run reached both first-scene actors;
this is not a claim that all later scene effects have passed.

## Actual inputs and bridge mapping

The first E-mote actor entry in `build/game/common1_1_R.srcxml` is `@l=775`,
`chgpsb src="haz_h"`. Its opening timeline sequence uses `03`, `08`, `30`, `01`,
`14`, queued `00`, and `50`; its first voice is `haz00000`. The next actor at
`@l=816` is `nak_d`, with `02`, `01`, `29`, queued `02`, queued `53`, and first
voice `nak00000`.

Both actual PSBs were read from `StudySteadyR18/psb.noa` and parsed with the
existing verified PSB reader. Every listed timeline label exists. Both PSBs have
an enabled `face_eye_open` eye controller with automatic blinking, and their
mouth controller uses the `face_talk` talk variable.

The actual CSX native calls are covered by the existing bridge:

| Script/native use | Existing implementation |
| --- | --- |
| CreateSprite / SetBackColor / SetVisible | inherited ECSSprite |
| SetScreenSize / LoadPlayer | ECSEmoteSprite viewport and real NOA/TJS loader |
| SetScale / SetCoord | real root controllers, D3D duration/ease semantics |
| PlayTimeline / PostTimeline | actual engine timeline plus ordered queue |
| IsPlayingTimeline | actual engine active-state query |
| AttachVoiceSync | actual Resource playback cursor plus analyzed PCM curve |

There is no missing separate Eye/Mouth native method in these calls: eyes are
metadata controllers, and `ScreenData::AttachVoiceSync` at 0x2a77f passes the
literal `face_talk` with `ww.m_fpMouthVelocity * 5` to EmoteSprite.

`haz00000.mio` and `nak00000.mio` were read from the actual `voice.noa`.
Their original SoundInf records both declare 44,100 Hz, one channel, and 16-bit
samples, matching the current bridge's PCM analyzer. Sample counts are 141,738
and 312,472 respectively. This header check is not a device lip-sync test.

## Clone first-frame publication

`study_motion_clone_player` created a new actor and restored engine controller
state, but left its opaque base/user-scale fields at constructor values and did
not publish the restored state into the new Player before drawing. It now copies
both scale fields and performs the same zero-time dirty-state publication as the
already integrated `restore_state` path. It neither advances the timeline nor
shares a mutable output texture with the source.

The independent runtime probe now checks the actual root controller and wrapper
scale fields through the real TJS structured-binary reader. It verifies initial
clone fields equal the source, subsequent user scale 3 produces current root
scale `baseScale * 3`, and changing the clone does not change the source's
geometry. This replaces an invalid model-wide linear-AABB assumption: updating
scale also re-evaluates geometry/controllers, and the native engine snapshot
contains controller state, not every hair/parts physics-chain point cache.
Full dynamic pose pixel equality between independent actors is not asserted.

## Validation

Current host builds of the actual TJS/NCB/Player/runtime passed the complete
three-owner-cycle CPU probe for `haz_a`, `haz_h`, and `nak_d`, including timeline
state, cloning, subsequent scale, structured state roundtrip, cross-owner restore,
and negative malformed-state cases. Logs are:

- `build/analysis/emote-first-scene/haz_a-runtime-final.txt`
- `build/analysis/emote-first-scene/haz_h-runtime-final.txt`
- `build/analysis/emote-first-scene/nak_d-runtime-final.txt`

The updated runtime and independent probe also compiled as isolated Android
ARM64 API 29 objects. Root integrated the frozen runtime change. Actual game
play-through then loaded `haz_h.psb` and `nak_d.psb` from NOA, displayed both
actors together and advanced multiple dialogue sections without E-mote drawing
or progression errors. Evidence: `artifacts/game-actor-approach-05.txt`
(08:11:27 and 08:11:35 loads), `artifacts/game-first-two-actors.png`.
The original game wrote save slot 16 with both actors (70,924-byte BMP container,
93,697-byte decoded context,
`artifacts/game-actors-save-16.txt`). Cold actor restore and detailed visual
lip-sync verification are separate checks; no successful stub replaced a
missing API during this audit.
