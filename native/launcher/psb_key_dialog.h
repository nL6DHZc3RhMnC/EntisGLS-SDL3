#pragma once
#include <filesystem>
#include <string>
namespace entis::launcher {
bool ShowPsbKeySettings(const std::filesystem::path& gameDataDirectory,
                        const std::string& gameTitle);
}
