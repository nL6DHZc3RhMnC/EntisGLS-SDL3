#pragma once

#include <filesystem>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace entis::launcher {

struct FontSpec {
    std::string path;    // UTF-8; SDK virtual lookup path or storage://game URI.
    std::string family;  // Configured name/alias; empty means the font's own family.
};

struct FontAlias {
    std::string alias;
    std::string source;
};

struct GameLaunchConfig {
    std::string normalizedXml;
    std::string entryScript;
    std::string title;
    std::string gameId;
    // Migration only: 0.3.0 included XML psb_key in generated identity. The
    // current identity never depends on a decoding setting. Empty if unchanged.
    std::string previousGameId;
    bool explicitGameId = false;
    std::string compatibilityProfile; // Empty is the generic runtime.
    std::string configSource;
    std::optional<std::uint32_t> psbKey;
    std::vector<FontSpec> openTypeFonts;
    // Dependency order: target aliases precede their users. Filters are absent
    // from normalizedXml and must be registered after openTypeFonts.
    std::vector<FontAlias> fontAliases;
    std::vector<std::string> warnings;
};

class ConfigError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Reads configuration, never executes the game's Windows executable. Selection:
// explicitConfig, entis-launcher.xml, cotopha.xml, then one EXE IDR_COTOMI
// (including intact PEs in its overlay), then verified script.noa/script.csx.
// explicitConfig may name XML or an EXE inside gameDir. This discovers legacy
// Cotopha .csx applications; other VM formats are reported as unsupported.
// Safe before SDK/SDL initialization; uses the SDK's standalone ERISAN decoder
// and a bounded XML subset, without starting the VM or graphics.
GameLaunchConfig DiscoverGame(const std::filesystem::path& gameDir,
                              const std::filesystem::path& explicitConfig = {});

// Also used for a positively identified compatibility profile's configuration.
// No automatic profile selection occurs in this function.
GameLaunchConfig NormalizeGameConfig(const std::filesystem::path& gameDir,
                                     const std::string& xml,
                                     const std::string& source);

} // namespace entis::launcher
