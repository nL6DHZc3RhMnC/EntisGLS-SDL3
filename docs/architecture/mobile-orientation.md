# Mobile game orientation

Android and iOS choose the display axis from the game's logical window dimensions:
wide games allow both landscape orientations, tall games allow portrait orientations,
and square games allow both axes. The decision uses `CreateDisplay` / window creation
and follows `ChangeDisplaySize`; it never infers the game aspect from the phone's
current physical screen dimensions. The SDK's existing viewport/layout calculation
continues to preserve the game aspect ratio.

The SDL window remains resizable so rotation and drawable size events are delivered.
The launcher sets `SDL_HINT_ORIENTATIONS` before creating it. On Android the app maps
the two-sided hints to `SENSOR_LANDSCAPE` or `SENSOR_PORTRAIT`, because SDL's default
`USER_*` mapping follows the system rotation lock. The selected game axis therefore
still applies while the phone has portrait rotation locked, and both landscape sides
remain available. The manifest no longer assumes every game is landscape.

On iOS, changing a hint does not itself request a UIKit rotation. The platform bridge
updates the visible controller's supported orientations and requests scene geometry
on iOS 16 and later. It also calls UIKit's rotation reevaluation for older iOS versions
and windows without a scene. iPhone UIKit excludes upside-down portrait; iPad can use
both portrait sides. The app's Info.plist permits both landscape directions and portrait.

Logical game-window close restores the previous SDL orientation hint immediately,
even if a shared GL context keeps the physical SDL window alive. Failed window setup
also restores the hint. The native iOS library refreshes its own orientation after it
becomes visible; Android returns to a separate launcher Activity with its own policy.

`mobile_orientation_test` checks dimension selection, both-side hints, and restoration
on close/setup cleanup without game data or a display. The optional iOS Simulator
presentation diagnostic additionally starts a resizable window in portrait, requests
landscape through the production helper, and waits for a landscape drawable before
publishing readiness. These checks do not replace Android/iOS physical-device rotation
testing. The normal release workflow builds iOS; the Simulator workflow is manual.
