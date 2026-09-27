#pragma once

namespace study::platform::sdl {
// Asset-free diagnostic. Keep the visible GLES pattern alive until the CI
// harness terminates the app, so UIKit can commit and capture the presented frame.
int RunIOSPresentationSmoke();
}
