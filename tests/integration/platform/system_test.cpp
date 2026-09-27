// Link with the SDL-backed gls4 target. All fixtures are created under argv[1].
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdio>
#include "platform/sdl/system.h"
#include "../../fixtures/game_file_backend_fixture.h"
#include <sakura/sakura.h>
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
                std::unique_ptr<SFileInterface> duplicate(source->Duplicate());
                Require(duplicate && duplicate->GetPosition() == 2 && duplicate->GetLength() == 10,
                        "provider duplicate preserves stream position and length");
                source.reset(); duplicate.reset();
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
