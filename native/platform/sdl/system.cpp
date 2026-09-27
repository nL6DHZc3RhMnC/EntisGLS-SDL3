#include "platform/sdl/system.h"
#include "platform/sdl/memory_info.h"
#include "platform/sdl/game_file_opener.h"
#include "io/game_files.h"

#include <sakura/sakura.h>
#include <sakura/ssys_fragment_file.h>
#include <sakura/ssys_http_file.h>
#include <SDL3/SDL.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace study::platform::sdl {
namespace {
using namespace SSystem;
SystemPaths systemPaths;
bool configured = false;
bool inUse = false;
std::mutex configurationMutex;
std::atomic<SDL_Window*> dialogParent{nullptr};
SymbolResolver symbolResolver = nullptr;
void* symbolResolverData = nullptr;

std::string UTF8(const wchar_t* text) {
    if (!text) return {};
    const auto bytes = SString(text).ToUTF8();
    if (!bytes.GetLength()) return {};
    std::string result(reinterpret_cast<const char*>(bytes.GetConstArray()), bytes.GetLength());
    if (!result.empty() && result.back() == '\0') result.pop_back();
    return result;
}

SString Wide(const std::string& text) {
    SString result;
    result.FromUTF8(reinterpret_cast<const uint8_t*>(text.c_str()));
    return result;
}

bool IsAbsolute(const std::string& path) {
    return !path.empty() && (path.front() == '/' || path.front() == '\\' ||
        (path.size() > 2 && path[1] == ':' && (path[2] == '/' || path[2] == '\\')));
}

std::string Join(const std::string& root, const std::string& relative) {
    if (root.empty()) return relative;
    return root + (root.back() == '/' || root.back() == '\\' ? "" : "/") + relative;
}

bool AssetPath(const wchar_t* source, std::string& result) {
    const std::string raw = UTF8(source);
    std::string relative;
    std::size_t offset = 0;
    while (offset < raw.size()) {
        const auto end = raw.find_first_of("/\\", offset);
        const auto component = raw.substr(offset, end == std::string::npos ? end : end - offset);
        if (component == ".." || component.find(':') != std::string::npos)
            return SDL_SetError("Asset paths must stay within their configured root");
        if (!component.empty() && component != ".") {
            if (!relative.empty()) relative += '/';
            relative += component;
        }
        if (end == std::string::npos) break;
        offset = end + 1;
    }
    result = Join(systemPaths.assetsRoot, relative);
    return true;
}

class AssetFile final : public SFileInterface {
public:
    AssetFile(std::string path, SDL_IOStream* stream) : path_(std::move(path)), stream_(stream) {}
    ~AssetFile() override { SDL_CloseIO(stream_); }
    SFileInterface* Duplicate() const override {
        auto* copy = SDL_IOFromFile(path_.c_str(), "rb");
        if (!copy) return nullptr;
        if (SDL_SeekIO(copy, SDL_TellIO(stream_), SDL_IO_SEEK_SET) < 0) {
            SDL_CloseIO(copy);
            return nullptr;
        }
        return new AssetFile(path_, copy);
    }
    size_t Read(void* buffer, size_t bytes) override { return SDL_ReadIO(stream_, buffer, bytes); }
    size_t Write(const void*, size_t) override {
        SDL_SetError("Packaged assets are read-only");
        return 0;
    }
    bool IsSeekable() const override { return SDL_TellIO(stream_) >= 0; }
    int64_t GetLength() const override { return SDL_GetIOSize(stream_); }
    int64_t Seek(int64_t offset, SeekOrigin origin) override {
        if (origin < FromBegin || origin > FromEnd) return -1;
        const SDL_IOWhence modes[] = {SDL_IO_SEEK_SET, SDL_IO_SEEK_CUR, SDL_IO_SEEK_END};
        return SDL_SeekIO(stream_, offset, modes[origin]);
    }
    int64_t GetPosition() const override { return SDL_TellIO(stream_); }
    SError SetEndOfFile() override { return errNotSupported; }
private:
    std::string path_;
    SDL_IOStream* stream_;
};

class AssetOpener final : public SFileOpener {
public:
    SFileInterface* NewOpenFile(const wchar_t* name, long flags) override {
        if ((flags & (modeCreateFlag | modeWriteFlag | modeCreateDirFlag)) || !(flags & modeReadFlag)) {
            SDL_SetError("Packaged assets only support reading");
            return nullptr;
        }
        std::string path;
        if (!AssetPath(name, path)) return nullptr;
        auto* stream = SDL_IOFromFile(path.c_str(), "rb");
        return stream ? new AssetFile(path, stream) : nullptr;
    }
    bool IsExisting(const wchar_t* name) override {
        auto* file = NewOpenFile(name, modeRead);
        if (!file) return false;
        delete file;
        return true;
    }
    SError QueryState(const wchar_t* name, State& state) override {
        std::memset(&state, 0, sizeof(state));
        auto* file = NewOpenFile(name, modeRead);
        if (!file) return errFailed;
        const auto length = file->GetLength();
        delete file;
        if (length < 0) return errFailed;
        state.bitFields = fieldAttributes | fieldFileSize;
        state.bitAttributes = permissionRUSR;
        state.nFileSize = static_cast<uint64_t>(length);
        return errSuccess;
    }
    void ListSubFiles(SObjectArray<SString>& files, const wchar_t* directory) override {
        Enumerate(files, directory, false);
    }
    void ListSubDirectories(SObjectArray<SString>& directories, const wchar_t* directory) override {
        Enumerate(directories, directory, true);
    }
private:
    static void Enumerate(SObjectArray<SString>& results, const wchar_t* directory, bool directories) {
        if (systemPaths.assetsRoot.empty()) {
            SDL_SetError("Packaged-asset directory enumeration is not supported by this SDL backend");
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            return;
        }
        std::string path;
        if (!AssetPath(directory, path)) return;
        struct Context { SObjectArray<SString>& results; bool directories; } context{results, directories};
        const auto callback = [](void* userdata, const char* directoryName, const char* name) {
            auto& ctx = *static_cast<Context*>(userdata);
            SDL_PathInfo info{};
            if (!SDL_GetPathInfo(Join(directoryName, name).c_str(), &info)) return SDL_ENUM_FAILURE;
            if ((info.type == SDL_PATHTYPE_DIRECTORY) == ctx.directories)
                ctx.results.Add(new SString(Wide(name)));
            return SDL_ENUM_CONTINUE;
        };
        if (!SDL_EnumerateDirectory(path.c_str(), callback, &context))
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Asset enumeration failed: %s", SDL_GetError());
    }
};

// Route every file operation through the same two SDK offset openers. In
// particular, NewOffsetOpener("storage://game") retains this router, so later
// relative paths still reach an externally selected game directory.
class StorageOpener final : public SFileOpener {
public:
    StorageOpener(const std::string& storage, const std::string& game)
        : storage_(Wide(storage), L'/', new SStandardFileOpener, true),
          game_(Wide(game.empty() ? Join(storage, "game") : game), L'/', NewGameFileOpener(), true),
          hasOverride_(!game.empty()) {}
    SFileInterface* NewOpenFile(const wchar_t* path, long flags) override {
        const auto resolved = Resolve(path);
        return resolved.opener->NewOpenFile(resolved.path, flags);
    }
    bool IsExisting(const wchar_t* path) override {
        const auto resolved = Resolve(path);
        if (!*resolved.path) {
            State state{};
            return resolved.opener->QueryState(L"", state) == errSuccess;
        }
        return resolved.opener->IsExisting(resolved.path);
    }
    SError QueryState(const wchar_t* path, State& state) override {
        const auto resolved = Resolve(path);
        return resolved.opener->QueryState(resolved.path, state);
    }
    void ListSubFiles(SObjectArray<SString>& files, const wchar_t* path) override {
        const auto resolved = Resolve(path);
        resolved.opener->ListSubFiles(files, resolved.path);
        if (hasOverride_ && resolved.opener == &storage_ && !*resolved.path) {
            for (size_t i = files.GetLength(); i > 0; --i) {
                const auto* name = files.GetAt(i - 1);
                if (name && SString::CompareNoCase(*name, L"game") == 0) files.RemoveAt(i - 1);
            }
        }
    }
    void ListSubDirectories(SObjectArray<SString>& directories, const wchar_t* path) override {
        const auto resolved = Resolve(path);
        resolved.opener->ListSubDirectories(directories, resolved.path);
        State gameState{};
        if (resolved.opener == &storage_ && !*resolved.path &&
            game_.QueryState(L"", gameState) == errSuccess && (gameState.bitAttributes & attrDirectory)) {
            for (size_t i = 0; i < directories.GetLength(); ++i) {
                const auto* name = directories.GetAt(i);
                if (name && SString::CompareNoCase(*name, L"game") == 0) return;
            }
            directories.Add(new SString(L"game"));
        }
    }
    SError RemoveSubFile(const wchar_t* path) override {
        const auto resolved = Resolve(path);
        return resolved.opener->RemoveSubFile(resolved.path);
    }
    SError CreateSubDirectory(const wchar_t* path, long flags) override {
        const auto resolved = Resolve(path);
        return resolved.opener->CreateSubDirectory(resolved.path, flags);
    }
    SError RemoveSubDirectory(const wchar_t* path) override {
        const auto resolved = Resolve(path);
        return resolved.opener->RemoveSubDirectory(resolved.path);
    }
    SError RenameSubFile(const wchar_t* oldPath, const wchar_t* newPath) override {
        const auto oldResolved = Resolve(oldPath);
        const auto newResolved = Resolve(newPath);
        if (oldResolved.opener == newResolved.opener)
            return oldResolved.opener->RenameSubFile(oldResolved.path, newResolved.path);
        SString oldDirect, newDirect;
        auto error = oldResolved.opener->DirectPathOf(oldDirect, oldResolved.path);
        if (error != errSuccess) return error;
        error = newResolved.opener->DirectPathOf(newDirect, newResolved.path);
        if (error == errSuccess && (entis::io::IsVirtual(UTF8(oldDirect)) || entis::io::IsVirtual(UTF8(newDirect))))
            return errNotSupported;
        return error == errSuccess ? SFile::RenameFile(oldDirect, newDirect) : error;
    }
    SError DirectPathOf(SString& direct, const wchar_t* path) override {
        const auto resolved = Resolve(path);
        return resolved.opener->DirectPathOf(direct, resolved.path);
    }
private:
    struct Resolved { SOffsetFileOpener* opener; const wchar_t* path; };
    Resolved Resolve(const wchar_t* path) {
        if (!path) path = L"";
        while (*path == L'/' || *path == L'\\') ++path;
        if (SString::CompareLeftNoCase(path, L"game") == 0 &&
            (path[4] == 0 || path[4] == L'/' || path[4] == L'\\')) {
            path += 4;
            while (*path == L'/' || *path == L'\\') ++path;
            return {&game_, path};
        }
        return {&storage_, path};
    }
    SOffsetFileOpener storage_;
    SOffsetFileOpener game_;
    bool hasOverride_;
};

void RegisterRoot(const wchar_t* scheme, const std::string& root) {
    if (root.empty()) return;
    g_defURLOpener.RegisterScheme(scheme,
        new SOffsetFileOpener(Wide(root), L'/', new SStandardFileOpener, true));
}

void SDLCALL StdoutLog(void*, int, SDL_LogPriority, const char* text) {
    static std::mutex mutex;
    const std::lock_guard<std::mutex> lock(mutex);
    std::fprintf(stdout, "%s\n", text);
    std::fflush(stdout);
}
} // namespace

bool ConfigureSystemPaths(const SystemPaths& paths) {
    const std::lock_guard<std::mutex> lock(configurationMutex);
    if (inUse) return SDL_SetError("System paths cannot change while EntisGLS is initialized");
    if (!IsAbsolute(paths.storageRoot) || !IsAbsolute(paths.localRoot) ||
        (!paths.assetsRoot.empty() && !IsAbsolute(paths.assetsRoot)) ||
        (!paths.gameRoot.empty() && !IsAbsolute(paths.gameRoot)) ||
        (!paths.sharedRoot.empty() && !IsAbsolute(paths.sharedRoot)) ||
        (!paths.dataRoot.empty() && !IsAbsolute(paths.dataRoot)))
        return SDL_SetError("Explicit absolute storage/local roots and absolute optional roots are required");
    systemPaths = paths;
    configured = true;
    return true;
}

void RequireSystemPaths() {
    const std::lock_guard<std::mutex> lock(configurationMutex);
    if (!configured) throw std::runtime_error("ConfigureSystemPaths must precede EntisGLS initialization");
    inUse = true;
}

void RegisterSystemSchemes() {
    SFileOpener::SetDefaultOpener(&g_defURLOpener);
    g_defURLOpener.RegisterScheme(L"file://", NewGameFileOpener());
    g_defURLOpener.RegisterScheme(L"http://",
        new SOffsetFileOpener(L"http://", L'/', new SHttpFileOpener, true), SVirtualURLOpener::schemeOverNetwork);
    g_defURLOpener.RegisterScheme(L"https://",
        new SOffsetFileOpener(L"https://", L'/', new SHttpFileOpener, true), SVirtualURLOpener::schemeOverNetwork);
    auto* assets = new AssetOpener;
    g_defURLOpener.RegisterScheme(L"assets://", assets);
    g_defURLOpener.RegisterScheme(L"fragments://", new SFragmentFileOpener(L"", L'/', assets, false));
    g_defURLOpener.RegisterScheme(L"storage://", new StorageOpener(systemPaths.storageRoot, systemPaths.gameRoot));
    RegisterRoot(L"local://", systemPaths.localRoot);
    RegisterRoot(L"sd://", systemPaths.sharedRoot);
    RegisterRoot(L"data://", systemPaths.dataRoot);
}

void ReleaseSystemPaths() {
    const std::lock_guard<std::mutex> lock(configurationMutex);
    inUse = false;
}

void InstallStdoutLogging() {
    // SDL already routes Android logs to logcat. Replacing that sink with
    // printf would lose diagnostics because Android normally discards stdout.
    if (SDL_strcmp(SDL_GetPlatform(), "Android") != 0)
        SDL_SetLogOutputFunction(StdoutLog, nullptr);
}
void SetDialogParent(SDL_Window* window) { dialogParent.store(window); }
void SetSymbolResolver(SymbolResolver resolver, void* userdata) {
    const std::lock_guard<std::mutex> lock(configurationMutex);
    symbolResolver = resolver;
    symbolResolverData = userdata;
}

std::uintptr_t ResolveSymbol(const wchar_t* name, const wchar_t* reserved) {
    SymbolResolver resolver;
    void* userdata;
    {
        const std::lock_guard<std::mutex> lock(configurationMutex);
        resolver = symbolResolver;
        userdata = symbolResolverData;
    }
    if (!name || !resolver || (reserved && *reserved)) {
        SDL_SetError("EntisGLS symbol lookup requires a configured resolver and no reserved argument");
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return 0;
    }
    return reinterpret_cast<std::uintptr_t>(resolver(UTF8(name).c_str(), userdata));
}

int ResolveDefaultDirectory(SString& directory, const wchar_t* placement, const wchar_t* option) {
    directory.FreeArray();
    if (!placement) return errInvalidParam;
    if (option && *option) return errNotSupported;
    const auto is = [&](const wchar_t* name) { return SString::Compare(placement, name) == 0; };
    {
        const std::lock_guard<std::mutex> lock(configurationMutex);
        if (!configured) return errFailed;
        if (is(SFile::DefaultDirectory::CurrentDirectory))
            directory = entis::io::IsVirtual(systemPaths.gameRoot) ? SString(L"storage://game") :
                Wide(systemPaths.gameRoot.empty() ? Join(systemPaths.storageRoot, "game") : systemPaths.gameRoot);
        else if (is(SFile::DefaultDirectory::ApplicationData) || is(SFile::DefaultDirectory::AndroidLocalFiles))
            directory = Wide(systemPaths.localRoot);
        else if (is(SFile::DefaultDirectory::AndroidExternalStoragePrivate))
            directory = Wide(systemPaths.storageRoot);
        else if (is(SFile::DefaultDirectory::AndroidExternalStorage)) {
            if (systemPaths.sharedRoot.empty()) return errNotSupported;
            directory = Wide(systemPaths.sharedRoot);
        } else if (is(SFile::DefaultDirectory::ApplicationInstalled))
            directory = systemPaths.assetsRoot.empty() ? SString(L"assets://") : Wide(systemPaths.assetsRoot);
        if (!directory.IsEmpty()) return errSuccess;
    }
    SDL_Folder folder;
    if (is(SFile::DefaultDirectory::UserDocuments)) folder = SDL_FOLDER_DOCUMENTS;
    else if (is(SFile::DefaultDirectory::UserMusic)) folder = SDL_FOLDER_MUSIC;
    else if (is(SFile::DefaultDirectory::UserPictures)) folder = SDL_FOLDER_PICTURES;
    else if (is(SFile::DefaultDirectory::UserVideos)) folder = SDL_FOLDER_VIDEOS;
    else if (is(SFile::DefaultDirectory::WindowsDesktop)) folder = SDL_FOLDER_DESKTOP;
    else return errNotSupported;
    const char* path = SDL_GetUserFolder(folder);
    if (!path) return errNotSupported;
    directory = Wide(path);
    return errSuccess;
}

void FillMemoryStatus(MEMORY_STATUS& status) {
    std::memset(&status, 0, sizeof(status));
    MemoryInfo memory{};
    const bool known = QueryMemoryInfo(memory);
    status.nTotalPhys = static_cast<int64_t>(memory.totalPhysical);
    if (memory.availablePhysicalKnown) status.nAvailPhys = static_cast<int64_t>(memory.availablePhysical);
    if (memory.swapKnown) {
        status.nTotalVirtual = static_cast<int64_t>(memory.totalSwap);
        status.nAvailVirtual = static_cast<int64_t>(memory.availableSwap);
    }
    static std::atomic<bool> warned{false};
    if ((!known || !memory.availablePhysicalKnown || !memory.swapKnown) && !warned.exchange(true))
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Some memory counters are unavailable; their legacy fields are zero (unknown)");
}

void FillPlatformInformation(PLATFORM_INFORMATION& information) {
    std::memset(&information, 0, sizeof(information));
    information.platformFamily = platformFamilyPosix;
    information.runtimeArchitecture = sizeof(void*) * 8;
    const char* platform = SDL_GetPlatform();
    if (SDL_strcmp(platform, "Android") == 0) {
        information.runtimeOS = platformOS_LinuxAndroid;
#if defined(SDL_PLATFORM_ANDROID)
        const int version = SDL_GetAndroidSDKVersion();
        if (version > 0) information.versionOS = static_cast<uint32_t>(version);
#endif
    }
    else if (SDL_strcmp(platform, "Windows") == 0) {
        information.platformFamily = platformFamilyWin32;
        information.runtimeOS = platformOS_WindowsNT;
    } else if (SDL_strcmp(platform, "macOS") == 0) information.runtimeOS = platformFamilyPosix | 0x20;
    else if (SDL_strcmp(platform, "iOS") == 0) information.runtimeOS = platformFamilyPosix | 0x21;
    else if (SDL_strcmp(platform, "Linux") == 0) information.runtimeOS = platformFamilyPosix | 0x30;
    // The original enum has no Apple/Linux identifiers. Reserve distinct POSIX
    // extension IDs instead of claiming Android. versionOS=0 means unknown
    // where SDL has no OS-version query.
}

void FillLocalDate(DATE_TIME& date) {
    std::memset(&date, 0, sizeof(date));
    SDL_Time now;
    SDL_DateTime local{};
    if (!SDL_GetCurrentTime(&now) || !SDL_TimeToDateTime(now, &local, true)) return;
    date.nYear = static_cast<int16_t>(local.year);
    date.nMonth = static_cast<uint16_t>(local.month);
    date.nDay = static_cast<uint16_t>(local.day);
    date.nWeek = static_cast<uint16_t>(local.day_of_week);
    date.nHour = static_cast<uint16_t>(local.hour);
    date.nMinute = static_cast<uint16_t>(local.minute);
    date.nSecond = static_cast<uint16_t>(local.second);
    date.nMilliSec = static_cast<uint16_t>(local.nanosecond / 1000000);
}

std::int32_t LocalTimeDifference(wchar_t* name, std::size_t capacity) {
    if (name && capacity) name[0] = 0;
    SDL_Time now;
    SDL_DateTime local{};
    if (!SDL_GetCurrentTime(&now) || !SDL_TimeToDateTime(now, &local, true)) return 0;
    if (name && capacity) {
        const int minutes = SDL_abs(local.utc_offset / 60);
        char label[24];
        SDL_snprintf(label, sizeof(label), "UTC%c%02d:%02d", local.utc_offset < 0 ? '-' : '+', minutes / 60, minutes % 60);
        std::size_t i = 0;
        for (; i + 1 < capacity && label[i]; ++i) name[i] = static_cast<unsigned char>(label[i]);
        name[i] = 0;
    }
    return -local.utc_offset;
}

int CPUFamily() {
#if defined(__aarch64__) || defined(_M_ARM64)
    return cpuFamily_ARM64;
#elif defined(__x86_64__) || defined(_M_X64)
    return cpuFamily_X86_64;
#elif defined(__arm__) || defined(_M_ARM)
    return cpuFamily_ARM;
#elif defined(__i386__) || defined(_M_IX86)
    return cpuFamily_X86;
#else
    return cpuFamily_Unknown;
#endif
}

std::uint64_t CPUFeatures() {
    uint64_t features = 0;
    const int family = CPUFamily();
    if (family == cpuFamily_ARM64) features |= cpuARM_Feature_ARMv7 | cpuARM_Feature_VFPv3;
    if (family == cpuFamily_ARM || family == cpuFamily_ARM64) {
        if (SDL_HasNEON()) features |= cpuARM_Feature_NEON;
    } else if (family == cpuFamily_X86 || family == cpuFamily_X86_64) {
        if (SDL_HasMMX()) features |= cpuX86_Feature_MMX;
        if (SDL_HasSSE()) features |= cpuX86_Feature_SSE;
        if (SDL_HasSSE2()) features |= cpuX86_Feature_SSE2;
        if (SDL_HasSSE3()) features |= cpuX86_Feature_SSE3;
    }
    return features;
}

int ShowMessageBox(const wchar_t* message, const wchar_t* caption, int style) {
    std::vector<SDL_MessageBoxButtonData> buttons;
    const auto add = [&](int id, const char* label, SDL_MessageBoxButtonFlags flags = 0) {
        buttons.push_back({flags, id, label});
    };
    switch (style) {
    case msgboxStyleOk: add(msgboxResultOk, "OK", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); break;
    case msgboxStyleOkCancel: add(msgboxResultOk, "OK", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT); add(msgboxResultCancel, "Cancel", SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); break;
    case msgboxStyleYesNo: add(msgboxResultYes, "Yes", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT); add(msgboxResultNo, "No", SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); break;
    case msgboxStyleYesNoCancel: add(msgboxResultYes, "Yes", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT); add(msgboxResultNo, "No"); add(msgboxResultCancel, "Cancel", SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); break;
    case msgboxStyleRetryCancel: add(msgboxResultRetry, "Retry", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT); add(msgboxResultCancel, "Cancel", SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); break;
    case msgboxStyleAbortRetryIgnore: add(msgboxResultAbort, "Abort", SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT); add(msgboxResultRetry, "Retry", SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT); add(msgboxResultIgnore, "Ignore"); break;
    default: return errNotSupported;
    }
    const std::string title = caption ? UTF8(caption) : "EntisGLS Launcher";
    const std::string text = UTF8(message);
    struct Request { SDL_MessageBoxData data; int result = -1; bool success = false; std::string error; } request{};
    request.data = {SDL_MESSAGEBOX_INFORMATION, dialogParent.load(), title.c_str(), text.c_str(), static_cast<int>(buttons.size()), buttons.data(), nullptr};
    const auto callback = [](void* userdata) {
        auto& req = *static_cast<Request*>(userdata);
        req.success = SDL_ShowMessageBox(&req.data, &req.result);
        if (!req.success) req.error = SDL_GetError();
    };
    const auto countLocked = SSystem::UnlockAll();
    const bool dispatched = SDL_RunOnMainThread(callback, &request, true);
    SSystem::Relock(countLocked);
    if (!dispatched || !request.success) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Message box failed: %s", request.error.empty() ? SDL_GetError() : request.error.c_str());
        // errFailed numerically equals msgboxResultCancel in the SDK. A
        // negative result avoids reporting an invented user button choice.
        return -1;
    }
    return request.result < 0 ? msgboxResultCancel : request.result;
}
} // namespace study::platform::sdl
