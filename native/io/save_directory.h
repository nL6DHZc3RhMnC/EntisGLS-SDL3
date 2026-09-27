#pragma once
#include <filesystem>

namespace entis::launcher {
// All platforms save inside the selected game directory. Create it if missing;
// existing files are left intact. No old application-data directories are read.
void PrepareSaveDirectory(const std::filesystem::path& gameDirectory);
}
