#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

// Independent of ECS/JNI. Native paths use POSIX atomic rename; selected document
// trees use the game backend's recoverable replacement and seekable descriptors.
class LegacyAtomicPath {
public:
    static bool IsWithinRoot(const std::string &root, const std::string &path);
    // Once scope is verified, failures must not fall back to a truncating open.
    // This output lets the SDK adapter avoid repeating the same scope queries.
    static std::shared_ptr<LegacyAtomicPath> OpenWithinRoot(
        const std::string &root, const std::string &path, unsigned flags, bool *candidate = nullptr);
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
    bool ReopenTemporary();
    mutable std::mutex mutex_;
    std::string path_, temporary_;
    int fd_ = -1;
    unsigned flags_ = 0;
    bool discard_ = false;
    // Failed in-place body bytes cannot be published by unrelated writes or
    // treated as a trusted prefix on a retry with a different body boundary.
    bool bodyNeedsValidation_ = false;
    uint64_t unvalidatedBodyBegin_ = 0;
    // A failed in-place thumbnail write must be fully restaged before any save
    // can publish this private file. Ordinary save-body retries preserve it.
    bool damagedPrefix_ = false;
    uint64_t damagedPrefixBegin_ = 0, damagedPrefixEnd_ = 0;
};
