#include "io/save_directory.h"
#include "../../fixtures/game_file_backend_fixture.h"
#include "runtime/cotopha_port/legacy_atomic_path.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace fs = std::filesystem;
static void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
static std::string Read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
class FailingRename final : public entis::io::Backend {
public:
    explicit FailingRename(fs::path path) : delegate(std::move(path)) {}
    entis::io::FileInfo Stat(const std::string& p) override { return delegate.Stat(p); }
    std::vector<entis::io::Entry> List(const std::string& p) override { return delegate.List(p); }
    FILE* Open(const std::string& p, const char* mode) override { return delegate.Open(p, mode); }
    void CreateDirectory(const std::string& p) override { delegate.CreateDirectory(p); }
    void Remove(const std::string& p, bool directory) override { delegate.Remove(p, directory); }
    void Rename(const std::string&, const std::string&) override { throw std::runtime_error("provider rejected replacement"); }
    MappedGameTestBackend delegate;
};
int main() {
    char pattern[] = "/tmp/entis-game-saves-XXXXXX";
    const char* created = mkdtemp(pattern);
    if (!created) return 1;
    const fs::path root(created), game = root / "game";
    try {
        fs::create_directory(game);
        const fs::path virtualRoot = "/__entis_saf__/save-test";
        {
            GameTestMount mount(virtualRoot, std::make_shared<MappedGameTestBackend>(game));
            entis::launcher::PrepareSaveDirectory(virtualRoot);
            Require(fs::is_directory(game / "savedata"), "save directory created in selected tree");
            std::ofstream(game / "savedata/slot.dat") << "existing save";
            entis::launcher::PrepareSaveDirectory(virtualRoot);
            Require(Read(game / "savedata/slot.dat") == "existing save", "existing selected saves retained");
            const auto saveRoot = (virtualRoot / "savedata").string(), slot = (virtualRoot / "savedata/slot.dat").string();
            {
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5);
                Require(bool(file), "virtual atomic save opened");
                file->BeginSave();
                Require(file->StagePrefix("BMP", 3, 0), "virtual thumbnail staged");
                file->BeginSave();
                Require(file->Replace("payload", 7, 3), "virtual serialized save replaced");
                Require(file->Close(), "virtual save close");
                Require(Read(game / "savedata/slot.dat") == "BMPpayload", "virtual write reaches selected folder");
            }
            {
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5);
                Require(bool(file), "second virtual save opened");
                file->BeginSave();
                Require(!file->Replace("bad", 3, 1), "failed serialization rejected");
                Require(file->Close(), "failed serialization close");
                Require(Read(game / "savedata/slot.dat") == "BMPpayload", "failed serializer preserves prior save");
            }
            bool escaped = false;
            try { entis::io::Open(virtualRoot / "../../outside", "wb"); } catch (const std::exception&) { escaped = true; }
            Require(escaped, "mapped-root traversal rejected");
        }
        {
            GameTestMount mount(virtualRoot, std::make_shared<FailingRename>(game));
            auto file = LegacyAtomicPath::OpenWithinRoot((virtualRoot / "savedata").string(), (virtualRoot / "savedata/slot.dat").string(), 5);
            Require(bool(file), "provider-failure save opened");
            file->BeginSave();
            Require(!file->Replace("rejected", 8, 0), "provider publish failure surfaced");
            Require(file->Close(), "failed provider staging discarded");
            Require(Read(game / "savedata/slot.dat") == "BMPpayload", "provider failure preserves prior save");
        }
        const auto nativeGame = root / "native-game";
        fs::create_directory(nativeGame);
        entis::launcher::PrepareSaveDirectory(nativeGame);
        Require(fs::is_directory(nativeGame / "savedata"), "native platform uses same game-relative save location");
        const auto emptyGame = root / "empty-game";
        fs::create_directory(emptyGame);
        entis::launcher::PrepareSaveDirectory(emptyGame);
        Require(fs::is_directory(emptyGame / "savedata"), "fresh game creates local savedata");
        const auto linkedGame = root / "linked-game";
        fs::create_directory(linkedGame); fs::create_directory_symlink(game / "savedata", linkedGame / "savedata");
        bool rejected = false;
        try { entis::launcher::PrepareSaveDirectory(linkedGame); } catch (const std::exception&) { rejected = true; }
        Require(rejected, "save symlink outside game is rejected");
        fs::remove_all(root);
        std::cout << "Game directory saves PASS: mkdir, existing saves, native and virtual IO, atomic replacement, failure preservation\n";
        return 0;
    } catch (const std::exception& error) {
        entis::io::SetBackend({}, {});
        std::cerr << error.what() << "; fixtures: " << root << '\n';
        return 1;
    }
}
