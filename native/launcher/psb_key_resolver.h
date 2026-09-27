#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

namespace entis::launcher {

class PsbKeyError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Reads game DLLs as bounded PE data. Never loads or executes their code. The
// configuration override is optional: zero is a valid explicit key, not absence.
// Call on the complete, still-encrypted v4 PSB before its header is modified.
// Instances are used by the game worker, not concurrently by multiple threads.
class PsbKeyResolver {
public:
    PsbKeyResolver(std::filesystem::path gameDir,
                   std::filesystem::path gameDataDir,
                   std::optional<std::uint32_t> overrideKey = {});
    std::uint32_t Resolve(const std::uint8_t* rawPsb, std::size_t size);
    const std::string& LastSource() const noexcept { return source_; }
    const std::string& LastWarning() const noexcept { return warning_; }

private:
    std::filesystem::path gameDir_;
    std::filesystem::path gameDataDir_;
    std::optional<std::uint32_t> overrideKey_;
    std::string source_;
    std::string warning_;
};

} // namespace entis::launcher
