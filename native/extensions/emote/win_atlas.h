#pragma once

#include "PSBRawFile.h"
#include <cstdint>
#include <string>
#include <vector>

namespace studysteady::motion {

// Renderer boundary for MotionPlayer's spec=win source route. Coordinates are
// source pixels (top-left origin), as in PlayerResource.cpp::findSource.
struct AtlasIcon {
    std::string name;
    int left = 0, top = 0, width = 0, height = 0;
    int originX = 0, originY = 0;
};

// Retains the PSB raw owner: pixel storage remains alive after the caller drops
// its PSBFile. RGBA8 data can therefore be passed directly to GLES without a
// second 64 MiB atlas allocation on haz_a.psb.
class WinAtlas {
public:
    WinAtlas(const PSB::PSBRawNode &root, const std::string &group);
    WinAtlas(const WinAtlas &) = delete;
    WinAtlas &operator=(const WinAtlas &) = delete;
    WinAtlas(WinAtlas &&) = delete;
    WinAtlas &operator=(WinAtlas &&) = delete;
    int width() const { return width_; }
    int height() const { return height_; }
    int truncatedWidth() const { return truncatedWidth_; }
    int truncatedHeight() const { return truncatedHeight_; }
    const std::string &format() const { return format_; }
    const std::vector<AtlasIcon> &icons() const { return icons_; }
    const AtlasIcon &icon(const std::string &name) const;
    const uint8_t *rgbaPixels() const { return rgba_; }
    size_t rgbaBytes() const { return static_cast<size_t>(width_) * height_ * 4; }

    // Kirikiri's renderer consumes BGRA after TVPReverseRGB. This exposes that
    // existing byte convention for a future iTVPTexture2D bridge. GLES uses the
    // original RGBA bytes directly, with exactly the same visible channels.
    std::vector<uint8_t> copyBgra() const;
    std::vector<uint8_t> copyIconRgba(const AtlasIcon &icon) const;

private:
    PSB::PSBRawNode texture_;
    int width_ = 0, height_ = 0;
    int truncatedWidth_ = 0, truncatedHeight_ = 0;
    std::string format_;
    const uint8_t *rgba_ = nullptr;
    std::vector<uint8_t> converted_;
    std::vector<AtlasIcon> icons_;
};

// Loads the actual game's header-encrypted PSB through the existing verified
// reader. No TJS dispatch, animation interpreter or rendering stub is emulated.
PSB::PSBFile loadPsb(const std::string &path, uint32_t seed);
}
