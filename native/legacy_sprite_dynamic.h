#pragma once
#include <sakuraglx/sprite/sglx_sprite.h>

bool LegacySpriteDynamicModeEnabled(const SakuraGL::SGLSprite*);
// Returns the GLS3 direct-child mode and its integral drawing offset. A
// buffered sprite falls back to its ordinary image when faded or transformed.
bool GetLegacySpriteDynamicTransform(const SakuraGL::SGLSprite&,SakuraGL::SGLAffine&,
    const SakuraGL::SGLSprite::Virtual3DParam* = nullptr,
    SakuraGL::SGLSprite::Stereo3DView = SakuraGL::SGLSprite::s3dMonoview);
bool GetLegacySpriteDynamicRectangle(const SakuraGL::SGLSprite&,const SakuraGL::SGLAffine&,
    SakuraGL::SGLRect&);
bool CheckLegacySpriteDynamic();
