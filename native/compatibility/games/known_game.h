#pragma once
#include <filesystem>

namespace study::launcher {

// Complete known script archive fingerprint. Does not inspect the directory
// name, execute game code, or initialize SDL/the SDK. Missing or unreadable
// archives and every other release simply return false.
bool IsKnownStudySteady(const std::filesystem::path& gameDir);

} // namespace study::launcher
