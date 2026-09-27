#pragma once
#include "platform/game_files.h"
#include <filesystem>
#include <memory>
#include <utility>

// The virtual root deliberately does not exist on disk. All successful reads
// therefore prove discovery used the injected storage interface.
class MappedGameTestBackend final : public entis::io::Backend {
public:
    explicit MappedGameTestBackend(std::filesystem::path directory) : root(std::move(directory)) {}
    entis::io::FileInfo Stat(const std::string& path) override { ++stats; return entis::io::Stat(root / path); }
    std::vector<entis::io::Entry> List(const std::string& path) override { ++lists; return entis::io::List(root / path); }
    std::FILE* Open(const std::string& path, const char* mode) override { ++opens; return entis::io::Open(root / path, mode); }
    void CreateDirectory(const std::string& path) override { entis::io::CreateDirectory(root / path); }
    void Remove(const std::string& path, bool directory) override { entis::io::Remove(root / path, directory); }
    void Rename(const std::string& from, const std::string& to) override { entis::io::Rename(root / from, root / to); }
    std::filesystem::path root;
    unsigned stats = 0, lists = 0, opens = 0;
};

class GameTestMount final {
public:
    GameTestMount(const std::filesystem::path& virtualRoot, std::shared_ptr<entis::io::Backend> backend) {
        entis::io::SetBackend(virtualRoot.u8string(), std::move(backend));
    }
    ~GameTestMount() { entis::io::SetBackend({}, {}); }
};
