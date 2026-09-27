#include "save_directory.h"
#include "platform/game_files.h"
#include <stdexcept>

namespace entis::launcher {
void PrepareSaveDirectory(const std::filesystem::path& gameDirectory) {
    const auto destination = gameDirectory / "savedata";
    const auto current = io::Stat(destination);
    if (current.kind != io::FileInfo::Kind::Missing) {
        if (current.kind != io::FileInfo::Kind::Directory || current.symbolicLink)
            throw std::runtime_error("The game's savedata path is not an ordinary directory: " + destination.string());
        return;
    }
    io::CreateDirectory(destination);
}
}
