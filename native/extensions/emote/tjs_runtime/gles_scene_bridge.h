#pragma once
#include "Player.h"
#include "RenderManager.h"
namespace studysteady::motion {
// Original native GPU item's stencil, mesh tessellation and batching pipeline.
void renderSceneItems(bool priorDraw, iTVPTexture2D *targetTexture,
    const ::motion::D3DTargetTextureGetter_guess &targetTextureGetter,
    const tTVPRect &targetRect,
    const ::motion::D3DSourceTextureGetter_guess &sourceTextureGetter,
    ::motion::detail::PreparedRenderItemList &mainList,float xOffset,float yOffset);
}
