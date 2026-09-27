#pragma once
#include <string>

namespace study::launcher {

// Select only non-cryptographic compatibility behavior before the game worker.
bool ConfigureCompatibility(const std::string& profileId);
std::string ActiveCompatibilityProfile();

// Profile IDs come from the launcher config's explicit or verified selection.
// An empty profile has no game-specific fonts, aliases, or resource requirements.
bool ApplyCompatibilityFonts(const std::string& profileId);
bool CheckCompatibilityFonts(const std::string& profileId);

} // namespace study::launcher
