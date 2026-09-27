#include "game_files.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace entis::io {
namespace {
namespace fs = std::filesystem;
std::mutex backendMutex;
fs::path backendRoot;
std::shared_ptr<Backend> selectedBackend;

bool Within(const fs::path& root, const fs::path& path) {
    auto r = root.begin(), p = path.begin();
    for (; r != root.end(); ++r, ++p)
        if (p == path.end() || *r != *p) return false;
    return true;
}
struct Route {
    std::shared_ptr<Backend> backend;
    fs::path path;
    std::string relative;
};
Route Resolve(const fs::path& path) {
    const auto absolute = fs::absolute(path);
    auto normalized = absolute.lexically_normal();
    if (normalized != normalized.root_path() && normalized.filename().empty())
        normalized = normalized.parent_path();
    std::lock_guard<std::mutex> lock(backendMutex);
    if (!selectedBackend) return {{}, normalized, {}};
    // Reject an explicit mapped-root traversal even if normalization would turn
    // it into a host path. No provider operation may escape its selected tree.
    if (Within(backendRoot, absolute) && !Within(backendRoot, normalized))
        throw std::runtime_error("Game path escapes the selected directory: " + path.u8string());
    if (!Within(backendRoot, normalized)) return {{}, normalized, {}};
    const auto relative = normalized.lexically_relative(backendRoot).generic_u8string();
    return {selectedBackend, normalized, relative == "." ? "" : relative};
}
[[noreturn]] void IOError(const fs::path& path, const char* action) {
    throw std::runtime_error(std::string(action) + ": " + path.u8string() + ": " + std::strerror(errno));
}
using File = std::unique_ptr<std::FILE, decltype(&std::fclose)>;
}

void SetBackend(const std::string& root, std::shared_ptr<Backend> backend) {
    fs::path normalized;
    if (backend) {
        if (root.empty() || !fs::u8path(root).is_absolute())
            throw std::runtime_error("Game backend requires an absolute virtual root");
        normalized = fs::u8path(root).lexically_normal();
        if (normalized != normalized.root_path() && normalized.filename().empty()) normalized = normalized.parent_path();
        if (normalized == normalized.root_path()) throw std::runtime_error("Game backend cannot replace the filesystem root");
    }
    std::lock_guard<std::mutex> lock(backendMutex);
    backendRoot = std::move(normalized);
    selectedBackend = std::move(backend);
}
bool IsVirtual(const fs::path& path) { return bool(Resolve(path).backend); }
fs::path Canonical(const fs::path& path, bool mustExist) {
    const auto route = Resolve(path);
    if (route.backend) {
        if (mustExist && route.backend->Stat(route.relative).kind == FileInfo::Kind::Missing)
            throw std::runtime_error("Game path does not exist: " + path.u8string());
        return route.path;
    }
    // Resolve the original path, not its lexical normalization: native symlinks
    // followed by '..' must keep their actual filesystem meaning.
    return mustExist ? fs::canonical(path) : fs::weakly_canonical(path);
}
FileInfo Stat(const fs::path& path) {
    const auto route = Resolve(path);
    if (route.backend) return route.backend->Stat(route.relative);
    std::error_code error;
    const auto link = fs::symlink_status(path, error);
    if (error == std::errc::no_such_file_or_directory || error == std::errc::not_a_directory || link.type() == fs::file_type::not_found) return {};
    if (error) throw fs::filesystem_error("Cannot inspect game file", path, error);
    FileInfo result;
    result.symbolicLink = fs::is_symlink(link);
    const auto state = fs::status(path, error);
    if (error == std::errc::no_such_file_or_directory) return result;
    if (error) throw fs::filesystem_error("Cannot inspect game file", path, error);
    if (fs::is_regular_file(state)) { result.kind = FileInfo::Kind::File; result.size = fs::file_size(path); }
    else if (fs::is_directory(state)) result.kind = FileInfo::Kind::Directory;
    result.writable = (state.permissions() & (fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write)) != fs::perms::none;
    const auto modified = fs::last_write_time(path, error);
    if (!error) {
        const auto systemTime = modified - fs::file_time_type::clock::now() + std::chrono::system_clock::now();
        result.modifiedMs = std::chrono::duration_cast<std::chrono::milliseconds>(systemTime.time_since_epoch()).count();
    }
    return result;
}
std::vector<Entry> List(const fs::path& path) {
    const auto route = Resolve(path);
    if (route.backend) {
        auto entries = route.backend->List(route.relative);
        for (const auto& entry : entries)
            if (entry.name.empty() || entry.name == "." || entry.name == ".." || entry.name.find_first_of("/\\\0", 0, 3) != std::string::npos)
                throw std::runtime_error("Invalid filename returned by game provider");
        return entries;
    }
    std::vector<Entry> entries;
    for (const auto& entry : fs::directory_iterator(path)) entries.push_back({entry.path().filename().u8string(), Stat(entry.path())});
    return entries;
}
std::FILE* Open(const fs::path& path, const char* mode) {
    const auto route = Resolve(path);
    std::FILE* file;
    if (route.backend) file = route.backend->Open(route.relative, mode);
    else {
#if defined(_WIN32)
        const std::string textMode(mode);
        const std::wstring wideMode(textMode.begin(), textMode.end());
        file = _wfopen(path.c_str(), wideMode.c_str());
#else
        file = std::fopen(path.c_str(), mode);
#endif
    }
    if (!file) IOError(path, "Cannot open game file");
    return file;
}
void CreateDirectory(const fs::path& path) {
    const auto route = Resolve(path);
    if (route.backend) route.backend->CreateDirectory(route.relative);
    else fs::create_directories(path);
}
void Remove(const fs::path& path, bool directory) {
    const auto route = Resolve(path);
    if (route.backend) { route.backend->Remove(route.relative, directory); return; }
    const auto state = Stat(path);
    if (state.kind == FileInfo::Kind::Missing) return;
    if ((state.kind == FileInfo::Kind::Directory) != directory) throw std::runtime_error("Wrong file type for removal");
    fs::remove(path);
}
void Rename(const fs::path& oldPath, const fs::path& newPath) {
    const auto oldRoute = Resolve(oldPath), newRoute = Resolve(newPath);
    if (oldRoute.backend != newRoute.backend) throw std::runtime_error("Cannot rename between game storage providers");
    if (oldRoute.backend) oldRoute.backend->Rename(oldRoute.relative, newRoute.relative);
    else fs::rename(oldPath, newPath);
}
std::vector<std::uint8_t> ReadPrefix(const fs::path& path, std::size_t count) {
    File file(Open(path, "rb"), &std::fclose);
    std::vector<std::uint8_t> bytes(count);
    if ((count && std::fread(bytes.data(), 1, count, file.get()) != count) || std::ferror(file.get()))
        throw std::runtime_error("Incomplete game file read: " + path.u8string());
    return bytes;
}
std::vector<std::uint8_t> ReadFile(const fs::path& path, std::size_t limit) {
    const auto info = Stat(path);
    if (info.kind != FileInfo::Kind::File || info.size > limit)
        throw std::runtime_error("Game file is unavailable or exceeds the read limit: " + path.u8string());
    File file(Open(path, "rb"), &std::fclose);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(info.size));
    if ((!bytes.empty() && std::fread(bytes.data(), 1, bytes.size(), file.get()) != bytes.size()) ||
        std::fgetc(file.get()) != EOF || std::ferror(file.get()))
        throw std::runtime_error("Game file changed or could not be read completely: " + path.u8string());
    return bytes;
}
} // namespace entis::io
