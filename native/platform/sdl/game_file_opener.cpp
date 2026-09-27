#include "platform/sdl/game_file_opener.h"
#include "io/game_files.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <limits>
#include <stdexcept>
#if defined(_WIN32)
#include <io.h>
#define ENTIS_FSEEK _fseeki64
#define ENTIS_FTELL _ftelli64
#else
#include <unistd.h>
#define ENTIS_FSEEK fseeko
#define ENTIS_FTELL ftello
#endif

namespace study::platform::sdl {
namespace {
using namespace SSystem;
namespace io = entis::io;
namespace fs = std::filesystem;
std::string Utf8(const wchar_t* path) {
    if (!path) return {};
    auto text = SString(path).ToUTF8();
    if (!text.GetLength()) return {};
    std::string result(reinterpret_cast<const char*>(text.GetConstArray()), text.GetLength());
    if (!result.empty() && result.back() == '\0') result.pop_back();
    for (char& c : result) if (c == '\\') c = '/';
    return result;
}
SString Wide(const std::string& text) {
    SString result; result.FromUTF8(reinterpret_cast<const uint8_t*>(text.c_str())); return result;
}
void Report(const std::exception& error) {
    SDL_SetError("%s", error.what());
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game folder: %s", error.what());
}

class DocumentFile final : public SFileInterface {
public:
    DocumentFile(fs::path path, FILE* file, bool writable)
        : path_(std::move(path)), file_(file), writable_(writable) {}
    ~DocumentFile() override {
        if (std::fclose(file_) != 0) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Closing game document failed: %s", path_.string().c_str());
    }
    SFileInterface* Duplicate() const override {
        try {
            if (writable_ && std::fflush(file_) != 0) return nullptr;
            const auto position = ENTIS_FTELL(file_);
            auto* copy = io::Open(path_, writable_ ? "r+b" : "rb");
            if (!copy) return nullptr;
            if (position < 0 || ENTIS_FSEEK(copy, position, SEEK_SET) != 0) { std::fclose(copy); return nullptr; }
            return new DocumentFile(path_, copy, writable_);
        } catch (const std::exception& error) { Report(error); return nullptr; }
    }
    size_t Read(void* data, size_t bytes) override { return std::fread(data, 1, bytes, file_); }
    size_t Write(const void* data, size_t bytes) override { return writable_ ? std::fwrite(data, 1, bytes, file_) : 0; }
    bool IsSeekable() const override { return ENTIS_FTELL(file_) >= 0; }
    int64_t GetLength() const override {
        const auto position = ENTIS_FTELL(file_);
        if (position < 0 || ENTIS_FSEEK(file_, 0, SEEK_END) != 0) return -1;
        const auto length = ENTIS_FTELL(file_);
        return ENTIS_FSEEK(file_, position, SEEK_SET) == 0 ? length : -1;
    }
    int64_t Seek(int64_t offset, SeekOrigin origin) override {
        if (origin < FromBegin || origin > FromEnd) return -1;
        const int modes[] = {SEEK_SET, SEEK_CUR, SEEK_END};
        return ENTIS_FSEEK(file_, offset, modes[origin]) == 0 ? ENTIS_FTELL(file_) : -1;
    }
    int64_t GetPosition() const override { return ENTIS_FTELL(file_); }
    SError SetEndOfFile() override {
        if (!writable_ || std::fflush(file_) != 0) return errFailed;
        const auto position = ENTIS_FTELL(file_);
        if (position < 0) return errFailed;
#if defined(_WIN32)
        return _chsize_s(_fileno(file_), position) == 0 ? errSuccess : errFailed;
#else
        return ::ftruncate(fileno(file_), position) == 0 ? errSuccess : errFailed;
#endif
    }
private:
    fs::path path_;
    FILE* file_;
    bool writable_;
};

class GameFileOpener final : public SFileOpener {
public:
    SFileInterface* NewOpenFile(const wchar_t* source, long flags) override {
        try {
            const auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) return native_.NewOpenFile(source, flags);
            const bool write = (flags & modeWriteFlag) != 0;
            if (!(flags & (modeReadFlag | modeWriteFlag))) return nullptr;
            if ((flags & modeCreateDirFlag) && write) io::CreateDirectory(path.parent_path());
            const bool create = write && (flags & modeCreateFlag);
            if (!create && io::Stat(path).kind != io::FileInfo::Kind::File) return nullptr;
            FILE* file = io::Open(path, create ? "w+b" : write ? "r+b" : "rb");
            return file ? new DocumentFile(path, file, write) : nullptr;
        } catch (const std::exception& error) { Report(error); return nullptr; }
    }
    bool IsExisting(const wchar_t* source) override {
        try {
            auto path = fs::u8path(Utf8(source));
            return io::IsVirtual(path) ? io::Stat(path).kind != io::FileInfo::Kind::Missing : native_.IsExisting(source);
        } catch (const std::exception& error) { Report(error); return false; }
    }
    SError QueryState(const wchar_t* source, State& state) override {
        try {
            auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) return native_.QueryState(source, state);
            std::memset(&state, 0, sizeof(state));
            const auto info = io::Stat(path);
            if (info.kind == io::FileInfo::Kind::Missing) return errFailed;
            state.bitFields = fieldAttributes | fieldFileSize;
            state.bitAttributes = permissionRUSR | (info.writable ? permissionWUSR : 0) |
                (info.kind == io::FileInfo::Kind::Directory ? attrDirectory : 0);
            state.nFileSize = info.size;
            SDL_DateTime modified{};
            if (info.modifiedMs > 0 && info.modifiedMs <= std::numeric_limits<SDL_Time>::max() / 1000000 &&
                SDL_TimeToDateTime(info.modifiedMs * 1000000, &modified, true)) {
                state.bitFields |= fieldModifiedTime;
                state.dtModified.nYear = static_cast<int16_t>(modified.year);
                state.dtModified.nMonth = static_cast<uint16_t>(modified.month);
                state.dtModified.nDay = static_cast<uint16_t>(modified.day);
                state.dtModified.nWeek = static_cast<uint16_t>(modified.day_of_week);
                state.dtModified.nHour = static_cast<uint16_t>(modified.hour);
                state.dtModified.nMinute = static_cast<uint16_t>(modified.minute);
                state.dtModified.nSecond = static_cast<uint16_t>(modified.second);
                state.dtModified.nMilliSec = static_cast<uint16_t>(modified.nanosecond / 1000000);
            }
            return errSuccess;
        } catch (const std::exception& error) { Report(error); return errFailed; }
    }
    void ListSubFiles(SObjectArray<SString>& results, const wchar_t* path) override { List(results, path, false); }
    void ListSubDirectories(SObjectArray<SString>& results, const wchar_t* path) override { List(results, path, true); }
    SError CreateSubDirectory(const wchar_t* source, long flags) override {
        try {
            auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) return native_.CreateSubDirectory(source, flags);
            io::CreateDirectory(path); return errSuccess;
        } catch (const std::exception& error) { Report(error); return errFailed; }
    }
    SError RemoveSubFile(const wchar_t* source) override { return Remove(source, false); }
    SError RemoveSubDirectory(const wchar_t* source) override { return Remove(source, true); }
    SError RenameSubFile(const wchar_t* oldSource, const wchar_t* newSource) override {
        try {
            auto oldPath = fs::u8path(Utf8(oldSource)), newPath = fs::u8path(Utf8(newSource));
            if (!io::IsVirtual(oldPath) && !io::IsVirtual(newPath)) return native_.RenameSubFile(oldSource, newSource);
            if (!io::IsVirtual(oldPath) || !io::IsVirtual(newPath)) return errNotSupported;
            io::Rename(oldPath, newPath); return errSuccess;
        } catch (const std::exception& error) { Report(error); return errFailed; }
    }
    SError DirectPathOf(SString& direct, const wchar_t* source) override {
        try {
            auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) return native_.DirectPathOf(direct, source);
            // An explicit virtual path remains virtual. The atomic save layer
            // recognizes it and routes through the backend, never POSIX open.
            direct = Wide(io::Canonical(path, false).u8string()); return errSuccess;
        } catch (const std::exception& error) { Report(error); return errFailed; }
    }
private:
    SStandardFileOpener native_;
    void List(SObjectArray<SString>& results, const wchar_t* source, bool directories) {
        try {
            auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) {
                if (directories) native_.ListSubDirectories(results, source); else native_.ListSubFiles(results, source);
                return;
            }
            for (const auto& entry : io::List(path))
                if ((entry.info.kind == io::FileInfo::Kind::Directory) == directories && entry.info.kind != io::FileInfo::Kind::Missing)
                    results.Add(new SString(Wide(entry.name)));
        } catch (const std::exception& error) { Report(error); }
    }
    SError Remove(const wchar_t* source, bool directory) {
        try {
            auto path = fs::u8path(Utf8(source));
            if (!io::IsVirtual(path)) return directory ? native_.RemoveSubDirectory(source) : native_.RemoveSubFile(source);
            io::Remove(path, directory); return errSuccess;
        } catch (const std::exception& error) { Report(error); return errFailed; }
    }
};
}
SSystem::SFileOpener* NewGameFileOpener() { return new GameFileOpener; }
}
