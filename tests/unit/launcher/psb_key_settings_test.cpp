#include "launcher/psb_key_settings.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;
using namespace entis::launcher;
namespace {
unsigned checks = 0;
void Require(bool condition, const char* label) {
    if (!condition) throw std::runtime_error(label);
    ++checks;
}
void Reject(const std::function<void()>& action) {
    try { action(); } catch (const PsbKeySettingsError&) { ++checks; return; }
    throw std::runtime_error("Expected invalid setting to be rejected");
}
}
int main(int argc, char** argv) {
    const auto parent = argc > 1 ? fs::u8path(argv[1]) : fs::temp_directory_path();
    const auto root = parent / ("psb-settings-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        Require(!ParsePsbKey(" \t\r\n"), "empty means automatic");
        Require(ParsePsbKey("0") == 0, "zero is explicit");
        Require(ParsePsbKey("4294967295") == UINT32_MAX, "maximum decimal");
        Require(ParsePsbKey(" 0XFFFFFFFF\n") == UINT32_MAX, "maximum hex and outer whitespace");
        Require(ParsePsbKey("000012") == 12, "leading zero remains decimal");
        for (const auto* bad : {"-1", "+1", "1.0", "1e2", "0x", "0x100000000", "4294967296", "0x1g", "1 2", "auto", "１"})
            Reject([&] { ParsePsbKey(bad); });
        Reject([&] { ParsePsbKey(std::string(65, '0')); });
        const auto first = root / "games" / "one", second = root / "games" / "two";
        Require(!ReadPsbKeyOverride(first), "missing directory has no override");
        WritePsbKeyOverride(first, 0);
        Require(ReadPsbKeyOverride(first) == 0, "zero round trip");
        Require(!ReadPsbKeyOverride(second), "another game's key is isolated");
        WritePsbKeyOverride(first, UINT32_MAX);
        Require(ReadPsbKeyOverride(first) == UINT32_MAX, "replacing setting");
        WritePsbKeyOverride(second, 73);
        WritePsbKeyOverride(first, std::nullopt);
        Require(!ReadPsbKeyOverride(first) && !fs::exists(first / "psb-key.txt"), "automatic removes setting file");
        Require(ReadPsbKeyOverride(second) == 73, "clearing first preserves second");
        { std::ofstream corrupt(first / "psb-key.txt"); corrupt << "1garbage"; }
        Reject([&] { ReadPsbKeyOverride(first); });
        { std::ofstream large(first / "psb-key.txt"); large << std::string(100, '0'); }
        Reject([&] { ReadPsbKeyOverride(first); });
        WritePsbKeyOverride(first, std::nullopt);
        fs::create_directory(first / "psb-key.txt");
        { std::ofstream marker(first / "psb-key.txt" / "marker"); marker << "preserve"; }
        Reject([&] { WritePsbKeyOverride(first, 42); });
        Require(fs::exists(first / "psb-key.txt" / "marker"), "failed replacement retains original data");
        Require(std::distance(fs::directory_iterator(first), fs::directory_iterator()) == 1, "failed replacement cleans temporary file");
        fs::remove_all(root);
        std::cout << "PSB settings PASS: " << checks << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PSB settings FAIL: " << error.what() << " (" << root << ")\n";
        return 1;
    }
}
