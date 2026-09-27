// Link with the SDL-backed gls4 target. All fixtures are created under argv[1].
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <functional>
#include <iterator>
#include <thread>
#include <vector>
#include "platform/sdl/system.h"
#include "platform/sdl/game_file_opener.h"
#include "../../fixtures/game_file_backend_fixture.h"
#include <sakura/sakura.h>
#include <sakuragl/erisa/sgl_erisa_archive_file.h>
#include <SDL3/SDL.h>

namespace fs = std::filesystem;
using namespace SSystem;
using namespace study::platform::sdl;

static void Require(bool condition, const char* description) {
    if (!condition) throw std::runtime_error(description);
}

static SString Wide(const std::string& text) {
    SString result;
    result.FromUTF8(reinterpret_cast<const uint8_t*>(text.c_str()));
    return result;
}

static std::string Read(SFileOpener& opener, const wchar_t* path) {
    std::unique_ptr<SFileInterface> file(opener.NewOpenFile(path, SFile::shareRead));
    Require(file != nullptr, "open fixture");
    std::string contents(static_cast<std::size_t>(file->GetLength()), '\0');
    Require(file->Read(contents.data(), contents.size()) == contents.size(), "read fixture");
    return contents;
}

static void Write(SFileOpener& opener, const wchar_t* path, const std::string& contents) {
    std::unique_ptr<SFileInterface> file(opener.NewOpenFile(path, SFile::modeCreate));
    Require(file != nullptr, "create fixture");
    Require(file->Write(contents.data(), contents.size()) == contents.size(), "write fixture");
}

struct SyntheticReadState {
    unsigned closed = 0, duplicated = 0;
    bool seekable = true, badRead = false;
    size_t maxRead = std::numeric_limits<size_t>::max();
    int64_t length = (int64_t{1} << 33) + 128;
};

// No large file allocation is needed to exercise 64-bit offsets and short reads.
class SyntheticReadFile final : public SFileInterface {
public:
    explicit SyntheticReadFile(std::shared_ptr<SyntheticReadState> state) : state_(std::move(state)) {}
    ~SyntheticReadFile() override { ++state_->closed; }
    static unsigned char ByteAt(int64_t offset) { return static_cast<unsigned char>(offset % 251); }
    SFileInterface* Duplicate() const override { ++state_->duplicated; return new SyntheticReadFile(state_); }
    size_t Read(void* data, size_t count) override {
        if (state_->badRead) return std::numeric_limits<size_t>::max();
        if (position_ >= state_->length) return 0;
        count = std::min(count, state_->maxRead);
        count = static_cast<size_t>(std::min<std::uint64_t>(count, state_->length - position_));
        auto* bytes = static_cast<unsigned char*>(data);
        for (size_t i = 0; i < count; ++i) bytes[i] = ByteAt(position_ + static_cast<int64_t>(i));
        position_ += static_cast<int64_t>(count);
        return count;
    }
    size_t Write(const void*, size_t) override { return 0; }
    bool IsSeekable() const override { return state_->seekable; }
    int64_t GetLength() const override { return state_->length; }
    int64_t Seek(int64_t offset, SeekOrigin origin = FromBegin) override {
        if (!state_->seekable || origin != FromBegin || offset < 0) return -1;
        return position_ = offset;
    }
    int64_t GetPosition() const override { return position_; }
    SError SetEndOfFile() override { return errNotSupported; }
private:
    std::shared_ptr<SyntheticReadState> state_;
    int64_t position_ = 0;
};

static void CheckSharedReadCursors() {
    auto state = std::make_shared<SyntheticReadState>();
    std::unique_ptr<SFileInterface> original(ShareReadOnlyFile(new SyntheticReadFile(state)));
    const int64_t large = (int64_t{1} << 32) + 17;
    Require(original->Seek(large) == large && original->GetLength() == state->length, "shared reader supports 64-bit positions and length");
    std::unique_ptr<SFileInterface> left(original->Duplicate()), right(original->Duplicate());
    Require(left && right && state->duplicated == 0 && left->GetPosition() == large, "duplicates reuse the owner without reopening or duplicating it");
    original.reset();
    Require(state->closed == 0, "original destruction retains the shared source");
    unsigned char bytes[8]{};
    Require(left->Read(bytes, 3) == 3 && bytes[0] == SyntheticReadFile::ByteAt(large) && right->GetPosition() == large,
            "interleaved readers keep independent logical positions");
    Require(right->Seek(-2, SFileInterface::FromEnd) == state->length - 2 && right->Read(bytes, sizeof(bytes)) == 2 && right->Read(bytes, 1) == 0,
            "end-relative seek and short EOF reads");
    Require(right->Seek(10, SFileInterface::FromCurrent) == state->length + 10 && right->Read(bytes, 1) == 0,
            "seeking beyond EOF does not clamp the logical cursor");
    const auto previous = right->GetPosition();
    Require(right->Seek(-1, SFileInterface::FromBegin) == -1 && right->GetPosition() == previous,
            "negative seek fails without moving cursor");
    Require(right->Seek(std::numeric_limits<int64_t>::max(), SFileInterface::FromCurrent) == -1 && right->GetPosition() == previous,
            "positive seek overflow fails without moving cursor");
    Require(right->Seek(std::numeric_limits<int64_t>::min(), SFileInterface::FromEnd) == -1 && right->GetPosition() == previous,
            "minimum signed offset does not overflow");
    Require(right->Seek(std::numeric_limits<int64_t>::max()) == std::numeric_limits<int64_t>::max() && right->Read(bytes, 1) == 0,
            "maximum logical cursor cannot overflow on read");
    Require(left->Write(bytes, 1) == 0 && left->SetEndOfFile() != errSuccess, "shared read cursors reject writes and truncation");
    state->maxRead = 2;
    Require(left->Read(bytes, sizeof(bytes)) == 2 && left->GetPosition() == large + 5, "short read advances only by bytes returned");
    state->badRead = true;
    Require(left->Read(bytes, sizeof(bytes)) == 0 && left->GetPosition() == large + 5, "SDK size_t(-1) read failure does not advance cursor");
    state->badRead = false; state->maxRead = std::numeric_limits<size_t>::max();
    std::atomic<bool> correct{true};
    auto readMany = [&](SFileInterface& file, int64_t begin) {
        std::array<unsigned char, 97> block{};
        for (int i = 0; i < 200; ++i) {
            const auto offset = begin + i * 101;
            if (file.Seek(offset) != offset || file.Read(block.data(), block.size()) != block.size()) { correct = false; return; }
            for (size_t j = 0; j < block.size(); ++j)
                if (block[j] != SyntheticReadFile::ByteAt(offset + static_cast<int64_t>(j))) { correct = false; return; }
        }
    };
    std::thread a(readMany, std::ref(*left), large), b(readMany, std::ref(*right), int64_t{73});
    a.join(); b.join();
    Require(correct, "concurrent clone reads serialize each shared seek and read together");
    left.reset(); Require(state->closed == 0, "remaining clone retains owner");
    right.reset(); Require(state->closed == 1, "last clone closes the source exactly once");
    auto pipeState = std::make_shared<SyntheticReadState>(); pipeState->seekable = false;
    auto* pipe = new SyntheticReadFile(pipeState);
    std::unique_ptr<SFileInterface> unchanged(ShareReadOnlyFile(pipe));
    Require(unchanged.get() == pipe && !unchanged->IsSeekable(), "non-seekable source is returned unchanged");
}

static void WriteArchive(const fs::path& path, const std::vector<std::pair<const wchar_t*, std::string>>& files) {
    ERISA::SGLArchiveFile::SDirectory directory;
    ERISA::SGLArchiveFile::FILE_ENTRY_EX entry{};
    entry.nEncodeType = ERISA::SGLArchiveFile::encodeRaw;
    for (const auto& item : files) directory.AddFileEntry(item.first, entry);
    SStandardFileOpener native;
    std::unique_ptr<SFileInterface> file(native.NewOpenFile(Wide(path.u8string()), SFile::modeCreate));
    Require(file != nullptr, "create synthetic archive");
    ERISA::SGLArchiveFile archive;
    Require(archive.OpenArchive(file.get(), false, SFile::modeCreate, &directory) == errSuccess, "write synthetic archive index");
    for (const auto& item : files) {
        Require(archive.DescendFile(item.first) == errSuccess, "open synthetic archive member for writing");
        Require(archive.Write(item.second.data(), item.second.size()) == item.second.size() && archive.AscendFile() == errSuccess,
                "write synthetic archive member");
    }
    Require(archive.CloseArchive() == errSuccess, "finish synthetic archive");
}

static void CheckArchiveReaders(SFileOpener& opener, const fs::path& game, const std::shared_ptr<MappedGameTestBackend>& backend) {
    WriteArchive(game / "inner.noa", {{L"one.bin", "first-data"}, {L"two.bin", "second-data"}});
    std::ifstream input(game / "inner.noa", std::ios::binary);
    std::string inner((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    WriteArchive(game / "nested.noa", {{L"skin.noa", inner}});
    const auto beforeOpens = backend->opens;
    std::unique_ptr<SFileInterface> base(opener.NewOpenFile(L"nested.noa", SFile::shareRead));
    ERISA::SGLArchiveFile outer, nested;
    Require(base && outer.OpenArchive(base.get(), false) == errSuccess, "open provider-backed nested archive");
    std::unique_ptr<SFileInterface> skin(outer.NewOpenFile(L"skin.noa", SFile::shareRead));
    Require(skin && nested.OpenArchive(skin.get(), false) == errSuccess, "open raw nested archive member");
    for (int i = 0; i < 200; ++i) {
        std::unique_ptr<SFileInterface> one(nested.NewOpenFile(L"one.bin", SFile::shareRead));
        std::unique_ptr<SFileInterface> two(nested.NewOpenFile(L"two.bin", SFile::shareRead));
        char first[10]{}, second[11]{};
        Require(one && two && one->Read(first, 3) == 3 && two->Read(second, 11) == 11 && one->Read(first + 3, 7) == 7,
                "interleave SDK archive member domains");
        Require(std::string(first, 10) == "first-data" && std::string(second, 11) == "second-data", "SDK archive members preserve byte contents");
    }
    Require(backend->opens == beforeOpens + 1, "400 nested resource opens share one provider descriptor");
    nested.CloseArchive(); skin.reset(); outer.CloseArchive(); base.reset();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: sdl_system_test <temporary-fixture-parent>\n");
        return 2;
    }
    if (!SDL_Init(0)) return 2;
    InstallStdoutLogging();
    const auto root = fs::absolute(argv[1]) / ("sdl-system-" + std::to_string(SDL_GetPerformanceCounter()));
    bool initialized = false;
    try {
        const auto assets = root / "assets";
        const auto game = root / u8"Original Game 日本語";
        const auto storage = root / "storage";
        const auto local = root / "local";
        fs::create_directories(assets);
        fs::create_directories(game);
        fs::create_directories(storage / "game2");
        fs::create_directories(local / "savedata");
        std::ofstream(assets / "cotopha.xml") << "<cotopha/>";
        std::ofstream(game / "script.noa") << "game-fixture";
        std::ofstream(storage / "game2/script.noa") << "other-fixture";
        SystemPaths paths;
        paths.assetsRoot = assets.string();
        paths.storageRoot = storage.string();
        paths.localRoot = local.string();
        paths.gameRoot = game.string();
        SystemPaths invalid = paths;
        invalid.localRoot = "relative/path";
        Require(!ConfigureSystemPaths(invalid), "relative roots must be rejected");
        Require(ConfigureSystemPaths(paths), "configure explicit roots");
        SSystem::Initialize();
        initialized = true;
        Require(!ConfigureSystemPaths(paths), "live paths must remain frozen");
        {
        CheckSharedReadCursors();
        Require(Read(g_defURLOpener, L"storage://game/script.noa") == "game-fixture", "game root override");
        Require(Read(g_defURLOpener, L"storage://game2/script.noa") == "other-fixture", "game prefix boundary");
        SString direct;
        Require(g_defURLOpener.DirectPathOf(direct, L"storage://game/script.noa") == errSuccess, "direct path status");
        Require(direct == Wide((game / "script.noa").string()), "direct path matches selected game root");
        SString defaultDirectory;
        Require(SFile::GetDefaultDirectory(defaultDirectory, SFile::DefaultDirectory::CurrentDirectory) == errSuccess &&
                defaultDirectory == Wide(game.string()), "default CURRENT directory uses game override");
        Require(SFile::GetDefaultDirectory(defaultDirectory, SFile::DefaultDirectory::ApplicationData) == errSuccess &&
                defaultDirectory == Wide(local.string()), "default AppData directory uses save root");
        SFileOpener::State state{};
        Require(g_defURLOpener.QueryState(L"storage://game/script.noa", state) == errSuccess &&
                state.nFileSize == 12, "file attributes use selected root");

        std::unique_ptr<SFileOpener> current(g_defURLOpener.NewOffsetOpener(L"storage://game", L'/'));
        Require(Read(*current, L"script.noa") == "game-fixture", "CURRENT offset opener retains game redirection");
        fs::create_directories(game / "nested-assets");
        std::ofstream(game / "nested-assets/owner.eri") << "image";
        std::ofstream(game / "nested-assets/reference.eri") << "sibling-reference";
        std::unique_ptr<SFileInterface> nestedFile(current->NewOpenFile(L"nested-assets/owner.eri", SFile::shareRead));
        Require(nestedFile && nestedFile->IsExisting(L"reference.eri") && Read(*nestedFile, L"reference.eri") == "sibling-reference",
                "shared native stream preserves its relative reference-image opener");
        SString siblingPath;
        Require(nestedFile->DirectPathOf(siblingPath, L"reference.eri") == errSuccess &&
                siblingPath == Wide((game / "nested-assets/reference.eri").string()), "shared stream delegates direct-path resolution");
        nestedFile.reset();
        fs::remove_all(game / "nested-assets");
#if !defined(_WIN32)
        // The SDK's Windows share flags can prohibit replacing an open file.
        // Preserve those flags; POSIX allows us to verify the original inode stays alive.
        Write(*current, L"replace.txt", "before");
        std::unique_ptr<SFileInterface> oldVersion(current->NewOpenFile(L"replace.txt", SFile::shareRead));
        Require(oldVersion && oldVersion->Seek(2) == 2, "open native shared read cursor");
        fs::rename(game / "replace.txt", game / "previous.txt");
        Write(*current, L"replace.txt", "after!");
        std::unique_ptr<SFileInterface> oldClone(oldVersion->Duplicate());
        oldVersion.reset();
        char oldBytes[4]{};
        Require(oldClone && oldClone->Read(oldBytes, sizeof(oldBytes)) == sizeof(oldBytes) && std::string(oldBytes, sizeof(oldBytes)) == "fore",
                "native duplicate retains the opened file after path replacement and owner destruction");
        Require(Read(*current, L"replace.txt") == "after!", "independent native open observes replacement instead of a path cache");
        oldClone.reset();
        fs::remove(game / "replace.txt"); fs::remove(game / "previous.txt");
#endif
        Require(g_defURLOpener.IsExisting(L"storage://game"), "selected game root exists without trailing slash");
        Require(g_defURLOpener.IsExisting(L"storage://game/"), "selected game root exists with trailing slash");
        SObjectArray<SString> directories;
        g_defURLOpener.ListSubDirectories(directories, L"storage://");
        bool listed = false;
        for (size_t i = 0; i < directories.GetLength(); ++i)
            if (directories.GetAt(i) && *directories.GetAt(i) == L"game") listed = true;
        Require(listed, "virtual game directory is visible in storage listing");
        Require(g_defURLOpener.CreateSubDirectory(L"storage://game/probe", 0) == errSuccess, "create directory via game route");
        Write(g_defURLOpener, L"storage://game/probe/one.txt", "one");
        Require(fs::is_regular_file(game / "probe/one.txt"), "created file reaches selected game root");
        Require(g_defURLOpener.RenameSubFile(L"storage://game/probe/one.txt", L"storage://game/probe/two.txt") == errSuccess, "rename inside game root");
        Require(g_defURLOpener.RenameSubFile(L"storage://game/probe/two.txt", L"storage://moved.txt") == errSuccess, "rename across storage and game roots");
        Require(fs::is_regular_file(storage / "moved.txt"), "cross-root destination");
        Require(g_defURLOpener.RemoveSubFile(L"storage://moved.txt") == errSuccess, "remove file");
        Require(g_defURLOpener.RemoveSubDirectory(L"storage://game/probe") == errSuccess, "remove selected game subdirectory");

        Write(g_defURLOpener, L"local://savedata/context.dat", "save-fixture");
        Require(Read(g_defURLOpener, L"local://savedata/context.dat") == "save-fixture", "local saves round trip");
        Require(fs::is_regular_file(local / "savedata/context.dat"), "save path is independent of game assets");
        Require(Read(g_defURLOpener, L"assets://cotopha.xml") == "<cotopha/>", "packaged config read");
        std::unique_ptr<SFileInterface> asset(g_defURLOpener.NewOpenFile(L"assets://cotopha.xml", SFile::shareRead));
        Require(asset && asset->Seek(3) == 3, "asset seek");
        std::unique_ptr<SFileInterface> duplicate(asset->Duplicate());
        Require(duplicate && duplicate->GetPosition() == 3 && duplicate->GetLength() == 10, "asset duplication preserves position");
        Require(asset->SetEndOfFile() == errNotSupported, "assets refuse truncation");
        asset.reset();
        duplicate.reset();
        current.reset();
        directories.FreeArray();

        SSystem::ResetCurrentMilliSec(100);
        SDL_Delay(2);
        Require(SSystem::CurrentMilliSec() >= 102, "monotonic millisecond clock");
        Require(SSystem::GetPerformanceFrequency() > 0, "performance frequency");
        DATE_TIME date{};
        SSystem::CurrentLocalDate(date);
        Require(date.nYear >= 1970 && date.nMonth >= 1 && date.nMonth <= 12, "local date conversion");
        PLATFORM_INFORMATION info{};
        SSystem::GetPlatformInformation(info);
        Require(info.runtimeArchitecture == sizeof(void*) * 8, "correct runtime bitness");
        MEMORY_STATUS memory{};
        SSystem::GetMemoryStatus(memory);
        Require(memory.nTotalPhys > 0, "actual physical memory query");
        SString unused;
        Require(SSystem::BrowseOpenFileDialog(unused) == errNotSupported, "unsupported picker reports explicit error");
        } // Release all SDK-owned strings/files before its heap is finalized.
        SSystem::Finalize();
        initialized = false;
        Require(ConfigureSystemPaths(paths), "configuration can change after finalization");
        {
            const auto virtualRoot = root / "selected-document-tree";
            auto backend = std::make_shared<MappedGameTestBackend>(game);
            GameTestMount mount(virtualRoot, backend);
            paths.gameRoot = virtualRoot.string();
            Require(ConfigureSystemPaths(paths), "configure selected document tree");
            SSystem::Initialize();
            initialized = true;
            {
                SString currentDirectory;
                Require(SFile::GetDefaultDirectory(currentDirectory, SFile::DefaultDirectory::CurrentDirectory) == errSuccess &&
                        currentDirectory == L"storage://game", "virtual CURRENT remains on the SDK game router");
                std::unique_ptr<SFileOpener> current(g_defURLOpener.NewOffsetOpener(currentDirectory, L'/'));
                Require(Read(*current, L"script.noa") == "game-fixture", "CURRENT reads selected tree without copied resources");
                Require(!fs::exists(virtualRoot), "SDK accesses do not create a synthetic resource directory");
                SFileOpener::State state{};
                Require(g_defURLOpener.QueryState(L"storage://game/script.noa", state) == errSuccess && state.nFileSize == 12,
                        "document file attributes reach selected provider");
                Require((state.bitFields & SFileOpener::fieldModifiedTime) && state.dtModified.nYear >= 2020,
                        "provider modification dates remain available to save and load screens");
                SObjectArray<SString> files;
                current->ListSubFiles(files, L"");
                Require(files.GetLength() == 1 && *files.GetAt(0) == L"script.noa", "selected-tree file enumeration");
                files.FreeArray();
                Require(current->CreateSubDirectory(L"savedata", 0) == errSuccess, "savedata directory created in selected tree");
                Write(*current, L"savedata/slot.dat", "saved-game");
                Require(fs::is_regular_file(game / "savedata/slot.dat") && Read(*current, L"savedata/slot.dat") == "saved-game",
                        "SDK saves directly into selected folder");
                SString direct;
                Require(current->DirectPathOf(direct, L"savedata/slot.dat") == errSuccess &&
                        direct == Wide((virtualRoot / "savedata/slot.dat").string()), "atomic save path retains virtual provider identity");
                std::unique_ptr<SFileInterface> source(current->NewOpenFile(L"savedata/slot.dat", SFile::shareRead));
                Require(source && source->Seek(2) == 2, "provider stream supports seeking");
                const auto beforeDuplicate = backend->opens;
                std::unique_ptr<SFileInterface> duplicate(source->Duplicate());
                Require(duplicate && duplicate->GetPosition() == 2 && duplicate->GetLength() == 10,
                        "provider duplicate preserves stream position and length");
                Require(backend->opens == beforeDuplicate, "provider duplicate does not reopen the original file");
                source.reset(); duplicate.reset();
                std::unique_ptr<SFileInterface> writable(current->NewOpenFile(L"savedata/slot.dat", SFile::modeReadWrite));
                Require(writable && writable->Seek(2) == 2, "writable provider stream retains its original mode");
                const auto beforeWritableDuplicate = backend->opens;
                std::unique_ptr<SFileInterface> writableCopy(writable->Duplicate());
                Require(writableCopy && backend->opens == beforeWritableDuplicate + 1 && writableCopy->Write("X", 1) == 1,
                        "writable duplicate keeps the existing reopen-and-write behavior");
                writable.reset(); writableCopy.reset();
                Require(current->RenameSubFile(L"savedata/slot.dat", L"savedata/renamed.dat") == errSuccess,
                        "provider rename stays inside selected tree");
                Require(g_defURLOpener.RenameSubFile(L"storage://game/savedata/renamed.dat", L"storage://outside.dat") != errSuccess &&
                        fs::is_regular_file(game / "savedata/renamed.dat") && !fs::exists(storage / "outside.dat"),
                        "cross-provider rename fails without changing original save");
                std::unique_ptr<SFileInterface> escaped(current->NewOpenFile(L"../escaped.dat", SFile::modeCreate));
                Require(!escaped && !fs::exists(root / "escaped.dat"), "provider rejects traversal outside selected directory");
                Require(current->RemoveSubFile(L"savedata/renamed.dat") == errSuccess &&
                        current->RemoveSubDirectory(L"savedata") == errSuccess, "provider deletion updates selected directory");
                Require(backend->opens >= 4 && backend->lists > 0, "SDK file operations use injected provider");
                CheckArchiveReaders(*current, game, backend);
                std::unique_ptr<SFileInterface> retained(current->NewOpenFile(L"script.noa", SFile::shareRead));
                Require(retained != nullptr, "open provider stream before clearing route");
                const auto beforeUnmount = backend->opens;
                entis::io::SetBackend({}, {});
                std::unique_ptr<SFileInterface> afterUnmount(retained->Duplicate());
                retained.reset();
                char retainedBytes[12]{};
                Require(afterUnmount && afterUnmount->Read(retainedBytes, sizeof(retainedBytes)) == sizeof(retainedBytes) &&
                        std::string(retainedBytes, sizeof(retainedBytes)) == "game-fixture" && backend->opens == beforeUnmount,
                        "opened descriptor and duplicates survive backend reset without routing a new open");
            }
            SSystem::Finalize();
            initialized = false;
        }
        fs::remove_all(root);
        SDL_Quit();
        std::puts("SDL system services and unified path routing: PASS");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "SDL system probe failed: %s; SDL: %s; fixtures: %s\n", error.what(), SDL_GetError(), root.c_str());
        if (initialized) SSystem::Finalize();
        SDL_Quit();
        return 1;
    }
}
