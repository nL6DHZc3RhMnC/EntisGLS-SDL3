#pragma once
#include "legacy_sprite_draw.h"
#include <array>
#include <memory>
namespace SakuraGL {class SGLSprite;class SGLImageObject;}

// Call under the same SSystem lock used for Sprite drawing and actions.
// Filter returns a private premultiplied ARGB32 image (RGB32 for an opaque
// source). Use normal Sprite geometry/paint flags and GetDrawTransparency.
class LegacySuperShadingState final : public LegacySpriteBlendEffect {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    LegacySuperShadingState(SakuraGL::SGLSprite&,const std::array<uint32_t,33>& words,uint32_t degree);
    ~LegacySuperShadingState() override;
    uint32_t GetDegree() const override;
    void SetDegree(uint32_t) override;
    uint32_t GetDrawTransparency(uint32_t normal) const;
    SakuraGL::SGLImageObject* Filter(SakuraGL::SGLImageObject* source);
    friend bool CheckLegacySuperShading();
};
bool CheckLegacySuperShading();
