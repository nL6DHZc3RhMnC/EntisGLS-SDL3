#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

// POSIX implementation deliberately independent of ECS/JNI for host filesystem
// tests. Only OpenWithinRoot can construct it; NOA and virtual streams do not fit.
class LegacyAtomicPath {
public:
    static bool IsWithinRoot(const std::string &root, const std::string &path);
    static std::shared_ptr<LegacyAtomicPath> OpenWithinRoot(
        const std::string &root, const std::string &path, unsigned flags);
    ~LegacyAtomicPath();
    size_t Read(void *, size_t, uint64_t offset);
    size_t Write(const void *, size_t, uint64_t offset);
    uint64_t Length() const;
    bool Truncate(uint64_t length);
    void BeginSave();
    bool StagePrefix(const void *bytes,size_t length,uint64_t offset);
    bool Replace(const void *bytes, size_t length, uint64_t prefix);
    bool Close();
    const std::string &Path() const { return path_; }
private:
    LegacyAtomicPath() = default;
    bool PublishOpen();
    bool NewTemporary(std::string &path, int &fd) const;
    mutable std::mutex mutex_;
    std::string path_, temporary_;
    int fd_ = -1;
    unsigned flags_ = 0;
    bool discard_ = false;
};
