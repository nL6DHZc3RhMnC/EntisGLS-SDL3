# Traditional E-mote object save wire

Read-only audit of the supplied `build/analysis/ststeady.exe`, image base
`0x400000`, SHA-256
`9750f67ae51ba158cda4f756ccbd0875fb837ddd15e84db713be53f8573d91b3`.
Retained pseudocode is in `emote_save_wire.json`. No binary was patched.

## EmoteSprite payload

The primary vtable is `0x8CCD84`. Its Save/Load entries at +176/+180 point to
`0x82EF50` / `0x82DCE0`. After successful ECSSprite base Save/Load, the derived
record is exactly:

| Order | Disk data | Win32 member offset |
|---|---|---|
| 1 | Device ECSReference via its virtual Save/Load | `+0x8A0` / 2208 |
| 2 | Viewport width, height: two i32, 8 bytes | `+0x88C` / 2188 |
| 3 | Uniform scale: f32, 4 bytes | `+0x894` / 2196 |
| 4 | Coordinate X, Y: two f32, 8 bytes | `+0x898` / 2200 |
| 5 | Filename: u32 UTF-16 code-unit count, then count*2 bytes | EWideString at `+0x914` / 2324 |
| 6 | Current timeline name: same UTF-16 format | EWideString at `+0x924` / 2340 |
| 7 | u32 queued-name count; each name uses the same UTF-16 format | array at `+0x934` / 2356 |

All fixed fields are Win32 little-endian widths. Do not serialize Android
`wchar_t`, `long`, pointer sizes or current class-layout bytes. There is no
version marker, E-mote engine-state blob, voice reference, amplitude curve,
voice-variable name, voice gain or animation cursor in this original derived
record. The fixed scalar portion after the reference is 20 bytes.

Load first calls ECSSprite::Load (`0x44FF30`), loads the device reference, then
reads those scalar/string/queue fields. Player reconstruction is deferred to
CommitAllReference (`0x82D2E0`):

1. Call ECSSprite::CommitAllReference (`0x44F140`).
2. Commit the device reference and obtain its EmoteDevice entity.
3. Save the loaded scale/coordinate locally and copy the filename.
4. Call LoadPlayer (`0x82DED0`), which starts with private cleanup (`0x82ED40`).
5. Restore scale and coordinates with duration zero.
6. If a Player exists and the saved timeline name is nonempty, call its native
   slot +228 directly with `(name, flags=1)`.

This starts the named timeline from its beginning; it does not restore its
previous cursor. The queued names stay queued. Calling the outer PlayTimeline
wrapper here is incorrect because that wrapper (`0x82E840`) clears the queue.
The original private cleanup clears the device reference and filename but
leaves the current timeline string and queued names intact. The port now uses
its separate ReleaseMotionPlayer/OpenPlayer path during restoration, preserving
those fields and the Sprite base. Original Commit ignores LoadPlayer's failure result; the
port may report actual reconstruction failure instead of publishing a false
successful actor, without changing the record layout.

### Setter values and private cleanup

SetScale (`0x82F100`) sends the requested f32 scale/duration/ease-zero to native
slot +92, then immediately stores that same requested scale at `+0x894`.
SetCoord (`0x82F080`) likewise sends f32 X/Y/duration/ease-zero to +84, then
immediately stores the requested pair at `+0x898/+0x89C`. These stores happen
even if the Player pointer is null. Consequently the saved scalars are the
latest requested targets, not sampled values from the middle of a transition.
No tween duration or progress accompanies them in this wire.

Successful LoadPlayer calls Player +24 initialization, reads Player +96 scale
into the wrapper f32 and calls +88 to populate its coordinates. It does not
reapply the previous wrapper values. Ordinary loading therefore takes the new
Player's initial values; Commit explicitly saves and restores the loaded target
values around this operation. LoadPlayer begins with private cleanup, returns 1
for missing device/environment/file or failed native Player creation, and 0 for
successful creation/initialization/attachment. The script wrapper returns that
integer status to the script rather than converting every nonzero status into
an interpreter exception.

The private cleanup at `0x82ED40` releases the native Player, D3D texture and
surface, zeros the associated render dimensions, detaches its notify link,
clears the device reference, and clears the filename (`lea ecx,[this+0x914]`
at `0x82EF0C`). It preserves the Sprite base, requested viewport/scale/coordinate,
current timeline name, queue and voice state. The calls to `0x44D290/0x44D2B0`
are graphics-lock/unlock helpers; they are not the Sprite base release.

Do not conflate this private helper with the original **script ReleasePlayer**:
its name is the second entry at `0xA0C0A4`, its registered wrapper is `0x82CF50`,
and that wrapper calls object vtable +232. In this original EmoteSprite vtable,
that slot is the inherited `ECSSprite::Release` at `0x44D1A0`. The method map is
initialized by `0x899150`. The LoadPlayer/Commit path directly uses the private
helper instead and does not erase the restored Sprite base state.

## Reference traversal and hidden member indices

EmoteSprite's device reference is at `+0x8A0`; its voice reference is at `+0x948`
(the AttachVoice call at `0x82C6A6` explicitly loads `this+0x948`).
IndexAllMember (`0x82D780`) calls the Sprite base and then only device-reference
IndexAllMember. Cleanup (`0x82D2A0`) and Commit similarly process only that
derived reference. The original voice reference is not traversed or serialized
by these methods.

The original EmoteSprite does not add hidden GetVariableAt indices. Its vtable
+104 points to ECSSprite::GetVariableAt (`0x44ED00`), which handles only -1
through -10. The port's -11/-12 device/voice indices and extra voice traversal
are port-owned lifetime support, not fields or indices demonstrated in the
original Windows EmoteSprite implementation.

## EmoteDevice is not a serializable object body

The device vtable is `0x8CCCAC`; Save/Load are `0x82EF20` / `0x82DCB0`.
They first call ECSObject::Save (`0x431200`) / Load (`0x431210`). Those functions
unconditionally return non-null error messages:

- `0x91F054`: object serialization has not been defined.
- `0x91F088`: object reconstruction has not been defined.

The later device window-reference Save/Load branches are therefore unreachable
on this normal original path. No successful EmoteDevice payload containing a
window reference should be claimed from their apparent decompiled tails.

The window reference itself lives at `+44`. Device Index/Cleanup/Commit
(`0x82D760`, `0x82D290`, `0x82D2D0`) only forward to it. Device GetVariableAt is
the inherited constant-null function (`0x408740`); no hidden window member index
is exported. Device Commit does not create the rendering device. The original
Initialize (`0x82DB20`) is called by the script-facing initializer, not by Load.
This agrees with the game's `screen` and `emDevice` being static script data:
contexts reference existing initialized objects rather than serializing these
native object bodies.

## Boundary with the new native runtime snapshot API

The separately tested `study_motion_save_state/restore_state` KBAD envelope is
an optional Android runtime snapshot format, with finer Engine/controller state
than the original Sprite record. It must not be silently appended to that
unversioned record or described as the PC game's save format. The current root
integration explicitly chooses the original record and reconstruction behavior;
the KBAD API remains available for future explicitly versioned snapshots.

## Actual Android object-graph validation

On the Xiaomi 10 Pro / Android 13, the real `--emote-probe` passed at
`2026-09-23 05:31:53.223` (device log time). Evidence is
`artifacts/emote-sprite-save-test.txt`; implementation is
`native/legacy_emote_probe.cpp`. The complete global graph was **740 bytes**.
The probe does not use the optional KBAD runtime snapshot API.

- A real static Window and initialized EmoteDevice loaded `haz_a.psb` directly
  from the mounted original NOA archive. The initial 1024x768 rendered frame
  contained **56,181 nonzero-alpha pixels**.
- Before saving, the requested target scale/coordinates were `0.25, 17, -80`
  with an unfinished 60-frame transition. The wire contained those targets,
  active timeline `腕切替A`, and queued `腕切替B`, `腕切替C` in that order.
- Saving the actual ECS global graph, destroying the original actor, then
  factory-loading and committing a new actor succeeded. Its device reference
  pointed to the same initialized static instance; the Window, Sprite parent,
  position, priority and ID were preserved.
- The restored actual GLES frame contained **99,797 nonzero-alpha pixels**.
  The different count is expected: the original frame used scale 0.18, while
  original Windows load semantics immediately apply the saved target 0.25.
  The restored graph saved again, retaining the same target fields and queue.
- Advancing the real NativeSprite by 500 ms completed A, started B and left C
  queued. A truncated record failed Load. A structurally valid record naming
  `bad_z.psb` failed Commit with an actual missing-resource error. The good
  restored actor still rendered after both failures.
- The caller's context, global/static roots and primary-context pointer remained
  unchanged. The isolated fixture imports only the native class declaration
  needed by the real factory. An earlier fixture lacking that declaration
  failed object creation; the corrected probe also explicitly verified the
  real CSX production factory before running the isolated graph test.

This proves the implemented Android derived record and actual object lifecycle;
it does not by itself establish interchangeability of complete PC save files.

## ReleasePlayer use in the supplied game CSX

Read-only scan of `build/game/script.csx`, SHA-256
`897cb337bc2fc1d99d2c685a9f30b10a76439f03f5aa79a94a738333e773567a`, found
**no ReleasePlayer call sites**. Its sole method declaration is EmoteSprite
class 87, method 129. The scan decoded 73,771 distinct object instructions from
the function records and found no named or typed call. A second scan across the
entire 553,952-byte instruction image found no opcode 19/20 instruction encoding
for that class/method pair. None of the 2,858 constant strings contains
`ReleasePlayer` either.

The object decoder explicitly reports 14 unsupported naked-code or embedded-data
regions; these were not silently treated as decoded instructions. The whole-image
typed-call scan also covers those bytes. This does not exclude names assembled
at runtime or code from external files; no such use was observed in this CSX.

Actual EmoteSprite calls occur in `ScreenData::ChangeBustshot`,
`ScreenData::MoveEmoteBustshot`, timeline/voice methods and `UITitle::LoadChara` /
`UITitle::OnUpdateTimeline`; none calls ReleasePlayer. The inspected bustshot
clear/flush routines use movement/activation and `ScreenData::CleanUp` rather
than ReleasePlayer. Consequently there is currently no demonstrated game-script
dependency on the original method's preservation of Player/voice/queue state.
The original base-only script behavior and the port's broader `Release()` remain
a known compatibility difference; changing it is not required by any observed
call in this game, and must not be confused with the now-tested private cleanup
used by LoadPlayer/Commit.

Reproduce the scan with:

```sh
python3 tools/motion_audit_csx_release.py \
  --output native/motion_bridge/evidence/emote_csx_release_calls.json
```

The retained JSON contains all relevant Emote call sites and the explicit
undecoded-region list. No production source or original binary was changed.
