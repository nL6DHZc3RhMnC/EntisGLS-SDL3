#pragma once

#include "launcher/game_config.h"

namespace entis::launcher {

// Last-resort discovery for games without a readable launch manifest. Uses
// bounded NOA metadata reads and checks the compiled entry header before making
// a minimal in-memory configuration. Never extracts into the game directory.
// Missing script.noa or script.csx returns nullopt; malformed archives throw.
std::optional<GameLaunchConfig> DiscoverArchiveFallback(const std::filesystem::path& root);

} // namespace entis::launcher
