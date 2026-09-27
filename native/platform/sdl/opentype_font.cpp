#include "opentype_font.h"
#include "game_font_aliases.h"
#include "platform/log.h"
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_font.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H
#include <algorithm>
#include <array>
#include <cstring>
#include <cwchar>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace study::platform::sdl {
namespace {
using namespace SakuraGL;
constexpr size_t maxFontBytes = 64u * 1024u * 1024u;
constexpr size_t maxGlyphBytes = 64u * 1024u * 1024u;
using FontBytes = std::vector<uint8_t>;

int Ceil26(FT_Pos value) { return static_cast<int>((value + 63) >> 6); }
int Round26(FT_Pos value) { return static_cast<int>((value + 32) >> 6); }
int FloorHalf(int value) { return value >= 0 ? value / 2 : -((-value + 1) / 2); }
int CeilHalf(int value) { return value >= 0 ? (value + 1) / 2 : -((-value) / 2); }

class OpenTypeFont final : public SGLFontObject {
public:
    OpenTypeFont(std::shared_ptr<const FontBytes> bytes, std::wstring family)
        : bytes_(std::move(bytes)), family_(std::move(family)) {}
    ~OpenTypeFont() override {
        if (face_) FT_Done_Face(face_);
        if (library_) FT_Done_FreeType(library_);
    }

    SGLFontObject* NewFont(const SGLFontStyle& style) override {
        try {
            auto result = std::make_unique<OpenTypeFont>(bytes_, family_);
            if (result->SetStyle(style) != sglErrSuccess) return nullptr;
            return result.release();
        } catch (const std::bad_alloc&) { return nullptr; }
    }

    SGLError SetStyle(const SGLFontStyle& style) override {
        std::lock_guard<std::mutex> lock(mutex_);
        configured_ = false;
        cached_ = false;
        pixels_.clear();
        constexpr uint32_t supported = SGLFontStyle::styleBold | SGLFontStyle::styleItalic |
            SGLFontStyle::styleNoSmooth | SGLFontStyle::styleHighDefinition;
        if (!style.pszFace || family_ != style.pszFace ||
            style.nSize == 0 || style.nSize > 4096) return sglErrInvalidParam;
        if (style.nStyles & ~supported) return sglErrNotSupported;
        if (!face_) {
            if (!bytes_ || bytes_->empty() || bytes_->size() > maxFontBytes) return sglErrFailed;
            if (!library_ && FT_Init_FreeType(&library_)) return sglErrFailed;
            if (FT_New_Memory_Face(library_, bytes_->data(), static_cast<FT_Long>(bytes_->size()), 0, &face_))
                return sglErrFailed;
            if (!FT_IS_SCALABLE(face_) || FT_Select_Charmap(face_, FT_ENCODING_UNICODE)) {
                FT_Done_Face(face_);
                face_ = nullptr;
                return sglErrFailed;
            }
        }
        flags_ = style.nStyles;
        sampleScale_ = (flags_ & SGLFontStyle::styleHighDefinition) &&
            !(flags_ & SGLFontStyle::styleNoSmooth) ? 2 : 1;
        // The SDK's Windows font uses positive LOGFONT.lfHeight, a character
        // cell height including ascent/descent, not an em-square pixel size.
        // REAL_DIM preserves that convention, including the top-of-cell origin
        // used by SGLLetterer: exterior.y = ascent - glyph bitmap_top.
        FT_Size_RequestRec request{};
        request.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
        request.height = static_cast<FT_Long>(style.nSize) * sampleScale_ * 64;
        if (FT_Request_Size(face_, &request)) return sglErrFailed;
        configured_ = true;
        return sglErrSuccess;
    }

    SGLError GetMetrics(uint8_t* rasterized, size_t bytes,
                        SGLFontMetrics& metrics, uint32_t character) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!configured_) return sglErrFailed;
        if (character > 0x10ffff || (character >= 0xd800 && character <= 0xdfff))
            return sglErrInvalidParam;
        try {
            if (!cached_ || character != cachedCharacter_) {
                cached_ = false;
                const auto result = Render(character);
                if (result != sglErrSuccess) return result;
                cachedCharacter_ = character;
                cached_ = true;
            }
            // Never copy a partial glyph or write outside the caller's buffer.
            if (rasterized && bytes < pixels_.size()) return sglErrInvalidParam;
            metrics = metrics_;
            if (rasterized && !pixels_.empty()) std::memcpy(rasterized, pixels_.data(), pixels_.size());
            return sglErrSuccess;
        } catch (const std::bad_alloc&) { return sglErrFailed; }
    }

    const std::shared_ptr<const FontBytes>& Bytes() const { return bytes_; }
    bool IsBoldFace() const { return face_ && (face_->style_flags & FT_STYLE_FLAG_BOLD); }

private:
    SGLError Render(uint32_t character) {
        const auto index = FT_Get_Char_Index(face_, character);
        if (!index) return sglErrFailed; // Do not silently paint .notdef as a real character.
        const bool monochrome = (flags_ & SGLFontStyle::styleNoSmooth) != 0;
        const auto loadFlags = FT_LOAD_NO_BITMAP |
            (monochrome ? FT_LOAD_TARGET_MONO : FT_LOAD_TARGET_NORMAL);
        if (FT_Load_Glyph(face_, index, loadFlags)) return sglErrFailed;
        // Only synthesize styles absent from the actual face; do not
        // embolden an already-bold font a second time.
        if ((flags_ & SGLFontStyle::styleBold) && !(face_->style_flags & FT_STYLE_FLAG_BOLD))
            FT_GlyphSlot_Embolden(face_->glyph);
        if ((flags_ & SGLFontStyle::styleItalic) && !(face_->style_flags & FT_STYLE_FLAG_ITALIC))
            FT_GlyphSlot_Oblique(face_->glyph);
        if (FT_Render_Glyph(face_->glyph, monochrome ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL))
            return sglErrFailed;
        const auto& bitmap = face_->glyph->bitmap;
        if (bitmap.width > 16384 || bitmap.rows > 16384 ||
            (bitmap.rows && size_t(bitmap.width) > maxGlyphBytes / bitmap.rows)) return sglErrFailed;
        if ((bitmap.width && bitmap.rows) && (!bitmap.buffer ||
            (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY && bitmap.pixel_mode != FT_PIXEL_MODE_MONO)))
            return sglErrNotSupported;

        const int ascent = Ceil26(face_->size->metrics.ascender);
        const int descent = Ceil26(-face_->size->metrics.descender);
        const int left = face_->glyph->bitmap_left;
        const int top = ascent - face_->glyph->bitmap_top;
        SGLFontMetrics result{};
        result.nAscent = ascent / sampleScale_;
        result.nDescent = descent / sampleScale_;
        result.nHeight = (ascent + descent) / sampleScale_;
        result.nLeading = std::max(0, Round26(face_->size->metrics.height) - ascent - descent) / sampleScale_;
        result.nWidth = Round26(face_->glyph->advance.x) / sampleScale_;
        result.rctExterior.x = sampleScale_ == 2 ? FloorHalf(left) : left;
        result.rctExterior.y = sampleScale_ == 2 ? FloorHalf(top) : top;
        result.rctExterior.w = sampleScale_ == 2 ?
            CeilHalf(left + int(bitmap.width)) - result.rctExterior.x : int(bitmap.width);
        result.rctExterior.h = sampleScale_ == 2 ?
            CeilHalf(top + int(bitmap.rows)) - result.rctExterior.y : int(bitmap.rows);
        // Empty glyphs (notably spaces) carry advance/cell metrics but no image.
        if (!bitmap.width || !bitmap.rows) result.rctExterior.w = result.rctExterior.h = 0;
        const size_t width = size_t(result.rctExterior.w), height = size_t(result.rctExterior.h);
        if (height && width > maxGlyphBytes / height) return sglErrFailed;
        std::vector<uint8_t> raster(width * height);
        const auto source = [&](int x, int y) -> unsigned {
            if (x < 0 || y < 0 || x >= int(bitmap.width) || y >= int(bitmap.rows)) return 0;
            const size_t pitch = bitmap.pitch < 0 ? size_t(-int64_t(bitmap.pitch)) : size_t(bitmap.pitch);
            const size_t row = bitmap.pitch < 0 ? bitmap.rows - 1u - unsigned(y) : unsigned(y);
            const auto* line = bitmap.buffer + row * pitch;
            if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) return (line[x >> 3] & (0x80u >> (x & 7))) ? 255 : 0;
            if (bitmap.num_grays <= 1) return 0;
            return unsigned(line[x]) * 255u / (bitmap.num_grays - 1u);
        };
        for (size_t y = 0; y < height; ++y) for (size_t x = 0; x < width; ++x) {
            const int sx = (result.rctExterior.x + int(x)) * sampleScale_ - left;
            const int sy = (result.rctExterior.y + int(y)) * sampleScale_ - top;
            unsigned gray = source(sx, sy);
            if (sampleScale_ == 2)
                gray = (gray + source(sx + 1, sy) + source(sx, sy + 1) + source(sx + 1, sy + 1) + 2) / 4;
            raster[y * width + x] = static_cast<uint8_t>(gray);
        }
        metrics_ = result;
        pixels_ = std::move(raster);
        return sglErrSuccess;
    }

    // Each font owns its FreeType library/face. Only immutable original bytes
    // are shared, so changing one sprite's size cannot change another sprite.
    std::shared_ptr<const FontBytes> bytes_;
    std::wstring family_;
    FT_Library library_ = nullptr;
    FT_Face face_ = nullptr;
    std::mutex mutex_;
    bool configured_ = false, cached_ = false;
    uint32_t flags_ = 0, cachedCharacter_ = 0;
    int sampleScale_ = 1;
    SGLFontMetrics metrics_{};
    std::vector<uint8_t> pixels_;
};

std::shared_ptr<const FontBytes> ReadFont(const wchar_t* path, const OpenTypeFileOpener& opener) {
    std::unique_ptr<SSystem::SFileInterface> file(opener ? opener(path) :
        SSystem::SFileOpener::DefaultNewOpenFile(path, SSystem::SFileOpener::shareRead));
    if (!file) return {};
    const auto length = file->GetLength();
    if (length <= 0 || uint64_t(length) > maxFontBytes) return {};
    auto data = std::make_shared<FontBytes>(static_cast<size_t>(length));
    size_t offset = 0;
    while (offset < data->size()) {
        const auto read = file->Read(data->data() + offset, data->size() - offset);
        if (!read || read > data->size() - offset) return {};
        offset += read;
    }
    return data;
}

struct StockLock {
    StockLock() { SSystem::QuickLock(); }
    ~StockLock() { SSystem::QuickUnlock(); }
};
class FontStockAccess : public SGLFont {
public:
    static SGLFontObject* Find(const wchar_t* name) { return m_pFontStock ? m_pFontStock->GetAs(name) : nullptr; }
};
void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
bool SameMetrics(const SGLFontMetrics& a, const SGLFontMetrics& b) {
    return a.nFlags == b.nFlags && a.nAscent == b.nAscent && a.nDescent == b.nDescent &&
        a.nLeading == b.nLeading && a.nWidth == b.nWidth && a.nHeight == b.nHeight &&
        a.rctExterior.x == b.rctExterior.x && a.rctExterior.y == b.rctExterior.y &&
        a.rctExterior.w == b.rctExterior.w && a.rctExterior.h == b.rctExterior.h;
}
std::vector<uint8_t> Rasterize(SGLFontObject& font, uint32_t character, SGLFontMetrics& metrics) {
    Require(font.GetMetrics(nullptr, 0, metrics, character) == sglErrSuccess, "glyph metrics failed");
    Require(metrics.nHeight > 0 && metrics.nWidth > 0 && metrics.rctExterior.w > 0 &&
        metrics.rctExterior.h > 0, "visible glyph metrics empty");
    std::vector<uint8_t> result(size_t(metrics.rctExterior.w) * metrics.rctExterior.h);
    SGLFontMetrics actual{};
    Require(font.GetMetrics(result.data(), result.size(), actual, character) == sglErrSuccess &&
        SameMetrics(metrics, actual), "metrics changed while rasterizing glyph");
    Require(std::any_of(result.begin(), result.end(), [](uint8_t v) { return v != 0; }), "glyph is transparent");
    return result;
}
} // namespace

OpenTypeRegistration RegisterOpenTypeFont(const std::wstring& path,
                                         const std::wstring& expectedFamily,
                                         const OpenTypeFileOpener& opener,
                                         const std::wstring& configuredName) {
    OpenTypeRegistration result;
    try {
        auto data = ReadFont(path.c_str(), opener);
        if (!data) throw std::runtime_error("font cannot be read or exceeds 64 MiB");
        struct FontInspection {
            FT_Library library = nullptr;
            FT_Face face = nullptr;
            ~FontInspection() {
                if (face) FT_Done_Face(face);
                if (library) FT_Done_FreeType(library);
            }
        } inspection;
        if (FT_Init_FreeType(&inspection.library) ||
            FT_New_Memory_Face(inspection.library, data->data(),
                              static_cast<FT_Long>(data->size()), 0, &inspection.face))
            throw std::runtime_error("invalid OpenType/TrueType font");
        if (!FT_IS_SCALABLE(inspection.face) || !inspection.face->family_name ||
            FT_Select_Charmap(inspection.face, FT_ENCODING_UNICODE))
            throw std::runtime_error("font lacks scalable Unicode glyphs or a family name");
        SSystem::SString family;
        family.FromUTF8(reinterpret_cast<const uint8_t*>(inspection.face->family_name));
        result.family = static_cast<const wchar_t*>(family);
        if (result.family.empty()) throw std::runtime_error("font family is empty");
        if (!expectedFamily.empty() && expectedFamily != result.family)
            throw std::runtime_error("font family does not match the required compatibility family");
        result.registeredName = configuredName.empty() ? result.family : configuredName;
        auto font = std::make_unique<OpenTypeFont>(data, result.registeredName);
        SGLFontStyle style;
        style.pszFace = result.registeredName.c_str();
        style.nSize = 16;
        if (font->SetStyle(style) != sglErrSuccess)
            throw std::runtime_error("font cannot create a glyph rasterizer");
        // A family entry is convenient for scripts that request the real family
        // rather than the XML name. A second named face must still retain its
        // own bytes/style; aliasing it to the first family entry loses Bold etc.
        std::unique_ptr<OpenTypeFont> familyFont;
        if (result.registeredName != result.family) {
            familyFont = std::make_unique<OpenTypeFont>(data, result.family);
            style.pszFace = result.family.c_str();
            if (familyFont->SetStyle(style) != sglErrSuccess)
                throw std::runtime_error("font family cannot create a glyph rasterizer");
        }
        {
            StockLock lock;
            if (auto* existing = FontStockAccess::Find(result.registeredName.c_str())) {
                auto* existingFace = dynamic_cast<OpenTypeFont*>(existing);
                if (!existingFace || *existingFace->Bytes() != *data)
                    throw std::runtime_error("configured font name is already used by a different face");
            } else SGLFont::RegisterStockFont(result.registeredName.c_str(), font.release());
            if (familyFont && !FontStockAccess::Find(result.family.c_str()))
                SGLFont::RegisterStockFont(result.family.c_str(), familyFont.release());
        }
        result.loaded = true;
        LogPrint(LogPriority::Info, "EntisGLS", "SDL OpenType face available: %ls (family %ls), %zu bytes from %ls",
                 result.registeredName.c_str(), result.family.c_str(), data->size(), path.c_str());
    } catch (const std::exception& error) {
        result.error = error.what();
        LogPrint(LogPriority::Warn, "EntisGLS", "OpenType font %ls: %s", path.c_str(), error.what());
    }
    return result;
}

bool CheckOpenTypeFont(const std::wstring& family,
                       const std::vector<uint32_t>& visibleCharacters,
                       const std::vector<uint32_t>& spaceCharacters) {
    try {
        Require(!visibleCharacters.empty(), "font probe has no visible characters");
        const auto sampleCharacter = visibleCharacters.front();
        std::shared_ptr<const FontBytes> data;
        {
            StockLock lock;
            auto* registered = dynamic_cast<OpenTypeFont*>(FontStockAccess::Find(family.c_str()));
            Require(registered != nullptr, "OpenType family not registered");
            data = registered->Bytes();
        }
        size_t glyphs = 0;
        for (const auto size : {16u, 40u, 56u}) {
            SGLFontStyle style;
            style.pszFace = family.c_str();
            style.nSize = size;
            SGLFont font;
            Require(font.SetStyle(style) == sglErrSuccess, "public font selection failed");
            for (const auto character : visibleCharacters) {
                SGLFontMetrics metrics{};
                const auto pixels = Rasterize(font, character, metrics);
                Require(metrics.nHeight >= int(size) - 1 && metrics.nHeight <= int(size) + 2,
                    "font size no longer means SDK character-cell height");
                Require(metrics.rctExterior.y >= -2 && metrics.rctExterior.y + metrics.rctExterior.h <= metrics.nHeight + 2,
                    "Glyph falls outside its line cell");
                std::vector<uint8_t> small(pixels.size(), 0x5a);
                Require(font.GetMetrics(small.data(), small.size() - 1, metrics, character) != sglErrSuccess &&
                    std::all_of(small.begin(), small.end(), [](uint8_t v) { return v == 0x5a; }),
                    "short glyph buffer was accepted or partially overwritten");
                ++glyphs;
            }
            for (const auto character : spaceCharacters) {
                SGLFontMetrics metrics{};
                uint8_t guard = 0x5a;
                Require(font.GetMetrics(&guard, 0, metrics, character) == sglErrSuccess && guard == 0x5a &&
                    metrics.nWidth > 0 && metrics.nHeight > 0 && metrics.rctExterior.w == 0 && metrics.rctExterior.h == 0,
                    "space must advance without allocating an image");
            }
            for (const auto character : {0xd800u, 0xdfffu, 0x110000u, 0xffffffffu, 0x10ffffu}) {
                SGLFontMetrics metrics{};
                Require(font.GetMetrics(nullptr, 0, metrics, character) != sglErrSuccess,
                    "invalid or missing character became a fake glyph");
            }
            for (const auto invalid : {0u, 4097u, UINT32_MAX}) {
                style.nSize = invalid;
                Require(font.SetStyle(style) != sglErrSuccess, "invalid font size accepted");
                SGLFontMetrics metrics{};
                Require(font.GetMetrics(nullptr, 0, metrics, sampleCharacter) != sglErrSuccess, "invalid style retained stale glyphs");
            }
        }

        SGLFontStyle style;
        style.pszFace = family.c_str();
        style.nSize = 40;
        OpenTypeFont styled(data, family);
        Require(styled.SetStyle(style) == sglErrSuccess, "font style setup failed");
        SGLFontMetrics plainMetrics{};
        const auto plain = Rasterize(styled, sampleCharacter, plainMetrics);
        style.nStyles = SGLFontStyle::styleBold;
        Require(styled.SetStyle(style) == sglErrSuccess, "bold style failed");
        SGLFontMetrics boldMetrics{};
        const auto bold = Rasterize(styled, sampleCharacter, boldMetrics);
        if (styled.IsBoldFace())
            Require(bold == plain && SameMetrics(plainMetrics, boldMetrics),
                    "already-bold original face was emboldened twice");
        else Require(bold != plain, "requested bold did not change a regular face");
        style.nStyles = SGLFontStyle::styleItalic;
        Require(styled.SetStyle(style) == sglErrSuccess, "italic style failed");
        SGLFontMetrics italicMetrics{};
        Require(Rasterize(styled, sampleCharacter, italicMetrics) != plain, "italic style did not change actual glyph outline");
        style.nStyles = SGLFontStyle::styleNoSmooth;
        Require(styled.SetStyle(style) == sglErrSuccess, "monochrome style failed");
        SGLFontMetrics monoMetrics{};
        const auto mono = Rasterize(styled, sampleCharacter, monoMetrics);
        Require(std::all_of(mono.begin(), mono.end(), [](uint8_t v) { return v == 0 || v == 255; }),
            "NoSmooth font still has grayscale edge pixels");
        style.nStyles = SGLFontStyle::styleHighDefinition;
        Require(styled.SetStyle(style) == sglErrSuccess, "high-definition style failed");
        SGLFontMetrics highMetrics{};
        const auto high = Rasterize(styled, sampleCharacter, highMetrics);
        Require(std::any_of(high.begin(), high.end(), [](uint8_t v) { return v > 0 && v < 255; }),
            "high-definition glyph lost antialiasing");
        style.nStyles = 0x80000000u;
        Require(styled.SetStyle(style) == sglErrNotSupported, "unknown font flags silently ignored");

        // Separate FreeType faces and libraries keep simultaneous message/UI
        // sizes isolated. Verify real bytes under concurrent repeated use.
        style.nStyles = 0;
        style.nSize = 16;
        OpenTypeFont small(data, family), large(data, family);
        Require(small.SetStyle(style) == sglErrSuccess, "small font failed");
        style.nSize = 56;
        Require(large.SetStyle(style) == sglErrSuccess, "large font failed");
        SGLFontMetrics smallMetrics{}, largeMetrics{};
        const auto smallPixels = Rasterize(small, sampleCharacter, smallMetrics);
        const auto largePixels = Rasterize(large, sampleCharacter, largeMetrics);
        Require(largeMetrics.nHeight > smallMetrics.nHeight && largePixels.size() > smallPixels.size(),
            "independent font sizes collapsed into one face size");
        std::array<bool, 2> passed{true, true};
        const auto work = [&](size_t index, OpenTypeFont& font, const std::vector<uint8_t>& expected) {
            try { for (int i = 0; i < 50; ++i) { SGLFontMetrics m{}; if (Rasterize(font, sampleCharacter, m) != expected) passed[index] = false; } }
            catch (...) { passed[index] = false; }
        };
        std::thread worker(work, 0, std::ref(small), std::cref(smallPixels));
        work(1, large, largePixels);
        worker.join();
        Require(passed[0] && passed[1], "simultaneous font instances changed each other's rasterization");

        // Give this generator a unique backing allocation and destroy both the
        // generator and external byte owner before asking its child for glyphs.
        auto transientData = std::make_shared<const FontBytes>(*data);
        std::weak_ptr<const FontBytes> lifetime(transientData);
        auto generator = std::make_unique<OpenTypeFont>(transientData, family);
        std::unique_ptr<SGLFontObject> child(generator->NewFont(style));
        Require(child != nullptr, "font child creation failed");
        generator.reset();
        transientData.reset();
        Require(!lifetime.expired(), "child lost original font bytes after generator destruction");
        SGLFontMetrics childMetrics{};
        Rasterize(*child, sampleCharacter, childMetrics);
        child.reset();
        Require(lifetime.expired(), "font bytes leaked after final instance destruction");

        LogPrint(LogPriority::Info, "EntisGLS",
            "SDL OpenType probe PASS: %ls, %zu glyphs at 16/40/56px, cell metrics, spaces, styles, invalid scalars/sizes/buffers, independent concurrent faces, source lifetime",
            family.c_str(), glyphs);
        return true;
    } catch (const std::exception& error) {
        LogPrint(LogPriority::Error, "EntisGLS", "SDL OpenType probe FAIL: %s", error.what());
        return false;
    }
}
} // namespace study::platform::sdl
