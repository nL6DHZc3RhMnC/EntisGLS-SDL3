#pragma once

#include <string>

namespace study::platform::sdl {
// Run on SDL's main thread. Imports copy a user-selected folder into Documents;
// the runtime never relies on a security-scoped URL after this screen closes.
bool ChooseIOSLibraryGame(const std::string& documents, std::string& game,
                          double smokeSeconds = 0);
}
