# SDK source comparison — 2026-09-27

> 目录整理更新：文中历史路径 `EntisGLS/EntisGLS4.07.03/` 现已上移为 `EntisGLS/`；源码内容保持不变。`Primrose2` 与 `loquaty_lib_1.02` 发行目录已移除，构建仍使用 `vendor/official-loquaty` 源码。

Compared the former input `EntisGLS4.07.03/Cotopha` with the replacement supplied by the user, `EntisGLS/EntisGLS4.07.03/Cotopha`. “Official” below identifies the replacement selected by the user; this local comparison does not independently authenticate its upstream provenance.

## Byte comparison

| Input group | Former files | Replacement files | Identical | Different |
| --- | ---: | ---: | ---: | ---: |
| `Source/android/java` | 29 | 29 | 29 | 0 |
| `Source/android/gls4jclass` | 7 | 7 | 7 | 0 |
| `Include` | 534 | 534 | 529 | 5 |
| `Source` | 390 | 390 | 386 | 4 |

The Java builder excludes the unfinished `CameraCapture.java`, leaving the same 28 Java inputs on both sides. All seven JNI translation units are unchanged.

The nine changed C++ source/header files are:

- `Include/android/sakuragl/sgl_generic_window.h`
- `Include/common/glscs/glscs_sakura2_obj_window.h`
- `Include/common/sakuragl/sgl_window.h`
- `Include/cotopha/sakuragl/sgl_window_implement.h`
- `Include/opengl/sakuragl/sgl_opengl_window_implement.h`
- `Source/android/sakuragl/sgl_generic_window.cpp`
- `Source/common/glscs/glscs_sakura2_obj_window.cpp`
- `Source/common/glscs/glscs_sakura2_std_vm.cpp`
- `Source/cotopha/sakuragl/sgl_window_implement.cpp`

Eight implement the former SDK's nonofficial `FreezePaint` / `UnfreezePaint` extension: the Android window had an extra counter, `AddFreezePaint` / `ReleaseFreezePaint` methods, and an early return in `OnDraw`. The corresponding script wrappers are also absent from the replacement. The ninth, `glscs_sakura2_std_vm.cpp`, differs in exception/call-trace diagnostic strings; the former copy contains `test` text and extra trace output. The port does not need those diagnostic changes.

The two other changed XML files, `Scripts/Tools/make_fragment_files/dst/assets/cotopha.xml` and `Scripts/Tools/make_fragment_files/file_list.xml`, contain game-specific archive entries in the former SDK. They are not current APK inputs; the project owns `apps/android/legacy/assets/cotopha.xml`.

The replacement has no `Cotopha/Library/android` prebuilt archives. The old `libgls4.a` must not be combined with official headers: removing the added window member changes C++ object layout, in addition to provenance concerns. The current dependency migration must rebuild the Android SDK libraries from replacement source.

## ObjectHeap compatibility

`Source/common/glscs/glscs_sakura2_obj_heap.cpp` is byte-identical. SHA-256 of both inputs:

`33737052ea5e55cfb1855dbe63cf0d825958990b9e2cf21d12297e09387deb0d`

The project's checked serialization generator can keep its existing transformations after changing the input root. A generated heap translation unit compiled successfully against the official headers using Android NDK r27c, ARM64/API 29. This is a compile check, not device execution. Log: `build/official-sdk-audit/object_heap.compile.log`.

## Project-local paint freeze adaptation

`native/runtime/cotopha_port/legacy_window.cpp` previously directly called the nonofficial methods. It now keeps a nesting count per frozen native window in a project-owned registry. `tools/sdk/prepare_android_jni.py` reads the official `VirtualWindow_java.cpp`, requires exactly one expected `pGenWnd->OnDraw()` call, and emits a build-directory copy that calls `StudySteadyDrawAndroidWindow` instead. Its declaration lives in `native/runtime/cotopha_port/legacy_window_draw.h`. The other JNI source files remain direct official inputs.

The wrapper checks the freeze registry before entering official `OnDraw`. Frozen draws return immediately; unfrozen draws call the original SDK implementation. The production registry and dispatch gate live in the standalone header `native/runtime/cotopha_port/legacy_paint_gate.h` and have their own short-held mutex. No registry operation acquires the SDK UI mutex, and the registry mutex is released before calling `OnDraw`. This preserves the former extension's key timing: a frozen render callback can finish even when the script owns the UI mutex and waits for synchronous rendering work. Official `OnDraw` is nonvirtual, so merely adding a method to `WindowBridge` would not intercept the JNI call.

Nested freezes require matching unfreezes, unmatched unfreeze reports an error as before, and closing the display erases all nesting state before closing the SDK window. Window flags are unchanged. The renderer's check-then-draw race window is the same as the previous SDK's pre-lock counter check. SDK source files and base-class layouts remain untouched.

Five checks were added to the existing `CheckLegacyWindowCommandQueue` runtime probe. They cover nesting, a real cross-thread call to the JNI draw wrapper while the script owns the SDK UI mutex, option changes during freezing, final and unmatched unfreeze, and clearing nested freezes on close. They run with the existing runtime self-test; they have not been executed on a device as part of this source comparison.

Both the final updated `native/runtime/cotopha_port/legacy_window.cpp` and generated JNI translation unit independently compiled successfully against official headers with Android NDK r27c, ARM64/API 29. Logs: `build/official-sdk-audit/legacy_window.compile.log` and `build/official-sdk-audit/virtual_window_jni.compile.log`. Whole-APK link and device behavior are separate validation steps.

The host test `tests/unit/runtime/legacy_paint_gate_test.cpp` compiled with macOS Clang and ran successfully. It exercises the production gate implementation with a separate thread holding a simulated UI mutex, verifies that frozen drawing immediately returns, confirms that unfrozen drawing follows normal UI synchronization, and reenters Freeze/Unfreeze from the draw callback to verify the registry mutex is already released. Nesting, unmatched unfreeze, independent windows, null input, and reset are also covered. All eight checks passed; output is in `build/official-sdk-audit/legacy_paint_gate_test.log`. This is a host concurrency test, not an Android runtime or game smoke test.

The JNI generator was also run on the real official source and verified to reject source fixtures with zero or two matching draw calls. Those fixtures were temporary; SDK inputs were not modified.
