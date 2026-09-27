#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// Game resources may live in a user-authorized document provider rather than a
// POSIX directory. This interface is independent of SDL/SDK initialization.
namespace entis::io {
struct FileInfo {
    enum class Kind { Missing, File, Directory };
    Kind kind = Kind::Missing;
    std::uint64_t size = 0;
    std::int64_t modifiedMs = 0;
    bool writable = false;
    bool symbolicLink = false;
};
struct Entry { std::string name; FileInfo info; };

class Backend {
public:
    virtual ~Backend() = default;
    virtual FileInfo Stat(const std::string& relative) = 0;
    virtual std::vector<Entry> List(const std::string& relative) = 0;
    // Returns an owned stream, closed by the caller. Failures throw; resource
    // streams must support seeking because archives require random access.
    virtual std::FILE* Open(const std::string& relative, const char* mode) = 0;
    virtual void CreateDirectory(const std::string& relative) = 0;
    virtual void Remove(const std::string& relative, bool directory) = 0;
    virtual void Rename(const std::string& oldRelative, const std::string& newRelative) = 0;
};

// One selected game per process. A null backend clears the mapping. Configure
// before launch; streams and in-flight operations retain their own ownership.
void SetBackend(const std::string& absoluteVirtualRoot, std::shared_ptr<Backend> backend);
bool IsVirtual(const std::filesystem::path& path);
std::filesystem::path Canonical(const std::filesystem::path& path, bool mustExist = true);
FileInfo Stat(const std::filesystem::path& path);
std::vector<Entry> List(const std::filesystem::path& path);
std::FILE* Open(const std::filesystem::path& path, const char* mode);
void CreateDirectory(const std::filesystem::path& path);
void Remove(const std::filesystem::path& path, bool directory);
void Rename(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);
std::vector<std::uint8_t> ReadFile(const std::filesystem::path& path, std::size_t limit);
// Reads exactly count bytes or throws. The caller can use Stat to bound count.
std::vector<std::uint8_t> ReadPrefix(const std::filesystem::path& path, std::size_t count);
} // namespace entis::io
