#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_sprite_dynamic.h"
#include <cmath>

bool GetLegacySpriteDynamicTransform(const SakuraGL::SGLSprite& sprite,SakuraGL::SGLAffine& offset,
    const SakuraGL::SGLSprite::Virtual3DParam* view,SakuraGL::SGLSprite::Stereo3DView stereo) {
    if(!LegacySpriteDynamicModeEnabled(&sprite))return false;
    SakuraGL::SGLPaintParam parameters;SakuraGL::SGLAffine affine;
    if(!sprite.GetPaintParam(parameters,affine,view,stereo))return false;
    // EImageSprite::SetParameterToDraw clears pImageAxes for this identity
    // basis (epsilon 1e-5). A rotation centre changes only the base offset.
    const bool noAxes=std::abs(affine.a11-affine.a22)<1e-5f&&
        std::abs(affine.a21-affine.a12)<1e-5f&&std::abs(affine.a11-1)<1e-5f&&std::abs(affine.a21)<1e-5f;
    if(sprite.GetAttachedImage()&&(parameters.nTransparency!=0||!noAxes))return false;
    // DrawDynamicMode applies only the drawing offset, without its own image,
    // background, alpha or zoom. Fixed-point offsets use an arithmetic >>16.
    offset=SakuraGL::SGLAffine();offset.a13=std::floor(affine.a13);offset.a23=std::floor(affine.a23);
    return true;
}
bool GetLegacySpriteDynamicRectangle(const SakuraGL::SGLSprite& sprite,const SakuraGL::SGLAffine& offset,
    SakuraGL::SGLRect& rectangle) {
    bool found=false;
    // Original DrawDynamicMode accumulates bounds for hidden children too.
    for(size_t i=0;i<sprite.GetChildCount();++i) {
        auto* child=sprite.GetChildAt(i);SakuraGL::SGLRect bounds;
        if(!child||!child->GetRectangle(bounds))continue;
        if(found)rectangle|=bounds;else {rectangle=bounds;found=true;}
    }
    if(!found){rectangle=SakuraGL::SGLRect(0,0,-1,-1);return false;}
    rectangle.left+=int32_t(offset.a13);rectangle.right+=int32_t(offset.a13);
    rectangle.top+=int32_t(offset.a23);rectangle.bottom+=int32_t(offset.a23);
    return true;
}
