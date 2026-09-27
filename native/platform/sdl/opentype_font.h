#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace SSystem { class SFileInterface; }

namespace study::platform::sdl {

struct OpenTypeRegistration {
    bool loaded = false;
    std::wstring family;
    std::wstring registeredName;
    std::string error;
};
using OpenTypeFileOpener = std::function<SSystem::SFileInterface*(const wchar_t*)>;

// Reads an explicit SDK path and registers the font's actual Unicode family
// (the first face for a collection).
// A caller-specified expectedFamily validates a compatibility resource; use
// RegisterFontAlias for a user-configured name that differs from the real family.
// configuredName selects an independent face even when another configured file
// has the same family (for example regular/bold). registeredName is the stock
// name to select; family always reports the font's actual family metadata.
// The existing family stock wins when registering an additional named face.
// Conflicting reuse of an explicit configured name is rejected. No font paths,
// game names, system font installation, or default fallback are implied here.
OpenTypeRegistration RegisterOpenTypeFont(const std::wstring& path,
                                         const std::wstring& expectedFamily = {},
                                         const OpenTypeFileOpener& opener = {},
                                         const std::wstring& configuredName = {});

bool CheckOpenTypeFont(const std::wstring& family,
                       const std::vector<uint32_t>& visibleCharacters,
                       const std::vector<uint32_t>& spaceCharacters = {uint32_t(' ')});

} // namespace study::platform::sdl
