#pragma once
#include <string>

namespace study::platform::sdl {
// The Activity supplies a scoped document-tree grant; no filesystem path is
// inferred from a provider URI. Call on the SDL thread before game discovery.
void ConfigureAndroidGameFiles(const std::string& root);
}
