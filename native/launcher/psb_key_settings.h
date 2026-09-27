#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

namespace entis::launcher {

class PsbKeySettingsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Whitespace-only means automatic discovery; zero is a valid explicit value.
// Accept decimal or 0x/0X hexadecimal only, with no sign or trailing text.
std::optional<std::uint32_t> ParsePsbKey(const std::string& text);

// gameDataDirectory is the launcher's app-data/games/<gameId>, never the game
// resource directory. A missing psb-key.txt means no per-game user override.
std::optional<std::uint32_t> ReadPsbKeyOverride(const std::filesystem::path& gameDataDirectory);
void WritePsbKeyOverride(const std::filesystem::path& gameDataDirectory,
                         std::optional<std::uint32_t> key);

} // namespace entis::launcher
