#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace study::platform::sdl {

bool HasStockFont(const std::wstring& name);

// Both names are copied. The alias has its own generator and safely invalidates
// when its source is destroyed. Existing game-configured names take precedence.
bool RegisterFontAlias(const std::wstring& alias, const std::wstring& source);

// Compare a registered alias with its source, including lifetime/error paths.
bool CheckFontAlias(const std::wstring& alias, const std::wstring& source,
                    const std::vector<uint32_t>& visibleCharacters);

} // namespace study::platform::sdl
