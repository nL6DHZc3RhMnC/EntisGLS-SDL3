# Primary Cotopha context restoration

This file records the Android port's restore behavior, not a claim that arbitrary
Windows saves are compatible with the port.

## Actual game failure isolated on device

`artifacts/game-primary-trace-test.txt` (PID 28386, 2026-09-23 07:00:51) shows
`before-global-commit`, but no `before-stack-commit`, `before-arg-commit`, or
`load-complete`. The saved local `UISave` object remains at absolute stack slot
59. `UISave::DoModal`'s reference at slot 61 has not yet been resolved. Execution
nevertheless reaches `UISave::Release`, then fails on `this[1]` at saved IP
0x580c9. This is a consequence of an incomplete restore, not evidence that a
successful stack commit subsequently lost `this`.

The follow-up `artifacts/game-primary-error-test.txt` at 07:04:37 preserved the
first error: `Saved Sprite item cannot be found in the reloaded skin page`.
Sprite restoration is handled separately. The main returned object in this
save is null; it is not responsible for the failure.

The save was successfully parsed from
`artifacts/game-save0014-heap-fixed.bmp`: the inner context is 89,311 bytes,
including a 16,496-byte nonempty heap. Processor continuation is 0x17472 inside
script `SaveContext`. The original script intentionally returns from the saved
`UISave::DoModal` call into `UISave::Release` when load succeeds.

## Failure boundary

`tools/sdk/patch_legacy_context_failure.py` modifies only the generated legacy
`glscs_context.cpp`. After the EMC header validates and before `OnBeginningLoad`,
a local guard marks the operation as destructive. Any early return or exception
halts the context. The guard is dismissed only after all loads, reference
commits, processor reconnect, extended data, and `OnFinishedLoad` callbacks have
completed. There is no attempt to execute or roll back a partially replaced
stack.

`native/runtime/cotopha_port/legacy_script_file.cpp` keeps the original `context.Load` error instead
of replacing it with a generic load message. `Call_LoadContext` propagates this
error when an executing context has been halted by failed restoration. It does
not push an Integer into the half-restored stack. File/header/decompression
failures before destructive restoration retain the old script convention:
return an Integer error to the still-valid caller. Successful load retains the
saved continuation and Integer zero result.

`OnFinishedLoad` is void in the original ABI. This guard cannot detect an
internally suppressed callback error. It does prevent callbacks from being
called on any earlier returned error.

## Independent reference commit defect

The original `ECSReference::CommitAllReference` consumes its saved path on
successful resolution. A second call used the now-empty path and replaced the
resolved target with its root (for example, the whole Stack). The own-object
branch unconditionally reinserted the same reference in its owner's backlink
list, allowing a self-cycle.

`tools/sdk/patch_legacy_reference_commit.py` preserves a non-null resolved target
whose path has already been consumed, and inserts an own-object backlink only
when it is not already linked. It still recursively commits own-object children,
including newly loaded pending references. This changes neither wire format nor
native class layout. This is a real separate defect; the first actual game
failure above happened before the primary Stack commit and does not establish
this defect as its cause.

## Validation

`tests/probes/cotopha/legacy_primary_context_probe.cpp` contains two actual legacy-runtime
probes, with their own execution image/context and restored primary-context
ownership on exit:

- `CheckLegacyPrimaryReferenceState`: serialize/load a reference to a real local
  receiver, commit it twice, validate `this[1] == 42`; repeat own-object commit
  after adding a new pending child, and check both children and finite backlink
  structure.
- `CheckLegacyContextLoadFailure`: write a real `File.SaveContext` including
  processor, graph and stack heap; make the real virtual extended-data stage
  reject the load; verify exact error propagation, halted saved IP and no pushed
  result; verify an invalid outer file preserves old execution and returns an
  Integer; verify the valid load preserves successful continuation/result.

2026-09-23: ARM64 API 29 isolated compilation passed for generated Context with
both trace and failure guard, File, and probes. Both new probes passed on the Xiaomi 10 Pro / Android 13 in the complete
device suite at 07:17:35.081, recorded in
`artifacts/optional-skin-reference-self-test.txt`. Earlier heap/thread probes
also passed separately.

The subsequent actual title-to-save14 load passed global commit and halted
correctly at a missing saved Stack reference; it did not execute the unfinished
restored frame. `artifacts/game-optional-skin-load-test.txt` records this next
independent issue. Its reference mode/path diagnostics are being expanded;
complete game load is not yet claimed.

## Cold static script cache and original container policy

The detailed cold-load trace (`artifacts/game-stack-reference-trace.txt`) located
Stack slot 21, Data mode 4, path
`[34,0,0,1,4,0,-1,4,0,-1,4,9,-1]`. Data[34] is the original `s_manScript`
ScriptManager; its field 0 is `m_scripts`, a Hash of lazily loaded ScriptObjects.
The cold title screen's Hash slot 0 is absent. A warm load after entering the
first scene completes with the same save and APK
(`artifacts/game-warm-stack-reference-trace.txt`, 07:28:54).

This is a port regression in the container error policy. The original GLS3
Array and Hash `CommitAllReference` both deliberately leave `return err`
commented out inside the child loop. The original game binary independently
confirms it: Array function 0x449B50 and Hash function 0x551450 call virtual +172
for every child, discard its returned error, and return zero. Original Context
Save 0x4378D0 and Load 0x437D60 do not serialize Data roots. No new static-cache
record is needed or introduced.

The original script reconstructs the cache after native load returns:
`OnContextLoaded` invokes `WitchWizard::OnContextLoaded` at 0x66c63, which reloads
`this[26] = s_manScript.LoadScript(this[24] + ".srcxml")`. The already suspended
XML command references are intentionally disposable on this path. In save14:

- Slot 21 is RunDynamicScript's local xmlCmd; return slot 22 is 0x622db.
- Slot 24 is RunDynamicScriptCommand's xmlCmd argument; return slot 28 is
  0x6255f (the OutMsg continuation).
- After restoration and the hook, 0x6255f only leaves the block and returns via
  0x66c61. 0x622db destroys the old local xmlCmd. The next loop iteration at
  0x62240 gets fresh code through the newly loaded `this[26]`.

Thus these stale XML references are not dereferenced on the observed original
saved-message return path. Restoring the original Array/Hash traversal lets
later references, including UISave's local this, be resolved normally. Missing
static targets remain null; no fabricated target or fake resource is returned.
The pending saved path is retained by ECSReference and can resolve if explicitly
committed after its real cache is rebuilt.

`tools/sdk/patch_legacy_array_commit.py` now retains this original child policy. The
port's added default-element commit and its direct error propagation are retained.
Corrupt wire parsing and direct object/processor/extended-data load errors still
propagate; the failed-restore halt guard remains active. The augmented primary
reference probe removes a real Data Hash entry, verifies that Array/Hash continue
to later and default references, then reconstructs the actual cache entry and
verifies both pending references resolve to the same new object. ARM64 isolated compilation passed for Array, Hash, the augmented primary
reference probe, and the updated Thread probe. Cold-device game-load and
complete-suite validation of this augmentation are pending.

Device follow-up: `artifacts/game-cold-load-fixed.txt` reached `load-complete`
at 07:36:35 after a fresh title startup, and a real subsequent click advanced to
the next dialogue line (`artifacts/game-cold-load-fixed.png`). The augmented
primary/reference/cache/Thread probes also passed in
`artifacts/cold-cache-self-test.txt`. That aggregate run still had one unrelated
older SpriteState assertion expecting container errors to be fatal; the UI
agent is updating it to test the required Sprite error directly. Therefore that
aggregate run is not labelled wholly passing here. Saving a new slot after this
successful cold load is a separate test still being investigated.
