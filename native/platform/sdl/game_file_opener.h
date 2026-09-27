#pragma once
#include <sakura/sakura.h>

namespace study::platform::sdl {
// Native opens retain SDK flags/path handling; document paths use the scoped
// backend. Read-only seekable streams share their original handle on Duplicate.
SSystem::SFileOpener* NewGameFileOpener();
// Takes ownership, including when allocation fails. Non-seekable streams are
// returned unchanged. Call only for a stream opened without write permissions.
SSystem::SFileInterface* ShareReadOnlyFile(SSystem::SFileInterface* source);
}
