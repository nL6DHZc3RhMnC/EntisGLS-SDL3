#include "platform/sdl/game_file_opener.h"
#include "platform/sdl/game_file_metrics.h"
#include "io/game_files.h"
#include <SDL3/SDL.h>
#include <array>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <limits>
#include <mutex>
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
std::atomic<bool> fileMetricsEnabled{false};
enum class FileOperation : unsigned { Duplicate, Read, Seek, GetLength, Count };
struct FileCounters {
    std::atomic<std::uint64_t> calls{0}, bytes{0}, totalNs{0}, maxNs{0};
    GameFileOperationMetrics Snapshot() const {
        return {calls.load(std::memory_order_relaxed), bytes.load(std::memory_order_relaxed),
            totalNs.load(std::memory_order_relaxed), maxNs.load(std::memory_order_relaxed)};
    }
};
std::array<FileCounters, static_cast<unsigned>(FileOperation::Count)> fileCounters;
thread_local unsigned fileMetricDepth = 0;

class FileOperationTimer {
    FileCounters* counters_ = nullptr;
    std::uint64_t started_ = 0, bytes_ = 0;
public:
    explicit FileOperationTimer(FileOperation operation) {
        if (!fileMetricDepth && fileMetricsEnabled.load(std::memory_order_relaxed)) {
            counters_ = &fileCounters[static_cast<unsigned>(operation)];
            const int savedError = errno;
            started_ = SDL_GetTicksNS();
            errno = savedError;
            ++fileMetricDepth;
        }
    }
    void ReadBytes(std::uint64_t bytes) { bytes_ = bytes; }
    ~FileOperationTimer() {
        if (!counters_) return;
        --fileMetricDepth;
        const int savedError = errno;
        const auto elapsed = SDL_GetTicksNS() - started_;
        counters_->calls.fetch_add(1, std::memory_order_relaxed);
        counters_->bytes.fetch_add(bytes_, std::memory_order_relaxed);
        counters_->totalNs.fetch_add(elapsed, std::memory_order_relaxed);
        auto maximum = counters_->maxNs.load(std::memory_order_relaxed);
        while (maximum < elapsed && !counters_->maxNs.compare_exchange_weak(
                   maximum, elapsed, std::memory_order_relaxed, std::memory_order_relaxed)) {}
        errno = savedError;
    }
};

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

// SDK streams do not expose a portable native descriptor. Sharing the stream and
// serializing each seek+read works for both SDK files and scoped document providers.
// Only Duplicate shares this owner: a new open must still see the current file.
struct SharedReadOwner {
    explicit SharedReadOwner(std::unique_ptr<SFileInterface> source) : file(std::move(source)) {}
    std::unique_ptr<SFileInterface> file;
    mutable std::mutex mutex;
};

class SharedReadFile final : public SFileInterface {
public:
    SharedReadFile(std::shared_ptr<SharedReadOwner> owner, int64_t position)
        : owner_(std::move(owner)), position_(position) {}
    // SFileInterface is also an opener. In particular, ERI reference images use
    // NewOpenFile(relative) on their input stream. Preserve the source's path
    // resolution and permissions instead of falling back to the global opener.
    SFileInterface* NewOpenFile(const wchar_t* path, long flags) override {
        SFileInterface* opened;
        {
            std::lock_guard<std::mutex> sourceLock(owner_->mutex);
            opened = owner_->file->NewOpenFile(path, flags);
        }
        return (flags & modeReadFlag) && !(flags & modeWriteFlag) ? ShareReadOnlyFile(opened) : opened;
    }
    bool IsExisting(const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->IsExisting(path);
    }
    SError QueryState(const wchar_t* path, State& state) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->QueryState(path, state);
    }
    void ListSubFiles(SObjectArray<SString>& files, const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        owner_->file->ListSubFiles(files, path);
    }
    void ListSubDirectories(SObjectArray<SString>& directories, const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        owner_->file->ListSubDirectories(directories, path);
    }
    SError RemoveSubFile(const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->RemoveSubFile(path);
    }
    SError CreateSubDirectory(const wchar_t* path, long flags) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->CreateSubDirectory(path, flags);
    }
    SError RemoveSubDirectory(const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->RemoveSubDirectory(path);
    }
    SError RenameSubFile(const wchar_t* from, const wchar_t* to) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->RenameSubFile(from, to);
    }
    SError DirectPathOf(SString& direct, const wchar_t* path) override {
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->DirectPathOf(direct, path);
    }
    SFileInterface* Duplicate() const override {
        FileOperationTimer timer(FileOperation::Duplicate);
        std::lock_guard<std::mutex> cursorLock(cursorMutex_);
        return new SharedReadFile(owner_, position_);
    }
    size_t Read(void* data, size_t bytes) override {
        FileOperationTimer timer(FileOperation::Read);
        std::lock_guard<std::mutex> cursorLock(cursorMutex_);
        // Clamp before adding a size_t read count to the signed 64-bit cursor.
        const auto remaining = static_cast<std::uint64_t>(std::numeric_limits<int64_t>::max() - position_);
        if (bytes > remaining) bytes = static_cast<size_t>(remaining);
        if (!bytes) return 0;
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        if (owner_->file->Seek(position_, FromBegin) != position_) return 0;
        const auto read = owner_->file->Read(data, bytes);
        // Some older SDK streams convert read(-1) to size_t. Never let that
        // advance the cursor or escape as a successful oversized read.
        if (read > bytes) {
            Report(std::runtime_error("Game stream returned an invalid read count"));
            return 0;
        }
        position_ += static_cast<int64_t>(read);
        timer.ReadBytes(read);
        return read;
    }
    size_t Write(const void*, size_t) override { return 0; }
    bool IsSeekable() const override { return true; }
    int64_t GetLength() const override {
        FileOperationTimer timer(FileOperation::GetLength);
        std::lock_guard<std::mutex> sourceLock(owner_->mutex);
        return owner_->file->GetLength();
    }
    int64_t Seek(int64_t offset, SeekOrigin origin) override {
        FileOperationTimer timer(FileOperation::Seek);
        std::lock_guard<std::mutex> cursorLock(cursorMutex_);
        int64_t base;
        switch (origin) {
        case FromBegin: base = 0; break;
        case FromCurrent: base = position_; break;
        case FromEnd: {
            std::lock_guard<std::mutex> sourceLock(owner_->mutex);
            base = owner_->file->GetLength();
            if (base < 0) return -1;
            break;
        }
        default: return -1;
        }
        if ((offset < 0 && offset < -base) ||
            (offset >= 0 && offset > std::numeric_limits<int64_t>::max() - base)) return -1;
        position_ = base + offset;
        return position_;
    }
    int64_t GetPosition() const override {
        std::lock_guard<std::mutex> cursorLock(cursorMutex_);
        return position_;
    }
    SError SetEndOfFile() override { return errNotSupported; }
private:
    const std::shared_ptr<SharedReadOwner> owner_;
    mutable std::mutex cursorMutex_;
    int64_t position_;
};

class DocumentFile final : public SFileInterface {
public:
    DocumentFile(fs::path path, FILE* file, bool writable)
        : path_(std::move(path)), file_(file), writable_(writable) {}
    ~DocumentFile() override {
        if (std::fclose(file_) != 0) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Closing game document failed: %s", path_.string().c_str());
    }
    SFileInterface* Duplicate() const override {
        FileOperationTimer timer(FileOperation::Duplicate);
        try {
            if (writable_ && std::fflush(file_) != 0) return nullptr;
            const auto position = ENTIS_FTELL(file_);
            auto* copy = io::Open(path_, writable_ ? "r+b" : "rb");
            if (!copy) return nullptr;
            if (position < 0 || ENTIS_FSEEK(copy, position, SEEK_SET) != 0) { std::fclose(copy); return nullptr; }
            return new DocumentFile(path_, copy, writable_);
        } catch (const std::exception& error) { Report(error); return nullptr; }
    }
    size_t Read(void* data, size_t bytes) override {
        FileOperationTimer timer(FileOperation::Read);
        const auto read = std::fread(data, 1, bytes, file_);
        timer.ReadBytes(read);
        return read;
    }
    size_t Write(const void* data, size_t bytes) override { return writable_ ? std::fwrite(data, 1, bytes, file_) : 0; }
    bool IsSeekable() const override { return ENTIS_FTELL(file_) >= 0; }
    int64_t GetLength() const override {
        FileOperationTimer timer(FileOperation::GetLength);
        const auto position = ENTIS_FTELL(file_);
        if (position < 0 || ENTIS_FSEEK(file_, 0, SEEK_END) != 0) return -1;
        const auto length = ENTIS_FTELL(file_);
        return ENTIS_FSEEK(file_, position, SEEK_SET) == 0 ? length : -1;
    }
    int64_t Seek(int64_t offset, SeekOrigin origin) override {
        FileOperationTimer timer(FileOperation::Seek);
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
            const bool write = (flags & modeWriteFlag) != 0;
            if (!io::IsVirtual(path)) {
                // Let the SDK interpret native paths and all platform-specific flags.
                auto* file = native_.NewOpenFile(source, flags);
                return !write && (flags & modeReadFlag) ? ShareReadOnlyFile(file) : file;
            }
            if (!(flags & (modeReadFlag | modeWriteFlag))) return nullptr;
            if ((flags & modeCreateDirFlag) && write) io::CreateDirectory(path.parent_path());
            const bool create = write && (flags & modeCreateFlag);
            if (!create && io::Stat(path).kind != io::FileInfo::Kind::File) return nullptr;
            FILE* file = io::Open(path, create ? "w+b" : write ? "r+b" : "rb");
            if (!file) return nullptr;
            std::unique_ptr<FILE, decltype(&std::fclose)> pending(file, &std::fclose);
            auto* stream = new DocumentFile(path, file, write);
            pending.release();
            return write ? stream : ShareReadOnlyFile(stream);
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
SSystem::SFileInterface* ShareReadOnlyFile(SSystem::SFileInterface* stream) {
    std::unique_ptr<SFileInterface> source(stream);
    if (!source || !source->IsSeekable()) return source.release();
    const auto position = source->GetPosition();
    if (position < 0) return source.release();
    auto owner = std::make_shared<SharedReadOwner>(std::move(source));
    return new SharedReadFile(std::move(owner), position);
}
void SetGameFileMetricsEnabled(bool enabled) { fileMetricsEnabled.store(enabled, std::memory_order_relaxed); }
bool GameFileMetricsEnabled() { return fileMetricsEnabled.load(std::memory_order_relaxed); }
GameFileMetricsSnapshot SnapshotGameFileMetrics() {
    return {GameFileMetricsEnabled(),
        fileCounters[static_cast<unsigned>(FileOperation::Duplicate)].Snapshot(),
        fileCounters[static_cast<unsigned>(FileOperation::Read)].Snapshot(),
        fileCounters[static_cast<unsigned>(FileOperation::Seek)].Snapshot(),
        fileCounters[static_cast<unsigned>(FileOperation::GetLength)].Snapshot()};
}
SSystem::SFileOpener* NewGameFileOpener() { return new GameFileOpener; }
}
