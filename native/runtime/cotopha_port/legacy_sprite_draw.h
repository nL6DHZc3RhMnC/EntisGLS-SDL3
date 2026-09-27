#pragma once
#include <memory>
#include <cstdint>
class ECSContext;
class ECSSprite;
class ECSObject;
class ECSResource;
class ESLFileObject;
namespace SakuraGL {struct SGLRect;class S3DRenderContextInterface;class SGLImageObject;}
template<class T> class ECSObjArray;
struct LegacySpriteDrawState;

ESLError HandleLegacySpriteDraw(ECSContext &, ECSSprite &, const wchar_t *method,
                               ECSObjArray<ECSObject> &, bool &handled);
void ResetLegacySpriteDrawState(ECSSprite &,bool preserveTone=false);
bool CheckLegacySpriteDraw();
bool LegacyEffectAnimationEnabled();
ESLError SaveLegacySpriteVisual(ESLFileObject&,ECSSprite&);
ESLError LoadLegacySpriteVisual(ESLFileObject&,ECSSprite&);
ESLError SaveLegacySpriteAnimation(ESLFileObject&,ECSSprite&);
ESLError LoadLegacySpriteAnimation(ESLFileObject&,ECSSprite&);
ESLError CommitLegacySpriteDrawing(ECSContext&,ECSSprite&,ECSResource* source,int frame,
                                  const SakuraGL::SGLRect&,ECSResource* alpha);
void RestoreLegacySpriteVisual(ECSSprite&);

// Some GLS3 effects use blend degree without changing ordinary transparency.
// Actions hold this state by shared ownership; it never owns the script sprite.
struct LegacySpriteBlendEffect {
    virtual ~LegacySpriteBlendEffect() = default;
    virtual uint32_t GetDegree() const = 0;
    virtual void SetDegree(uint32_t) = 0;
};
void SetLegacySpriteBlendEffect(ECSSprite&,std::shared_ptr<LegacySpriteBlendEffect>);
uint32_t GetLegacySpriteEffectDegree(ECSSprite&);

// GLS3 Refresh writes tone/alpha into GetInfo before a SuperSprite effect.
// Modern filters keep a private image, so effects explicitly obtain that stage.
SakuraGL::SGLImageObject* GetLegacySpriteFilteredImage(ECSSprite&,
    SakuraGL::S3DRenderContextInterface&,SakuraGL::SGLImageObject*);
