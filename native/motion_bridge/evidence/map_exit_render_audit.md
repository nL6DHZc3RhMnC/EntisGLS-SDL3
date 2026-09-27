# Map selection audit: no port defect reproduced (2026-09-23)

Controlled device checks passed in both normal mode and full-skip mode. The earlier map-failure interpretation was incorrect: while skip is enabled, the first click cancels skip; a second click confirms the location. The subsequent pet-shop camera sequence takes about 18 seconds in normal mode. No input, detach, snapshot, or render behavior was changed for this investigation.

## Device result and repeatable operation

`artifacts/game-map-skip-correct-confirm.txt` and `.png` record the strict check:

1. Cold-load the pre-choice automatic save (copied to slot 10).
2. Enable full skip, return to the map, and tap the pet-shop photo once. This cancels skip.
3. Leave skip disabled and tap the same photo again. Do not press F8 between the two taps.
4. The real map page detaches at 09:24:01. Pet-shop dialogue appears at 09:24:19.

Normal-mode check also passed: `game-map-detach-trace.txt` shows detach at 09:15:34.743, followed by pet-shop dialogue at 09:15:52.633 (`game-map-detach-after-idle.*`). A five-second screenshot showing an intermediate zoom was not a stalled frame.

The temporary map-detach logs and bounded Window scheduling counters were removed after these checks. Original production callback order and behavior are retained, including the existing Dynamic mode, input, screenshot, and animation fixes. The generic primary-context load marker was not changed.

## Why the first skip-mode click does not select

The original CSX explicitly implements this behavior:

- `WitchWizard::SetSkip` at `0x69cd5` calls `SetLeftClickFunction(1)` when enabling skip.
- `SetLeftClickFunction` maps the left mouse event to `ID_MESSAGE_CLICK` at `0x6a0cd..0x6a114`. Function 0 instead maps it to joystick device 0, key 4 at `0x6a083..0x6a0c5`.
- `UICharacterSelect::InputLoop` dispatches a queued Window command (`0x3f7a9`) and then flushes pushed joystick counts (`0x3f7ca`), before checking confirmation key 4 at `0x3f864`.
- `ID_MESSAGE_CLICK` handling at `0x6ad5e..0x6ad81` turns skip off; it does not choose a map location.

The unsuccessful controlled sequence in `game-map-skip-detach-trace.txt` enabled skip at 09:19:45, clicked once at 09:19:46, then pressed F8 again at 09:19:48. That restored skip rather than confirming the map. Its lack of a detach was expected.

Evidence: `build/analysis/map-set-skip.txt`, `map-set-left-click.txt`, `map-input-loop.txt`, and `WitchWizard-DispatchCommand-save-audit.txt`. The port's InputFilter correctly dispatches a command mapping without also counting it as joystick confirmation.

## Map cleanup and rendering

`UICharacterSelect::Initialize` builds `ID_CHARACTER_SELECT` into `this[1]` at `0x3e682`; Fadein attaches it to screen at priority 65536 (`0x3ee9e`). Its selection test uses the actual photo sprite rectangles and input cursor position. No map-specific native hit-test callback is missing.

Fadeout starts a fade toward transparency 256, waits at most 50 ms with `WaitUntilSpriteActive(page,50,1)`, then detaches the page at `0x3f5ad`. It does not Release the page first. Consequently transparency 33 at detach is compatible with the original 500 ms fade and short wait, not evidence of a broken action.

The real detach diagnostic returned the correct child, cleared its parent, reported a valid 1920×1080 rectangle, and marked both remaining ancestors dirty. They had no framebuffer. The next scene subsequently rendered normally.

Evidence: `build/analysis/map-{initialize,fadein,fadeout,test-cursor,run}.txt` and `artifacts/game-map-detach-trace.txt`.

## Save 18 and snapshot interpretation

`artifacts/current-save0018.bmp` is 71,274 bytes, SHA-256 `8d9ce1efc31f372f1039629e9ca16031d4e6627e5318f0872535177d78b9cd9d`. Its decoded context is 92,226 bytes, SHA-256 `1ee003a1069e0dfc16efce09e34c2897cd1124a6d1923cb3d732c05b2b848846` (`build/analysis/map-save18-context.bin`). Use `python3 tools/motion_locate_save.py artifacts/current-save0018.bmp` to repeat the script-name scan without modifying the save.

That saved context was already in `common1_5_choice_nak`. The stack contains `WitchWizard::OutMsg`, `MessageWindow::ShowMessage`, `WitchWizard::DispatchCommand`, and `UISave::DoModal`, with no UICharacterSelect frame. The BMP thumbnail itself shows the pet shop, and a cold load displays that scene. F2's ID_SAVE handler (`0x6a689..0x6a715`) only opens UISave and takes screenshots; it cannot select a map location. An earlier extra tap or Enter could have confirmed the map without a Window-command log. The old log does not identify that physical input, so save 18 did not establish live map residue.

The snapshot-copy hypothesis was also unsupported by actual script calls: the only DrawImage calls from screen are TakeScreenShot (`0x170e6`, preceded by Refresh `0x170b9`) and DoSaveScreenshot (`0x6c9a7`, preceded by Refresh `0x6c97a`). The third DrawImage uses a logo. No CopySprite or AttachImage call copies screen into the live transition. ScreenManager::ChangeScreen attaches and animates ScreenData SuperSprites. No unconditional GetImage refresh was added.

Evidence: `build/analysis/map-image-source-callers.txt`, `map-take-screenshot.txt`, `super-changescreen-audit.txt`, and `artifacts/game-map-cold18-before-fix.*`.
