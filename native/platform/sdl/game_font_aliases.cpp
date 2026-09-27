#include "platform/sdl/game_font_aliases.h"
#include "platform/log.h"
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_font.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace study::platform::sdl {
namespace {
using namespace SakuraGL;

struct StockLock {
    StockLock() { SSystem::QuickLock(); }
    ~StockLock() { SSystem::QuickUnlock(); }
};

class FontStockAccess : public SGLFont {
public:
    // The SDK exposes registration but not lookup. Access the protected stock
    // table only while holding its existing global lock.
    static SGLFontObject* Find(const wchar_t* name) {
        return m_pFontStock ? m_pFontStock->GetAs(name) : nullptr;
    }
};

class StockFontAlias final : public SGLFontObject {
public:
    StockFontAlias(SGLFontObject* source, const std::wstring& sourceName)
        : source_(source), sourceName_(sourceName) {}

    SGLFontObject* NewFont(const SGLFontStyle& style) override {
        auto result = std::make_unique<StockFontAlias>(source_.GetReference(), sourceName_);
        if (result->SetStyle(style) != sglErrSuccess) return nullptr;
        return result.release();
    }

    SGLError SetStyle(const SGLFontStyle& style) override {
        font_.reset();
        // The SDK BMF resampler divides by nSize. A zero-sized font cannot be
        // rasterized; do not manufacture a successful font with no glyphs.
        if (style.nSize == 0) return sglErrInvalidParam;
        auto* source = source_.GetReference();
        if (!source) return sglErrFailed;
        SGLFontStyle actual(style);
        actual.pszFace = sourceName_.c_str();
        std::unique_ptr<SGLFontObject> next(source->NewFont(actual));
        if (!next) return sglErrFailed;
        const auto error = next->SetStyle(actual);
        if (error != sglErrSuccess) return error;
        font_ = std::move(next);
        return sglErrSuccess;
    }

    SGLError GetMetrics(uint8_t* rasterized, size_t bytes,
                        SGLFontMetrics& metrics, uint32_t character) override {
        if (!source_.GetReference() || !font_) return sglErrFailed;
        SGLFontMetrics actual{};
        const auto error = font_->GetMetrics(nullptr, 0, actual, character);
        if (error != sglErrSuccess) return error;
        metrics = actual;
        if (!rasterized) return sglErrSuccess;
        if (actual.rctExterior.w < 0 || actual.rctExterior.h < 0) return sglErrFailed;
        const size_t width = static_cast<size_t>(actual.rctExterior.w);
        const size_t height = static_cast<size_t>(actual.rctExterior.h);
        if (height && width > std::numeric_limits<size_t>::max() / height) return sglErrFailed;
        // The BMF native-size path otherwise copies a truncated buffer and
        // returns success. Callers require a complete grayscale glyph.
        if (bytes < width * height) return sglErrInvalidParam;
        return font_->GetMetrics(rasterized, bytes, metrics, character);
    }

private:
    // SSmartReference is a non-owning, invalidating SDK reference. Registry
    // teardown may delete the original BMF before its aliases, without UAF or
    // registering the same owning pointer under two stock names.
    SSystem::SSmartReference<SGLFontObject> source_;
    std::wstring sourceName_;
    std::unique_ptr<SGLFontObject> font_;
};

void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool SameMetrics(const SGLFontMetrics& a, const SGLFontMetrics& b) {
    return a.nFlags == b.nFlags && a.nAscent == b.nAscent && a.nDescent == b.nDescent &&
           a.nLeading == b.nLeading && a.nWidth == b.nWidth && a.nHeight == b.nHeight &&
           a.rctExterior.x == b.rctExterior.x && a.rctExterior.y == b.rctExterior.y &&
           a.rctExterior.w == b.rctExterior.w && a.rctExterior.h == b.rctExterior.h;
}

std::vector<uint8_t> Rasterize(SGLFontObject& font, uint32_t character, SGLFontMetrics& metrics) {
    Require(font.GetMetrics(nullptr, 0, metrics, character) == sglErrSuccess, "glyph metrics failed");
    Require(metrics.nHeight > 0 && metrics.nWidth > 0 &&
            metrics.rctExterior.w > 0 && metrics.rctExterior.h > 0, "glyph metrics are empty");
    std::vector<uint8_t> raster(static_cast<size_t>(metrics.rctExterior.w) * metrics.rctExterior.h);
    Require(font.GetMetrics(raster.data(), raster.size(), metrics, character) == sglErrSuccess,
            "glyph rasterization failed");
    Require(std::any_of(raster.begin(), raster.end(), [](uint8_t value) { return value != 0; }),
            "glyph raster is entirely transparent");
    return raster;
}
} // namespace

bool HasStockFont(const std::wstring& name) {
    StockLock lock;
    return !name.empty() && FontStockAccess::Find(name.c_str());
}

bool RegisterFontAlias(const std::wstring& alias, const std::wstring& source) {
    if (alias.empty() || source.empty()) return false;
    StockLock lock;
    // Explicit configuration and original SDK fonts always win over a fallback.
    if (FontStockAccess::Find(alias.c_str())) return true;
    auto* generator = FontStockAccess::Find(source.c_str());
    if (!generator) return false;
    auto result = std::make_unique<StockFontAlias>(generator, source);
    SGLFont::RegisterStockFont(alias.c_str(), result.release());
    LogPrint(LogPriority::Info, "EntisGLS", "SDL font alias: %ls -> %ls", alias.c_str(), source.c_str());
    return true;
}

bool CheckFontAlias(const std::wstring& aliasName, const std::wstring& sourceName,
                    const std::vector<uint32_t>& visibleCharacters) {
    StockLock lock;
    try {
        size_t checkedGlyphs = 0;
        Require(!visibleCharacters.empty(), "font alias probe has no visible characters");
        {
            auto* source = FontStockAccess::Find(sourceName.c_str());
            auto* generator = FontStockAccess::Find(aliasName.c_str());
            Require(source && generator && source != generator, "alias has no independent stock ownership");
            int previousHeight = 0;
            for (const uint32_t size : {16u, 32u, 64u}) {
                SGLFontStyle style;
                style.pszFace = aliasName.c_str();
                style.nSize = size;
                style.nStyles = 0;
                std::unique_ptr<SGLFontObject> alias(generator->NewFont(style));
                style.pszFace = sourceName.c_str();
                std::unique_ptr<SGLFontObject> original(source->NewFont(style));
                Require(alias && original && alias.get() != original.get(), "font instance creation failed");
                for (const uint32_t character : visibleCharacters) {
                    SGLFontMetrics actual{}, expected{};
                    const auto bytes = Rasterize(*alias, character, actual);
                    const auto reference = Rasterize(*original, character, expected);
                    Require(SameMetrics(actual, expected) && bytes == reference,
                            "alias altered font metrics or grayscale pixels");
                    ++checkedGlyphs;
                    std::vector<uint8_t> small(bytes.size(), 0x5a);
                    Require(alias->GetMetrics(small.data(), small.size() - 1, actual, character) != sglErrSuccess,
                            "undersized glyph buffer incorrectly succeeded");
                    Require(std::all_of(small.begin(), small.end(), [](uint8_t v) { return v == 0x5a; }),
                            "undersized glyph buffer was partially overwritten");
                    if (character == visibleCharacters.back()) {
                        Require(actual.nHeight > previousHeight, "font size did not change glyph height");
                        previousHeight = actual.nHeight;
                    }
                }
                SGLFontMetrics metrics{};
                Require(alias->GetMetrics(nullptr, 0, metrics, 0x10ffff) != sglErrSuccess,
                        "missing glyph incorrectly succeeded");
                style.nSize = 0;
                Require(alias->SetStyle(style) != sglErrSuccess, "zero font size incorrectly succeeded");
                Require(alias->GetMetrics(nullptr, 0, metrics, visibleCharacters.front()) != sglErrSuccess,
                        "invalid alias style retained stale glyphs");
                Require(generator->NewFont(style) == nullptr, "invalid NewFont returned a usable font");
            }

            SGLFontStyle style;
            style.pszFace = sourceName.c_str();
            style.nSize = 16;
            style.nStyles = 0;
            std::unique_ptr<SGLFontObject> transient(source->NewFont(style));
            Require(transient != nullptr, "transient font reference creation failed");
            StockFontAlias expires(transient.get(), sourceName.c_str());
            Require(expires.SetStyle(style) == sglErrSuccess, "transient alias style failed");
            transient.reset();
            SGLFontMetrics metrics{};
            Require(expires.GetMetrics(nullptr, 0, metrics, visibleCharacters.front()) != sglErrSuccess &&
                    expires.SetStyle(style) != sglErrSuccess && expires.NewFont(style) == nullptr,
                    "alias continued using a destroyed source");

            // Exercise the public wrapper too, including its stock-generator
            // failure path; a missing/invalid font must not appear successful.
            SGLFont wrapper;
            style.pszFace = aliasName.c_str();
            Require(wrapper.SetStyle(style) == sglErrSuccess, "public alias selection failed");
            Rasterize(wrapper, visibleCharacters.front(), metrics);
            style.nSize = 0;
            Require(wrapper.SetStyle(style) != sglErrSuccess, "public zero-size alias incorrectly succeeded");
            Require(wrapper.GetMetrics(nullptr, 0, metrics, visibleCharacters.front()) != sglErrSuccess,
                    "invalid public font retained a stale glyph source");
        }
        LogPrint(LogPriority::Info, "EntisGLS",
                 "SDL font alias probe PASS: %ls -> %ls, %zu glyphs, 16/32/64px, exact pixels/metrics, buffer/size/missing/lifetime failures",
                 aliasName.c_str(), sourceName.c_str(), checkedGlyphs);
        return true;
    } catch (const std::exception& error) {
        LogPrint(LogPriority::Error, "EntisGLS", "SDL font alias probe FAIL: %s", error.what());
        return false;
    }
}
} // namespace study::platform::sdl
