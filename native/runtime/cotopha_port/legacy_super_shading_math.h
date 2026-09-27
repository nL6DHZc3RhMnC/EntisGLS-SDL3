#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace LegacySuperShading {
// GLS3's x86 implementation selects SSE pavgb for even-width images and
// the scalar (floor-average) kernel for odd widths. Borders are copied in
// the horizontal pass; top/bottom rows omit the vertical pass.
bool Loop421(uint8_t* destination,const uint8_t* source,uint32_t width,uint32_t height,
             std::vector<uint8_t>& scratch);
uint32_t SquareDegree(uint32_t degree);
uint32_t DrawTransparency(uint32_t kind,uint32_t flags,uint32_t degree,uint32_t normal);

class Raster {
    uint32_t width_=0,height_=0,levelsReady_=0;
    std::array<std::vector<uint8_t>,5> levels_;
    std::vector<uint8_t> scratch_;
    uint64_t revision_=0,passes_=0;
public:
    bool SetSource(const uint8_t* pixels,uint32_t width,uint32_t height,ptrdiff_t stride);
    bool Render(uint32_t kind,uint32_t degree,std::vector<uint8_t>& output);
    uint64_t Revision() const {return revision_;}
    uint64_t Passes() const {return passes_;}
};
}
