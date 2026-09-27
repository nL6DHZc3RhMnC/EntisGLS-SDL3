#pragma once
#include <sakuraglx/sprite/sglx_sprite.h>
#include "legacy_sprite_dynamic.h"

class ECSContext;
class ECSExecutionImage;
class ECSObject;
class ESLFileObject;
class ECSSprite;
template<class T> class ECSObjArray;
struct LegacySpriteCallbackState;

ESLError HandleLegacySpriteCallbacks(ECSContext&, ECSSprite&, const wchar_t*, ECSObjArray<ECSObject>&, bool&);
void ResetLegacySpriteCallbacks(ECSSprite&);
void IndexLegacySpriteCallbacks(ECSSprite&);
void CleanupLegacySpriteCallbacks(ECSSprite&, ECSContext&);
ESLError CommitLegacySpriteCallbacks(ECSSprite&, ECSContext&);
ECSObject* GetLegacySpriteCallbackAt(ECSSprite&, int index);
ESLError SaveLegacySpriteCallbacks(ESLFileObject&,ECSSprite&,ECSContext&);
ESLError LoadLegacySpriteCallbacks(ESLFileObject&,ECSSprite&,ECSContext&);
// Call before destroying the image or releasing its globals, after stopping main execution.
void ShutdownLegacySpriteCallbacksForImage(ECSExecutionImage&);
bool CheckLegacySpriteCallbacks();

enum class LegacySpriteCallback { HitTest, Timer, Mouse, Key };
bool InvokeLegacySpriteCallback(const SakuraGL::SGLSprite*, LegacySpriteCallback,
                               int ordinal, double a=0, double b=0, double c=0,
                               bool* attached=nullptr);

// The legacy interface has ordering requirements that the current SDK's single
// listener slot cannot express. Keep that slot available to native skin controls.
template<class Base> class LegacyCallbackSprite : public Base {
public:
    using Base::Base;
    void Draw(SakuraGL::S3DRenderContextInterface& render,
              const SakuraGL::SGLSprite::Virtual3DParam* view=nullptr,
              SakuraGL::SGLSprite::Stereo3DView stereo=SakuraGL::SGLSprite::s3dMonoview) const override {
        SakuraGL::SGLAffine offset;
        if(!GetLegacySpriteDynamicTransform(*this,offset,view,stereo)){Base::Draw(render,view,stereo);return;}
        if(!Base::IsVisible())return;
        render.PushTransformation();render.AppendTransformation(offset,0);
        Base::DrawChildren(render,stereo);render.PopTransformation();
    }
    void DrawImageList(SakuraGL::SGLDrawImageParamList& list,
              const SakuraGL::SGLSprite::Virtual3DParam* view=nullptr,
              SakuraGL::SGLSprite::Stereo3DView stereo=SakuraGL::SGLSprite::s3dMonoview) override {
        SakuraGL::SGLAffine offset;
        if(!GetLegacySpriteDynamicTransform(*this,offset,view,stereo)){Base::DrawImageList(list,view,stereo);return;}
        if(!Base::IsVisible())return;
        const auto saved=list.GetAffine();const auto transparency=list.GetTransparency();
        list.AppendAffine(offset);Base::DrawChildrenImageList(list,stereo);
        list.SetAffine(saved);list.SetTransparency(transparency);
    }
    bool GetRectangle(SakuraGL::SGLRect& rectangle) const override {
        SakuraGL::SGLAffine offset;
        return GetLegacySpriteDynamicTransform(*this,offset)?
            GetLegacySpriteDynamicRectangle(*this,offset,rectangle):Base::GetRectangle(rectangle);
    }
    void PostUpdate(SakuraGL::SGLRect* rectangle=nullptr) override {
        SakuraGL::SGLAffine offset;
        if(!GetLegacySpriteDynamicTransform(*this,offset)){Base::PostUpdate(rectangle);return;}
        // The cached image may already be dirty while direct children move
        // outside it. Propagate a full parent update rather than clipping them.
        Base::PostUpdate(nullptr);
        if(auto* parent=Base::GetParent())parent->PostUpdate();
    }
    bool IsHitSprite(double x,double y) const override {
        bool attached=false;
        const bool result=InvokeLegacySpriteCallback(this,LegacySpriteCallback::HitTest,0,x,y,0,&attached);
        return attached?result:Base::IsHitSprite(x,y);
    }
    void AdvanceTime(uint32_t milliseconds) override {
        InvokeLegacySpriteCallback(this,LegacySpriteCallback::Timer,0,milliseconds);
        Base::AdvanceTime(milliseconds);
    }
    bool OnMouseMove(double x,double y,int64_t flags) override {
        return InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,0,x,y)
            || Base::OnMouseMove(x,y,flags);
    }
    void OnMouseLeave(int64_t flags) override {
        InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,1);
        Base::OnMouseLeave(flags);
    }
    bool OnMouseWheel(int32_t delta,double x,double y,int64_t flags) override {
        return Base::OnMouseWheel(delta,x,y,flags)
            || InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,2,delta,x,y);
    }
    bool OnButtonDown(double x,double y,int64_t flags) override {
        if(Base::OnButtonDown(x,y,flags)) return true;
        const auto button=Base::GetButtonID(flags);
        return button<2 && InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,button?6:3,x,y);
    }
    bool OnButtonUp(double x,double y,int64_t flags) override {
        const auto button=Base::GetButtonID(flags);
        if(button<2 && InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,button?7:4,x,y)) return true;
        return Base::OnButtonUp(x,y,flags);
    }
    bool OnButtonDblClk(double x,double y,int64_t flags) override {
        if(Base::OnButtonDblClk(x,y,flags)) return true;
        const auto button=Base::GetButtonID(flags);
        return button<2 && InvokeLegacySpriteCallback(this,LegacySpriteCallback::Mouse,button?8:5,x,y);
    }
    bool OnKeyDown(int64_t key,int64_t flags) override {
        return Base::OnKeyDown(key,flags)
            || InvokeLegacySpriteCallback(this,LegacySpriteCallback::Key,0,double(key));
    }
    bool OnKeyUp(int64_t key,int64_t flags) override {
        return Base::OnKeyUp(key,flags)
            || InvokeLegacySpriteCallback(this,LegacySpriteCallback::Key,1,double(key));
    }
};
