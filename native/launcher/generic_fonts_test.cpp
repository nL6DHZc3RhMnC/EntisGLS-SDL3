// Real font regression without any game/config/archive inputs. Pass a regular
// Unicode TrueType font and optionally the existing CJK compatibility font.
#include "launcher/compatibility_profiles.h"
#include "platform/sdl/game_font_aliases.h"
#include "platform/sdl/opentype_font.h"
#include "platform/sdl/system.h"
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_font.h>
#include <SDL3/SDL.h>
#include <filesystem>
#include <cstdio>
#include <stdexcept>

namespace {
void Require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
std::wstring Wide(const char* text) {
    SSystem::SString value;
    value.FromUTF8(reinterpret_cast<const uint8_t*>(text));
    return static_cast<const wchar_t*>(value);
}
std::vector<uint8_t> Glyph(const std::wstring& name) {
    SakuraGL::SGLFontStyle style;
    style.pszFace = name.c_str(); style.nSize = 40;
    SakuraGL::SGLFont font;
    Require(font.SetStyle(style) == SakuraGL::sglErrSuccess, "named face selection failed");
    SakuraGL::SGLFontMetrics metrics{};
    Require(font.GetMetrics(nullptr, 0, metrics, L'M') == SakuraGL::sglErrSuccess,
            "named face glyph metrics failed");
    std::vector<uint8_t> pixels(size_t(metrics.rctExterior.w) * metrics.rctExterior.h);
    Require(font.GetMetrics(pixels.data(), pixels.size(), metrics, L'M') == SakuraGL::sglErrSuccess,
            "named face glyph raster failed");
    return pixels;
}
}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 5) {
        std::fprintf(stderr, "Usage: generic_fonts_test <fixture-root> <regular.ttf> [cjk.otf] [same-family-bold.ttf]\n");
        return 2;
    }
    using namespace study::platform::sdl;
    using namespace study::launcher;
    if (!SDL_Init(0)) return 2;
    const auto root = std::filesystem::absolute(argv[1]);
    std::filesystem::create_directories(root / "assets");
    std::filesystem::create_directories(root / "storage");
    std::filesystem::create_directories(root / "local");
    std::filesystem::create_directories(root / "game");
    SystemPaths paths;
    paths.assetsRoot = (root / "assets").string();
    paths.storageRoot = (root / "storage").string();
    paths.localRoot = (root / "local").string();
    paths.gameRoot = (root / "game").string();
    bool initialized = false;
    int result = 0;
    try {
        Require(ConfigureSystemPaths(paths), "isolated empty game roots rejected");
        SakuraGL::Initialize();
        initialized = true;
        Require(ConfigureCompatibility("") &&
                ActiveCompatibilityProfile().empty(), "generic profile inherited another game's settings");
        Require(ApplyCompatibilityFonts("") && CheckCompatibilityFonts(""), "generic game requires a known game's fonts");
        Require(!HasStockFont(L"Default") && !HasStockFont(L"MsgFont") &&
                !HasStockFont(L"Noto Serif CJK TC"), "generic startup loaded game-specific fonts");
        Require(ConfigureCompatibility("study-steady-r18") && ActiveCompatibilityProfile() == "study-steady-r18",
                "explicit compatibility profile not selected");
        Require(!ConfigureCompatibility("unknown") && ActiveCompatibilityProfile() == "study-steady-r18" &&
                !ApplyCompatibilityFonts("unknown"), "unknown profile silently accepted");
        Require(ConfigureCompatibility(""), "reset generic profile failed");
        Require(!RegisterOpenTypeFont(L"storage://game/missing.ttf").loaded,
                "missing font incorrectly registered");
        Require(!RegisterOpenTypeFont(Wide(argv[2]), L"Wrong family").loaded,
                "compatibility family mismatch accepted");
        const auto regular = RegisterOpenTypeFont(Wide(argv[2]));
        Require(regular.loaded && !regular.family.empty() && HasStockFont(regular.family),
                "real regular font family not discovered/registered");
        Require(RegisterFontAlias(std::wstring(L"Transient alias"), std::wstring(regular.family)),
                "arbitrary font alias failed");
        Require(CheckFontAlias(L"Transient alias", regular.family, {L'A', L'B', L'N'}),
                "font alias pixels/metrics/lifetime regression");
        Require(RegisterFontAlias(L"Default", regular.family) && RegisterFontAlias(L"Default", L"Unavailable"),
                "existing explicit Default was replaced by an unavailable fallback");
        Require(CheckFontAlias(L"Default", regular.family, {L'A', L'B', L'N'}),
                "explicit Default did not retain its actual source");
        Require(!RegisterFontAlias(L"Missing alias", L"Unavailable") && !HasStockFont(L"Missing alias"),
                "missing alias source created a fake font");
        Require(CheckOpenTypeFont(regular.family, {L'M', L'A', L'B', L'9'}),
                "regular TrueType style/size/rasterization regression");
        if (argc >= 4) {
            bool requested = false;
            const auto cjk = RegisterOpenTypeFont(L"archive-font.otf", L"Noto Serif CJK TC",
                [&](const wchar_t* name) -> SSystem::SFileInterface* {
                    requested = std::wstring(name) == L"archive-font.otf";
                    return SSystem::SFileOpener::DefaultNewOpenFile(Wide(argv[3]).c_str(),
                                                                   SSystem::SFileOpener::shareRead);
                });
            Require(requested && cjk.loaded, "explicit environment opener not used");
            Require(CheckOpenTypeFont(cjk.family,
                    {L'名', L'姓', L'樹', L'和', L'說', L'腳', L'夠', L'，', L'。', L'「', L'」'}, {L' ', L'　'}),
                    "original Traditional Chinese compatibility font regression");
            Require(CheckFontAlias(L"Default", regular.family, {L'A', L'B', L'N'}),
                    "registering a second family changed the explicit Default");
        }
        if (argc == 5) {
            const auto regularPixels = Glyph(regular.registeredName);
            const auto bold = RegisterOpenTypeFont(Wide(argv[4]), regular.family, {}, L"Heading");
            Require(bold.loaded && bold.family == regular.family && bold.registeredName == L"Heading",
                    "same-family named bold face was not independently registered");
            Require(Glyph(bold.registeredName) != regularPixels && Glyph(regular.registeredName) == regularPixels,
                    "named bold face reused or changed the regular family's glyphs");
            Require(CheckOpenTypeFont(bold.registeredName, {L'M', L'A', L'B', L'9'}),
                    "named bold face style/size/lifetime regression");
            Require(RegisterOpenTypeFont(Wide(argv[4]), regular.family, {}, L"Heading").loaded,
                    "re-registering the identical named face failed");
            Require(!RegisterOpenTypeFont(Wide(argv[2]), {}, {}, L"Heading").loaded &&
                    Glyph(regular.registeredName) == regularPixels,
                    "a conflicting named face silently replaced an existing registration");
        }
        std::puts("Generic font/profile test PASS: no game assets, real Unicode families, explicit aliases, independent faces/styles, profile isolation, optional CJK regression");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Generic font/profile test FAIL: %s\n", error.what());
        result = 1;
    }
    if (initialized) SakuraGL::Finalize();
    SDL_Quit();
    return result;
}
