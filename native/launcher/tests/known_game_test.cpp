// Intentionally never initializes SDL or the SDK: discovery precedes both.
#include "launcher/known_game.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "Usage: known_game_test <known-game-directory> <fixture-directory>\n");
        return 2;
    }
    namespace fs = std::filesystem;
    using study::launcher::IsKnownStudySteady;
    try {
        const fs::path game = fs::absolute(argv[1]);
        const fs::path fixture = fs::absolute(argv[2]);
        fs::create_directories(fixture);
        if (!IsKnownStudySteady(game)) throw std::runtime_error("full original fingerprint was not recognized");
        if (IsKnownStudySteady(fixture)) throw std::runtime_error("missing archive falsely recognized");
        fs::copy_file(game / "script.noa", fixture / "script.noa", fs::copy_options::overwrite_existing);
        if (!IsKnownStudySteady(fixture)) throw std::runtime_error("renaming directory changed content detection");
        {
            std::fstream file(fixture / "script.noa", std::ios::binary | std::ios::in | std::ios::out);
            file.seekg(1000000);
            char byte = 0;
            file.get(byte);
            file.seekp(1000000);
            file.put(byte ^ 1);
        }
        if (IsKnownStudySteady(fixture)) throw std::runtime_error("one-byte mutation outside header falsely recognized");
        fs::resize_file(fixture / "script.noa", 2770501);
        if (IsKnownStudySteady(fixture)) throw std::runtime_error("truncated archive falsely recognized");
        // Remove only the copy created by this test so it is safely repeatable.
        fs::remove(fixture / "script.noa");
        std::puts("Known-game fingerprint PASS: full bytes, renamed directory, missing/mutated/truncated archive, no SDK initialization");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Known-game fingerprint FAIL: %s\n", error.what());
        return 1;
    }
}
