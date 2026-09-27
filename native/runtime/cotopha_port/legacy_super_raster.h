#pragma once
#include "runtime/cotopha_port/legacy_sprite.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_super_raster_math.h"

// Call under the same SSystem lock as the owning ECSSuperSprite. x/y are the
// original integer destination minus rotation centre, not an affine transform.
// source must already include base RefreshRectPostFilter tone/alpha processing;
// the effect draws that image and must not apply those filters a second time.
class LegacySuperRaster final : public LegacySpriteBlendEffect {
    SSystem::SSmartReference<SakuraGL::SGLSprite> owner_;
    LegacySuperRasterMath math_;
    SakuraGL::SGLImage frame_;
public:
    LegacySuperRaster(SakuraGL::SGLSprite&,const std::array<uint32_t,33>&,uint32_t degree);
    static ESLError Validate(const std::array<uint32_t,33>&);
    uint32_t GetDegree() const override;
    void SetDegree(uint32_t) override;
    void Advance(uint32_t milliseconds,bool animationEnabled);
    SakuraGL::SGLError Draw(SakuraGL::SGLPaintContextInterface&,SakuraGL::SGLImageObject*,int32_t x,int32_t y);
    bool GetRectangle(SakuraGL::SGLRect&,int32_t x,int32_t y,uint32_t width,uint32_t height) const;
    uint32_t IntervalCounter() const { return math_.IntervalCounter(); }
};

// Actual SDK paint context, current-frame and clipping regression. No device,
// EGL context, filesystem, game roots or script state is changed by this probe.
bool CheckLegacySuperRaster();
