#include "io/save_directory.h"
#include "../../fixtures/game_file_backend_fixture.h"
#include "runtime/cotopha_port/legacy_atomic_path.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <map>
#include <unistd.h>

namespace fs = std::filesystem;
static void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
static std::string Read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
class SaveBackend final : public entis::io::Backend {
public:
    explicit SaveBackend(fs::path path) : delegate(std::move(path)) {}
    entis::io::FileInfo Stat(const std::string& p) override {
        ++statPaths[p];
        if (denySaveRoot && p == "savedata") throw std::runtime_error("provider access revoked");
        return delegate.Stat(p);
    }
    std::vector<entis::io::Entry> List(const std::string& p) override { return delegate.List(p); }
    FILE* Open(const std::string& p, const char* mode) override {
        const bool temporary = p.find(".tmp-") != std::string::npos;
        if (temporary && !std::strcmp(mode, "w+b")) {
            ++temporaryCreates;
            if (failTemporaryCreate) { --failTemporaryCreate; throw std::runtime_error("provider rejected staging creation"); }
        }
        if (temporary && !std::strcmp(mode, "rb")) {
            ++readbacks;
            if (failReadback) { --failReadback; throw std::runtime_error("provider rejected verification open"); }
            if (corruptReadback) {
                --corruptReadback;
                std::fstream output(delegate.root / p, std::ios::binary | std::ios::in | std::ios::out);
                output.seekp(-1, std::ios::end); output.put('!');
            }
        }
        if (temporary && !std::strcmp(mode, "r+b") && failPrivateReopen) {
            --failPrivateReopen; throw std::runtime_error("provider rejected private reopen");
        }
        return delegate.Open(p, mode);
    }
    void CreateDirectory(const std::string& p) override { delegate.CreateDirectory(p); }
    void Remove(const std::string& p, bool directory) override { ++removes; delegate.Remove(p, directory); }
    void Rename(const std::string& from, const std::string& to) override {
        ++renames;
        if (failRename) { --failRename; throw std::runtime_error("provider rejected replacement"); }
        delegate.Rename(from, to);
    }
    MappedGameTestBackend delegate;
    unsigned temporaryCreates = 0, readbacks = 0, removes = 0, renames = 0;
    unsigned failReadback = 0, corruptReadback = 0, failPrivateReopen = 0, failRename = 0;
    unsigned failTemporaryCreate = 0;
    bool denySaveRoot = false;
    std::map<std::string, unsigned> statPaths;
};
static void NoTemporaryFiles(const fs::path& directory) {
    for (const auto& file : fs::directory_iterator(directory))
        Require(file.path().filename().string().find(".tmp-") == std::string::npos, "staging file cleaned up");
}
int main() {
    char pattern[] = "/tmp/entis-game-saves-XXXXXX";
    const char* created = mkdtemp(pattern);
    if (!created) return 1;
    const fs::path root(created), game = root / "game";
    try {
        fs::create_directory(game);
        const fs::path virtualRoot = "/__entis_saf__/save-test";
        {
            auto backend = std::make_shared<SaveBackend>(game);
            GameTestMount mount(virtualRoot, backend);
            entis::launcher::PrepareSaveDirectory(virtualRoot);
            Require(fs::is_directory(game / "savedata"), "save directory created in selected tree");
            std::ofstream(game / "savedata/slot.dat") << "existing save";
            entis::launcher::PrepareSaveDirectory(virtualRoot);
            Require(Read(game / "savedata/slot.dat") == "existing save", "existing selected saves retained");
            const auto saveRoot = (virtualRoot / "savedata").string(), slot = (virtualRoot / "savedata/slot.dat").string();
            {
                const unsigned beforeScope = backend->statPaths["savedata"];
                bool candidate = false;
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5, &candidate);
                Require(bool(file) && candidate, "virtual atomic save opened in verified scope");
                Require(backend->statPaths["savedata"] == beforeScope + 1, "one root/parent query per save open");
                file->BeginSave();
                Require(file->StagePrefix("BMP", 3, 0), "virtual thumbnail staged");
                file->BeginSave();
                Require(file->Replace("payload", 7, 3), "virtual serialized save replaced");
                Require(file->Close(), "virtual save close");
                Require(Read(game / "savedata/slot.dat") == "BMPpayload", "virtual write reaches selected folder");
                Require(backend->temporaryCreates == 1 && backend->readbacks == 1 && backend->renames == 1 &&
                    backend->removes == 0, "thumbnail and context share one private file with full read-back verification");
                NoTemporaryFiles(game / "savedata");
            }
            {
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5);
                Require(bool(file), "second virtual save opened");
                file->BeginSave();
                Require(!file->Replace("bad", 3, 1), "failed serialization rejected");
                Require(file->Close(), "failed serialization close");
                Require(Read(game / "savedata/slot.dat") == "BMPpayload", "failed serializer preserves prior save");
                NoTemporaryFiles(game / "savedata");
            }
            {
                const unsigned before = backend->temporaryCreates;
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 6);
                Require(bool(file), "published file opened without truncate");
                Require(file->StagePrefix("ABCDEF", 6, 0) && file->StagePrefix("xy", 2, 2), "prefix updates share private copy");
                Require(Read(game / "savedata/slot.dat") == "BMPpayload", "prefix staging keeps published bytes unchanged");
                Require(file->Replace("short", 5, 4), "replacement truncates copied tail");
                Require(Read(game / "savedata/slot.dat") == "ABxyshort", "updated prefix and exact body published");
                Require(backend->temporaryCreates == before + 1, "published prefix needs one COW file, reused by Replace");
                Require(file->Replace("next", 4, 2), "successive save on same open file");
                Require(backend->temporaryCreates == before + 2 && Read(game / "savedata/slot.dat") == "ABnext",
                    "successive save COW protects newly published target");
                Require(file->Close(), "successive saves close");
                NoTemporaryFiles(game / "savedata");
            }
            // All failures occur after the private body was written. Keep the old
            // published save, then retry through the same logical open and temp.
            for (unsigned failure = 0; failure != 4; ++failure) {
                const std::string old = Read(game / "savedata/slot.dat");
                const unsigned before = backend->temporaryCreates;
                auto file = LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5);
                Require(bool(file) && file->StagePrefix("BMP", 3, 0), "failure fixture staged");
                if (failure == 0) backend->failReadback = 1;
                if (failure == 1) backend->corruptReadback = 1;
                if (failure == 2) backend->failRename = 1;
                if (failure == 3) { backend->failReadback = 1; backend->failPrivateReopen = 1; }
                Require(!file->Replace("first-body", 10, 3), "provider failure rejects publication");
                Require(Read(game / "savedata/slot.dat") == old, "failed verification or rename preserves published save");
                Require(file->Write("X", 1, 0) == 0 && !file->Truncate(3), "ordinary IO cannot publish a failed body");
                Require(!file->Replace("new", 3, 4), "failed body bytes cannot become a trusted prefix");
                Require(file->StagePrefix("BMP", 3, 0) && file->Write("X", 1, 0) == 0,
                    "restaging a prefix still requires body validation");
                Require(file->Replace("retry", 5, 3), "private staging can reopen and retry after provider failure");
                Require(file->Close() && Read(game / "savedata/slot.dat") == "BMPretry", "retry truncates failed body tail");
                Require(backend->temporaryCreates == before + 1, "failure retry reuses original private file");
                NoTemporaryFiles(game / "savedata");
            }
            bool escaped = false;
            try { entis::io::Open(virtualRoot / "../../outside", "wb"); } catch (const std::exception&) { escaped = true; }
            Require(escaped, "mapped-root traversal rejected");
        }
        {
            auto backend = std::make_shared<SaveBackend>(game);
            backend->failRename = 1;
            GameTestMount mount(virtualRoot, backend);
            auto file = LegacyAtomicPath::OpenWithinRoot((virtualRoot / "savedata").string(), (virtualRoot / "savedata/slot.dat").string(), 5);
            Require(bool(file), "provider-failure save opened");
            file->BeginSave();
            Require(!file->Replace("rejected", 8, 0), "provider publish failure surfaced");
            Require(file->Close(), "failed provider staging discarded");
            Require(Read(game / "savedata/slot.dat") == "BMPretry", "provider failure preserves prior save");
            NoTemporaryFiles(game / "savedata");
        }
        {
            auto backend = std::make_shared<SaveBackend>(game);
            GameTestMount mount(virtualRoot, backend);
            const auto saveRoot = (virtualRoot / "savedata").string(), slot = (virtualRoot / "savedata/slot.dat").string();
            bool candidate = true;
            Require(!LegacyAtomicPath::OpenWithinRoot(saveRoot, (virtualRoot / "outside.dat").string(), 5, &candidate) && !candidate,
                "out-of-scope path may use another appropriate file opener");
            const unsigned beforeScope = backend->statPaths["savedata"];
            backend->failTemporaryCreate = 1;
            Require(!LegacyAtomicPath::OpenWithinRoot(saveRoot + "/", slot, 5, &candidate) && candidate,
                "scoped staging failure forbids fallback that would truncate target");
            Require(backend->statPaths["savedata"] == beforeScope + 1,
                "trailing slash still reuses this call's identical root and parent");
            Require(Read(game / "savedata/slot.dat") == "BMPretry", "staging create failure preserves old save");
            fs::create_directory(game / "savedata/nested");
            const unsigned rootBefore = backend->statPaths["savedata"], parentBefore = backend->statPaths["savedata/nested"];
            auto nested = LegacyAtomicPath::OpenWithinRoot(saveRoot, (virtualRoot / "savedata/nested/new.dat").string(), 5, &candidate);
            Require(bool(nested) && candidate, "nested scoped open accepted");
            Require(backend->statPaths["savedata"] == rootBefore + 1 && backend->statPaths["savedata/nested"] == parentBefore + 1,
                "nested parent is canonicalized once and reused for the opened path");
            nested->BeginSave(); Require(nested->Close(), "unused nested staging discarded");
            Require(!LegacyAtomicPath::OpenWithinRoot(saveRoot, (virtualRoot / "savedata/nested").string(), 5, &candidate) && candidate,
                "in-scope directory rejection also forbids truncating fallback");
            backend->denySaveRoot = true;
            candidate = true;
            Require(!LegacyAtomicPath::OpenWithinRoot(saveRoot, slot, 5, &candidate) && !candidate,
                "new logical open rechecks revoked provider access instead of caching scope");
            NoTemporaryFiles(game / "savedata");
            NoTemporaryFiles(game / "savedata/nested");
        }
        {
            auto backend = std::make_shared<SaveBackend>(game);
            backend->corruptReadback = 1;
            GameTestMount mount(virtualRoot, backend);
            auto file = LegacyAtomicPath::OpenWithinRoot((virtualRoot / "savedata").string(), (virtualRoot / "savedata/slot.dat").string(), 5);
            Require(bool(file) && file->StagePrefix("BMP", 3, 0), "corrupt body close fixture");
            Require(!file->Replace("failed-body", 11, 3), "corrupt read-back prevents commit");
            Require(file->Write("X", 1, 0) == 0 && !file->Truncate(3), "corrupt body blocks ordinary publication");
            Require(file->StagePrefix("NEW", 3, 0), "private thumbnail can still be edited");
            Require(file->Close() && Read(game / "savedata/slot.dat") == "BMPretry",
                "prefix edit plus close discards unvalidated body and preserves old save");
            NoTemporaryFiles(game / "savedata");
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
