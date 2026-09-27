#include "launcher/psb_key_settings.h"

#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <limits>
#include <system_error>

namespace entis::launcher {
namespace {
namespace fs = std::filesystem;
constexpr const char* filename = "psb-key.txt";
constexpr const char* invalidKey = "PSB key must be an unsigned 32-bit decimal or 0x hexadecimal integer; leave it empty for automatic discovery";
bool Space(char value) {
    return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}
}

std::optional<std::uint32_t> ParsePsbKey(const std::string& text) {
    if (text.size() > 64) throw PsbKeySettingsError(invalidKey);
    std::size_t begin = 0, end = text.size();
    while (begin < end && Space(text[begin])) ++begin;
    while (end > begin && Space(text[end - 1])) --end;
    if (begin == end) return std::nullopt;
    unsigned base = 10;
    if (end - begin >= 2 && text[begin] == '0' && (text[begin + 1] == 'x' || text[begin + 1] == 'X')) {
        base = 16;
        begin += 2;
    }
    if (begin == end) throw PsbKeySettingsError(invalidKey);
    std::uint32_t value = 0;
    for (; begin < end; ++begin) {
        const auto c = text[begin];
        const unsigned digit = c >= '0' && c <= '9' ? c - '0' :
            c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 16;
        if (digit >= base || value > (std::numeric_limits<std::uint32_t>::max() - digit) / base)
            throw PsbKeySettingsError(invalidKey);
        value = value * base + digit;
    }
    return value;
}

std::optional<std::uint32_t> ReadPsbKeyOverride(const fs::path& directory) {
    const auto path = directory / filename;
    std::error_code error;
    const auto status = fs::symlink_status(path, error);
    if (error == std::errc::no_such_file_or_directory || status.type() == fs::file_type::not_found) return std::nullopt;
    if (error || !fs::is_regular_file(status))
        throw PsbKeySettingsError("Cannot read PSB key setting: " + path.u8string());
    std::ifstream input(path, std::ios::binary);
    if (!input) throw PsbKeySettingsError("Cannot open PSB key setting: " + path.u8string());
    std::array<char, 65> bytes{};
    input.read(bytes.data(), bytes.size());
    const auto count = input.gcount();
    if (input.bad()) throw PsbKeySettingsError("Cannot read PSB key setting: " + path.u8string());
    try { return ParsePsbKey(std::string(bytes.data(), static_cast<std::size_t>(count))); }
    catch (const PsbKeySettingsError&) { throw PsbKeySettingsError("Invalid PSB key setting in " + path.u8string() + ": " + invalidKey); }
}

void WritePsbKeyOverride(const fs::path& directory, std::optional<std::uint32_t> key) {
    const auto path = directory / filename;
    std::error_code error;
    if (!key) {
        fs::remove(path, error);
        if (error) throw PsbKeySettingsError("Cannot clear PSB key setting: " + path.u8string() + ": " + error.message());
        return;
    }
    fs::create_directories(directory, error);
    if (error) throw PsbKeySettingsError("Cannot create PSB settings directory: " + directory.u8string() + ": " + error.message());
    // Write completely before replacing the last valid setting. Failed saves do
    // not discard the previous override; resource files are never touched.
    static std::atomic<unsigned long> sequence{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto temporary = directory / (std::string(filename) + ".tmp-" + std::to_string(stamp) + "-" + std::to_string(sequence++));
    try {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) throw PsbKeySettingsError("Cannot create temporary PSB key setting");
        output << *key << '\n';
        output.close();
        if (!output) throw PsbKeySettingsError("Cannot complete PSB key setting write");
        fs::rename(temporary, path, error);
        if (error) throw PsbKeySettingsError("Cannot replace PSB key setting: " + error.message());
    } catch (...) {
        fs::remove(temporary, error);
        throw;
    }
}
} // namespace entis::launcher
