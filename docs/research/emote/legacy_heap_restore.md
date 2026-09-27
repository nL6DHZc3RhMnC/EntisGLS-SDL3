# Traditional heap restoration: real Buffer identity and checked SDK loader

The first actual F3 load crashed after the game had saved a 90,403-byte decoded
context. Evidence: `artifacts/game-load-first-crash.txt`, PID 24328,
2026-09-23 06:26:52 device time. This was not an empty-heap operation and was
not a null VirtualMachine argument.

## Exact crash mechanism

The imported ARM64 library's ObjectHeap::LoadHeapStatic starts at `0x100bfe0`.
The matching local ELF disassembly shows:

```text
0x100c070  ldr x8, [x22]       // valid VM virtual table
0x100c080  blr x8             // NewObjectByIdentity
0x100c08c  str x0, [slot]     // null factory result
0x100c0a0  ldr x8, [x0]       // actual tombstone PC, x0 = 0
```

The original modern SDK source
`Cotopha/Source/common/glscs/glscs_sakura2_obj_heap.cpp` assigns
`pObj->m_dwHighAddr` before checking `pObj != NULL`. The optimizer may therefore
remove its later null check. It also ignores the result of reading each class
ID and continues after a failed heap header read.

`ECSExecutionImage` is a real `ECSSakura2::StandardVM` subclass. Its generated
InitializeSakuraProcessor always allocates a traditional **ECSBuffer** naked
stack and puts it into `m_heapGlobal`, even when ordinary object bytecode is
executing. It also sets `m_pSakura2VM = m_pcsxi`. Additional buffers and native
stack frames can use the same heap, and saved references/registers address its
slots. Skipping heap restoration would lose real data and stack addresses.

The traditional Buffer's saved type name is `Buffer`. The current SDK's modern
factory table registers `SSystem_Buffer`, not that name. The old
ECSExecutionImage::NewObjectByIdentity first tried the modern factory, then
only `GetClassInfoAt(id)`. Runtime-added Buffer IDs have a class-vector name but
no CSX ECSClassInfo entry, so both paths returned null. For the supplied CSX
there are 170 declared classes; the first appended Buffer identity is therefore
expected to be 170 (0xaa). The crash register dump also contains 0xaa, but the
type conclusion primarily follows the actual factory paths; the decoded heap
payload has not independently been dumped by this audit.

## Implemented compatibility changes

`tools/sdk/patch_legacy_heap_state.py` changes only generated GLS3 copies:

- NewObjectByIdentity looks up the saved vector's **name** and first calls the
  real ECSContext::CreateObject. Buffer consequently becomes an ECSBuffer,
  whose existing SaveStatic/LoadStatic delegate to traditional Buffer Save/Load.
  Modern object creation remains the fallback for actual modern type names.
- Buffer uses the original 8-byte LE header (base, length) and exact payload.
  Reads and writes are checked; negative/oversized lengths and address overflow
  are rejected. A complete payload is read before replacing the old buffer.
- Context.Load propagates class-vector load failure and commits its return
  object before the final processor commit. A saved nonzero naked-stack address
  that does not resolve to ECSBuffer now returns an error in release builds.

`tools/sdk/motion_prepare_heap_sdk.py` generates a complete translation-unit copy
of the original modern ObjectHeap source. Its source SHA-256 is embedded in
the generated file. The original SDK and archive are unchanged. The same heap
layout and algorithms remain in use; the copy adds checked header/slot reads,
allocation bounds, early propagation of object-load/write failures, and a null
factory check **before** accessing the object. Invalid class IDs do not become
successful empty objects. Valid empty heaps still consume and validate their
actual header; nonempty heaps recreate every actual object and preserve indices,
count, allocation cursor and virtual selector.

The root build links this complete replacement object before imported
`libgls4.a`. An isolated symbol comparison found all **34** original externally
defined heap symbols in the replacement (52 definitions total). An actual
`ld.lld -r replacement.o libgls4.a` succeeded, and `--why-extract` confirmed that
the old `glscs_sakura2_obj_heap.o` archive member was not extracted. Artifacts:
`build/legacy_heap_sdk/archive-link-check.o`, `.log`, and
`archive-why-extract.tsv`.

## Validation and root hooks

Five isolated ARM64 translation units compiled successfully: generated heap,
execution image, Buffer, context, and `tests/probes/cotopha/legacy_heap_probe.cpp`. Command:
`python3 tools/diagnostics/motion_compile_heap_probe.py` (before the root prepare hook has
already patched its generated input). Diagnostic logs reside in
`build/legacy_heap_sdk/`. No full APK build or device run was performed by this
agent for this change.

The root integration hooks are:

1. Apply `patch_legacy_heap_state.patch_source` at the end of the existing
   generated GLS3 patch pipeline.
2. At configure time generate `legacy_heap_sdk/glscs_sakura2_obj_heap.cpp` with
   `motion_prepare_heap_sdk.py`; compile it directly into entisgls4 so its full
   definitions precede the imported archive.
3. Compile `tests/probes/cotopha/legacy_heap_probe.cpp` and invoke
   `CheckLegacyHeapState(environment)` from the independent self-test.

The probe uses actual InitializeSakuraProcessor, fills the real naked stack,
allocates another ECSBuffer with nonzero base, saves the real class vector and
heap, destroys and reloads them, reconnects the processor stack, and checks
both virtual addresses, all bytes and an exact second save. It also requires
errors for unknown identity, truncated payload and excessive heap allocation,
then validates a real empty-heap record.

The complete probe passed on the Xiaomi 10 Pro / Android 13 on 2026-09-23;
the unified self-test completed at device time **06:42:40.928**. Evidence:
`artifacts/heap-thread-full-self-test.txt`. The real stack plus Buffer heap was
**4,223 bytes**, with stable virtual addresses, byte-exact second save and all
negative cases passing. A subsequent actual game load passed heap restoration
without a native crash, but later failed in the saved UISave script receiver;
complete game-load continuation is therefore a separate outstanding test.
