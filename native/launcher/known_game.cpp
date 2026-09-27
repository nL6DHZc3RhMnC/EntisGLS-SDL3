#include "known_game.h"
#include <sakura/sakura.h>
#include <sakuracl/erisa/sgl_erisa_md5_context.h>
#include <algorithm>
#include <array>
#include <fstream>

namespace study::launcher {

bool IsKnownStudySteady(const std::filesystem::path& gameDir) {
    constexpr std::uintmax_t archiveBytes = 2770502;
    // MD5 741b4068552498c71b7a12fb42e5deaa, stored as four MD5 words.
    constexpr std::array<uint32_t, 4> expected{
        0x68401b74u, 0xc7982455u, 0xfb127a1bu, 0xaadee542u};
    try {
        const auto path = gameDir / "script.noa";
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error) || error ||
            std::filesystem::file_size(path, error) != archiveBytes || error) return false;
        std::ifstream input(path, std::ios::binary);
        if (!input) return false;
        // The existing SDK MD5 byte operations are independent of global SDK
        // state. Avoid its SString formatting helper before initialization.
        SakuraCL::MD5Context hash;
        std::array<uint8_t, 64 * 1024> bytes{};
        std::uintmax_t remaining = archiveBytes;
        while (remaining) {
            const auto count = static_cast<std::streamsize>(
                std::min<std::uintmax_t>(remaining, bytes.size()));
            input.read(reinterpret_cast<char*>(bytes.data()), count);
            if (input.gcount() != count) return false;
            hash.Stream(bytes.data(), static_cast<size_t>(count));
            remaining -= static_cast<std::uintmax_t>(count);
        }
        if (input.peek() != std::char_traits<char>::eof() || input.bad()) return false;
        hash.Flush();
        std::array<uint32_t, 4> digest{};
        hash.GetMD5Digest(digest.data());
        return digest == expected;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    } catch (const std::ios_base::failure&) {
        return false;
    }
}

} // namespace study::launcher
