#pragma once
#include <sakura/sakura.h>

namespace study::platform::sdl {
// Native paths retain the SDK implementation. Registered document paths use
// the same scoped backend as configuration discovery and E-mote DLL reading.
SSystem::SFileOpener* NewGameFileOpener();
}
