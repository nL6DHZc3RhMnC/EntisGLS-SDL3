#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_sprite.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include "runtime/cotopha_port/legacy_tone_filter.h"
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>
#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>
#include <sakuraglx/sprite/sglx_sprite_text.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <cstring>
#include <new>
#include <vector>
#include "platform/log.h"

namespace {
using namespace SakuraGL;
// The SDK Parameter constructor initializes the scalar fields and zoom, but
// S2DDVector/S3DDVector default constructors leave destination/centre untouched.
struct LegacySpriteParameter : SGLSprite::Parameter {
    LegacySpriteParameter(){vDst=S3DDVector(0,0,0);vCenter=S2DDVector(0,0);}
};
// GLS3 static text owns the declared text canvas even before it has glyphs.
// Modern SGLSpriteText::GetRectangle reports only its current glyph image;
// GetTextRectangle retains the real skin width/height and local transform.
bool LegacyItemRectangle(SGLSprite& owner,const wchar_t* id,SGLRect& bounds) {
    auto* item=id&&*id?owner.GetItemAs(id):&owner;
    bounds=SGLRect(0,0,-1,-1);if(!item)return false;
    auto* text=ESLTypeCast<SGLSpriteText>(item);
    bool found=false;
    if(text) {
        bounds=text->GetTextStyle().context.rectWritable;
        // UpdateTextImage stores glyph bearings/box alignment in vCenter.
        // Those move glyph pixels inside the old fixed text canvas, not the
        // canvas's layout bounds. Use its position/scale/rotation with a zero
        // canvas origin; never modify the live text sprite or its image.
        SGLSprite layout;
        auto parameter=text->GetParameter();parameter.vCenter=S2DDVector(0,0);
        layout.SetParameter(parameter);found=layout.LocalToGlobalRect(bounds);
    } else found=item->GetRectangle(bounds);
    if(!found)return false;
    while(item!=&owner) {
        auto* parent=item->GetParent();if(!parent)break;
        parent->LocalToGlobalRect(bounds);item=parent;
    }
    return true;
}
std::atomic<uint32_t> animationFlags{7};
struct GraphicsGuard { GraphicsGuard(){SSystem::Lock();} ~GraphicsGuard(){SSystem::Unlock();} };
class LegacyAlphaFilter final : public SGLSpriteFilter {
    std::unique_ptr<SGLImageObject> mask_;
    SGLImage filtered_,nativeFrame_,rgbaFrame_;
    SSystem::SSmartReference<SGLSprite> owner_;
    bool ready_=false;
    std::shared_ptr<LegacyToneState> tone_;
    uint64_t toneRevision_=UINT64_MAX;
public:
    DECLARE_CLASS_INFO(LegacyAlphaFilter,SGLSpriteFilter)
    uint32_t range=256,degree=0;
    LegacyAlphaFilter(SGLSprite& owner,std::unique_ptr<SGLImageObject> mask,uint32_t alphaRange,uint32_t blend,std::shared_ptr<LegacyToneState> tone={})
        :mask_(std::move(mask)),owner_(&owner),tone_(std::move(tone)),range(alphaRange),degree(blend){}
    bool UsesMaskDegree() const {return mask_ && range<256;}
    SGLImageObject* ResultImage() {return ready_?&filtered_:nullptr;}
    bool Active() const {return (UsesMaskDegree() && range>0 && degree>0)||(tone_&&HasLegacyToneEffect(*tone_));}
    void SetTone(std::shared_ptr<LegacyToneState> tone){tone_=std::move(tone);toneRevision_=UINT64_MAX;ready_=false;if(auto* owner=owner_.GetReference())owner->PostUpdate();}
    void OnTimer(SGLSprite& sprite,uint32_t) override {if(tone_&&toneRevision_!=tone_->revision){toneRevision_=tone_->revision;sprite.PostUpdate();}}
    void SetDegree(uint32_t value) {degree=value;if(auto* owner=owner_.GetReference())owner->PostUpdate();}
    bool IsDynamicDrawer() const override {return true;}
    void Filter(S3DRenderContextInterface&,SGLImageObject* image) override {
        ready_=false;
        if(!Active()||!image)return;
        SGLImageInfo info{};auto error=image->GetImageInfo(info);
        auto ensure=[&](SGLImage& target,uint32_t format,uint32_t depth) {
            SGLImageInfo current{};target.GetImageInfo(current);
            if(current.width==info.width&&current.height==info.height&&current.format==format&&current.depth==depth)return sglErrSuccess;
            return target.CreateImage(info.width,info.height,format,depth,SGLImageObject::bufferOnMemory);
        };
        const auto rgbaFormat=formatImageRGB|(info.format&(formatImageFlagAlpha|formatImageFlagNoProductOfAlpha));
        if(!error)error=ensure(nativeFrame_,info.format,info.depth);
        if(!error)error=ensure(rgbaFrame_,rgbaFormat,32);
        if(!error)error=ensure(filtered_,formatImageARGB|formatImageFlagNoProductOfAlpha,32);
        if(!error) {
            SGLImageBuffer from,middle,to;
            auto* read=nativeFrame_.LockBuffer(from),*rgba=rgbaFrame_.LockBuffer(middle),*write=filtered_.LockBuffer(to);
            from.ptrBuffer=read;middle.ptrBuffer=rgba;to.ptrBuffer=write;
            if(!read||!rgba||!write)error=sglErrFailed;
            if(!error)error=image->ReadFrameBuffer(from,read,image->GetSelectedFrame());
            if(!error)error=sglConvertImageBuffer(middle,from);
            if(!error)error=sglConvertImageBuffer(to,middle);
            if(read){const auto unlocked=nativeFrame_.UnlockBuffer();if(!error)error=unlocked;}
            if(rgba){const auto unlocked=rgbaFrame_.UnlockBuffer();if(!error)error=unlocked;}
            if(write){const auto unlocked=filtered_.UnlockBuffer();if(!error)error=unlocked;}
        }
        // GLS3 passes coefficient=range*16 to its 4-bit fixed-point routine.
        // Its intercept is 256-(range+1)*degree; GLS4 accepts the same integer
        // coefficient directly. Keep the source buffer intact between refreshes.
        if(!error&&tone_){SGLImageInfo info;image->GetImageInfo(info);error=SGLError(ApplyLegacyToneImage(filtered_,*tone_,(info.format&formatImageFlagAlpha)!=0));}
        if(!error&&UsesMaskDegree()&&range>0&&degree>0)error=filtered_.BlendWithAlphaChannel(mask_.get(),int32_t(range),256-int32_t((range+1)*degree));
        if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy alpha filter failed: %d",int(error));
        ready_=!error;
    }
    void Draw(S3DRenderContextInterface& render,const SGLPaintParam& parameters,SGLImageObject* image) override {
        if(auto* owner=owner_.GetReference())if(!owner->GetFrameBuffer())Filter(render,image);
        if(!Active())SGLSpriteFilter::Draw(render,parameters,image);
        else if(ready_)render.DrawImage(parameters,&filtered_);
    }
};
IMPLEMENT_CLASS_INFO(LegacyAlphaFilter,SGLSpriteFilter)
class LegacyAnimator final : public SGLSpriteAnimator {
    uint64_t elapsed_=0,duration_=0;
    size_t rewind_=0,end_=0,sequence_=0;
public:
    DECLARE_CLASS_INFO(LegacyAnimator, SGLSpriteAnimator)
    bool Running() const { return m_countLoop != 0 && m_pAnimation.Ptr(); }
    void SaveState(LegacySave::Writer& writer) const {
        writer.I32(int32_t(m_countLoop));writer.U32(sequence_);writer.U32(rewind_);
        writer.U32(end_?uint32_t(end_-1):0);writer.U32(duration_);
    }
    void Stop() { m_countLoop=0; }
    ESLError Start(SGLSprite &sprite,int loops,int first,int milliseconds,int rewind,int last) {
        auto *image=m_pAnimation.Ptr();
        if(!image) return eslErrGeneral;
        const size_t sequenceLength=image->GetSequenceLength();
        const size_t length=sequenceLength?sequenceLength:image->GetFrameCount();
        if(!length || rewind<0 || last<-1 || first<-1) return eslErrInvalidParam;
        end_=last<0?length:size_t(last)+1;
        if(end_>length || size_t(rewind)>=end_) return eslErrInvalidParam;
        if(first>=0) sequence_=size_t(first);
        if(sequence_>=end_) return eslErrInvalidParam;
        if(milliseconds<-1) return eslErrInvalidParam;
        duration_=milliseconds<0?image->GetTotalTime()*end_/length:uint64_t(milliseconds);
        m_countLoop=loops < 0 ? -1 : loops;
        rewind_=rewind;
        elapsed_=end_?sequence_*duration_/end_:0;
        SelectSequence(sprite,sequence_);
        return eslErrSuccess;
    }
    void SelectSequence(SGLSprite &sprite,size_t sequence) {
        auto *image=m_pAnimation.Ptr();
        if(!image) return;
        size_t frame=sequence;
        if(sequence<m_tableSeq.GetLength()) frame=m_tableSeq.At(sequence);
        image->SelectFrame(frame);
        AttachImageToSprite(sprite);
    }
    void OnAnimation(SGLSprite &sprite,uint32_t milliseconds) override {
        if(!Running()) return;
        if(!(animationFlags.load(std::memory_order_relaxed)&1)) {
            if(m_countLoop>=0) m_countLoop=0;
            return;
        }
        elapsed_+=milliseconds;
        if(elapsed_>=duration_) {
            if(m_countLoop>0) --m_countLoop;
            const uint64_t rewindTime=end_?rewind_*duration_/end_:0;
            if(m_countLoop!=0) elapsed_=duration_>rewindTime?(elapsed_-duration_)%(duration_-rewindTime)+rewindTime:rewindTime;
            else elapsed_=duration_?duration_-1:rewindTime;
        }
        const size_t next=duration_?size_t(elapsed_*end_/duration_):0;
        if(next!=sequence_) { sequence_=next;SelectSequence(sprite,sequence_); }
    }
};
IMPLEMENT_CLASS_INFO(LegacyAnimator, SGLSpriteAnimator)
class LegacyAction final : public SGLSpriteAction {
public:
    std::vector<uint32_t> durations;
    SSystem::SSmartReference<LegacyAlphaFilter> alphaFilter;
    std::shared_ptr<LegacySpriteBlendEffect> blendEffect;
    void CopyCurveState(const LegacyAction& from) {
        // Do not assign SGLObject's tracked identity while this action is live.
        m_typeAction=from.m_typeAction;m_flagPaused=from.m_flagPaused;
        m_msecPast=from.m_msecPast;m_msecStart=from.m_msecStart;m_msecDuration=from.m_msecDuration;
        m_maskSetElement=from.m_maskSetElement;m_maskModifyElement=from.m_maskModifyElement;
        m_bzPos=from.m_bzPos;m_bzCenter=from.m_bzCenter;m_bzZoom=from.m_bzZoom;
        m_bzAngle=from.m_bzAngle;m_bzTransparency=from.m_bzTransparency;
        m_bzFilterParam=from.m_bzFilterParam;m_bzFilterParam2=from.m_bzFilterParam2;
        durations=from.durations;alphaFilter=from.alphaFilter;blendEffect=from.blendEffect;
    }
    bool OnAction(SGLSprite::Parameter &param,uint32_t milliseconds) override {
        if(!(animationFlags.load(std::memory_order_relaxed)&2)) { OnFinish(param);return true; }
        return SGLSpriteAction::OnAction(param,milliseconds);
    }
    void EffectParameter(SGLSprite::Parameter &param,double t) override {
        if(t<1 && !durations.empty()) {
            double remaining=t*m_msecDuration;
            for(size_t i=0;i<durations.size();++i) {
                if(remaining<durations[i]) { t=(double(i)+remaining/durations[i])/durations.size();break; }
                remaining-=durations[i];
            }
        }
        const auto transparency=param.nTransparency;
        SGLSpriteAction::EffectParameter(param,t);
        if(blendEffect&&(m_maskSetElement&SGLSprite::flagParamTransparency)) {
            blendEffect->SetDegree(param.nTransparency);param.nTransparency=transparency;
        } else if(auto* filter=alphaFilter.GetReference())if(filter->UsesMaskDegree()&&(m_maskSetElement&SGLSprite::flagParamTransparency)) {
            filter->SetDegree(param.nTransparency);param.nTransparency=transparency;
        }
    }
};
uint32_t ToLegacyFlags(uint32_t flags) {
    uint32_t legacy=flags & 0xffff00f0u;
    if((flags&paintMaskFunction)!=paintFunctionMove) legacy|=1;
    else legacy&=~paintMaskFunction;
    if(flags&paintWithZOrder) legacy|=4;
    return legacy;
}
ESLError ToNativeFlags(uint32_t legacy,uint32_t &flags) {
    // Old glow maps an 8-bit mask between two colours. It is not equivalent
    // to the new Z-order bit at 0x2 and must never pass through unchanged.
    if(legacy&0x80) return ESLErrorMsg("Legacy shaped polygon drawing has not been ported");
    if(legacy&2) return ESLErrorMsg("Legacy grayscale glow drawing has not been ported");
    if(legacy&0x8000) return ESLErrorMsg("Legacy Sprite Z-scale projection has not been ported");
    if(legacy&0x3f08) return ESLErrorMsg("Unsupported legacy Sprite drawing flags");
    flags=legacy & 0xffff00b0u; // fixed-point positions are converted to doubles
    if(legacy&4) flags|=paintWithZOrder;
    if(!(legacy&1) && !(flags&paintMaskFunction)) flags|=paintNoBlendAlpha;
    return eslErrSuccess;
}
SGLRect ReadRect(ECSStructureInterface &source,int right=-1,int bottom=-1) {
    return SGLRect(source.GetMemberAsInt(L"left",0),source.GetMemberAsInt(L"top",0),
                   source.GetMemberAsInt(L"right",right),source.GetMemberAsInt(L"bottom",bottom));
}
ECSStructureInterface *MemberStruct(ECSStructure &source,const wchar_t *name) {
    return ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(source.GetMemberAs(name)));
}
}

struct LegacySpriteDrawState {
    std::unique_ptr<SGLImageObject> attachedImage;
    SSystem::SSmartReference<LegacyAnimator> animator;
    SSystem::SSmartReference<LegacyAlphaFilter> alphaFilter;
    std::shared_ptr<LegacySpriteBlendEffect> blendEffect;
    uint32_t blendDegree=0;
    std::shared_ptr<LegacyToneState> tone;
    LegacyAction pendingAction;
    SSystem::SSmartReference<SGLSpriteAction> activeAction;
    bool hasAction=false;
    uint32_t legacyFlags=0,dimColor=0,lightColor=0xffffff;
    bool flagsSet=false;
    double zScale=0;
    struct Visual {
        uint32_t functions=4,enabled=7,fillColor=0,alphaRange=256,blendDegree=0;
        bool fill=false,dynamic=false,visible=true;
        int priority=0;
        LegacySpriteParameter parameter;
        S3DVector screen{0,0,1024};
        EWideString id;
    } restored;
    std::array<int32_t,5> restoredAnimation{};
    bool restoreVisual=false,restoreAnimation=false;
};
namespace {
LegacySpriteDrawState &State(ECSSprite &sprite) {
    auto &ptr=sprite.LegacyDrawState();
    if(!ptr) ptr=std::make_shared<LegacySpriteDrawState>();
    return *ptr;
}
template<class T> void KeepRemainingCurve(SGLBezierCurves<T>& curve,double t) {
    if(curve.GetLength()<4)return;
    SGLBezierCurves<T> first,last;
    curve.DivideBezier(t,first,last);curve=last;
}
LegacyAction PrepareCurveEdit(LegacySpriteDrawState& state) {
    auto* active=static_cast<LegacyAction*>(state.activeAction.GetReference());
    LegacyAction edit(active?*active:state.pendingAction);
    // GLS3 DivideActivation keeps the original duration and duration list,
    // resets its elapsed time, and retains the remaining Bezier control points.
    // Loop/ping-pong actions are edited without subdivision in the old engine.
    if(active&&edit.m_typeAction==SGLSprite::actionOnce&&edit.m_msecDuration&&edit.m_msecPast) {
        const double t=double(std::min(edit.m_msecPast,edit.m_msecDuration))/edit.m_msecDuration;
        KeepRemainingCurve(edit.m_bzTransparency,t);KeepRemainingCurve(edit.m_bzPos,t);
        KeepRemainingCurve(edit.m_bzAngle,t);KeepRemainingCurve(edit.m_bzZoom,t);
        edit.m_msecPast=0;
    }
    return edit;
}
void CommitCurveEdit(LegacySpriteDrawState& state,const LegacyAction& edit) {
    state.pendingAction.CopyCurveState(edit);
    if(auto* active=static_cast<LegacyAction*>(state.activeAction.GetReference()))active->CopyCurveState(edit);
    state.hasAction=true;
}
ESLError ReadParameter(SGLSprite::Parameter &param,LegacySpriteDrawState &state,ECSStructure &source) {
    const uint32_t oldFlags=source.GetMemberAsInt(L"nFlags",state.flagsSet?state.legacyFlags:ToLegacyFlags(param.nFlags));
    uint32_t flags;
    const auto error=ToNativeFlags(oldFlags,flags);
    if(error) return error;
    const double fixed=(oldFlags&0x40)?65536.0:1.0;
    if(auto *point=MemberStruct(source,L"ptDstPos")) {
        param.vDst.x=point->GetMemberAsInt(L"x",std::llround(param.vDst.x*fixed))/fixed;
        param.vDst.y=point->GetMemberAsInt(L"y",std::llround(param.vDst.y*fixed))/fixed;
    }
    if(auto *point=MemberStruct(source,L"ptRevCenter")) {
        param.vCenter.x=point->GetMemberAsInt(L"x",param.vCenter.x);
        param.vCenter.y=point->GetMemberAsInt(L"y",param.vCenter.y);
    }
    param.nFlags=flags;
    param.vZoom.x=source.GetMemberAsReal(L"rHorzUnit",param.vZoom.x);
    param.vZoom.y=source.GetMemberAsReal(L"rVertUnit",param.vZoom.y);
    param.zAngle=source.GetMemberAsReal(L"rRevAngle",param.zAngle);
    param.xyCross=source.GetMemberAsReal(L"rCrossingAngle",param.xyCross);
    param.nTransparency=source.GetMemberAsInt(L"nTransparency",param.nTransparency);
    param.vDst.z=source.GetMemberAsReal(L"rZOrder",param.vDst.z);
    param.rgbColorParam=uint32_t(source.GetMemberAsInt(L"rgbColorParam1",param.rgbColorParam.ui32));
    if(param.nTransparency>256 || !std::isfinite(param.vZoom.x) || !std::isfinite(param.vZoom.y) ||
       !std::isfinite(param.zAngle) || !std::isfinite(param.xyCross) || !std::isfinite(param.vDst.z)) return eslErrInvalidParam;
    state.legacyFlags=oldFlags;state.flagsSet=true;
    state.dimColor=source.GetMemberAsInt(L"rgbDimColor",state.dimColor);
    state.lightColor=source.GetMemberAsInt(L"rgbLightColor",state.lightColor);
    state.zScale=source.GetMemberAsReal(L"rZScale",state.zScale);
    return eslErrSuccess;
}
void WriteParameter(ECSStructure &target,const SGLSprite::Parameter &param,const LegacySpriteDrawState &state) {
    const uint32_t flags=state.flagsSet?state.legacyFlags:ToLegacyFlags(param.nFlags);
    const double fixed=(flags&0x40)?65536.0:1.0;
    target.SetMemberAsInt(L"nFlags",flags);
    if(auto *point=MemberStruct(target,L"ptDstPos")) {
        point->SetMemberAsInt(L"x",std::llround(param.vDst.x*fixed));point->SetMemberAsInt(L"y",std::llround(param.vDst.y*fixed));
    }
    if(auto *point=MemberStruct(target,L"ptRevCenter")) {
        point->SetMemberAsInt(L"x",std::llround(param.vCenter.x));point->SetMemberAsInt(L"y",std::llround(param.vCenter.y));
    }
    target.SetMemberAsReal(L"rHorzUnit",param.vZoom.x);target.SetMemberAsReal(L"rVertUnit",param.vZoom.y);
    target.SetMemberAsReal(L"rRevAngle",param.zAngle);target.SetMemberAsReal(L"rCrossingAngle",param.xyCross);
    target.SetMemberAsReal(L"rZOrder",param.vDst.z);target.SetMemberAsInt(L"nTransparency",param.nTransparency);
    target.SetMemberAsInt(L"rgbDimColor",state.dimColor);target.SetMemberAsInt(L"rgbLightColor",state.lightColor);
    target.SetMemberAsInt(L"rgbColorParam1",param.rgbColorParam.ui32);target.SetMemberAsReal(L"rZScale",state.zScale);
}
bool CurveLength(size_t n) { return n==0 || n==1 || (n>=4 && (n-1)%3==0); }
ESLError ReadNumberCurve(SSystem::SArray<double> &values,ECSArray &array,double scale=1) {
    const size_t count=array.m_varArray.GetSize();
    if(!CurveLength(count)) return ESLErrorMsg("Bezier control point count must be 1 or 3n+1");
    values.SetLength(count);
    for(size_t i=0;i<count;++i) {
        auto *object=ECSObject::GetEntity(array.m_varArray.GetAt(i));
        if(auto *real=ESLTypeCast<ECSReal>(object)) values.SetAt(i,real->m_varReal*scale);
        else if(auto *integer=ESLTypeCast<ECSInteger>(object)) values.SetAt(i,integer->GetValue()*scale);
        else return ESLErrorMsg("Bezier control point is not numeric");
    }
    if(values.GetLength()==1) { const double value=values.At(0);values.SetLength(4);for(size_t i=0;i<4;++i)values.SetAt(i,value); }
    return eslErrSuccess;
}
}
void ResetLegacySpriteDrawState(ECSSprite &sprite,bool preserveTone) {
    GraphicsGuard lock;
    sprite.NativeSprite().SetSpriteImager(nullptr);
    sprite.NativeSprite().CancelAction();
    if(auto state=sprite.LegacyDrawState())if(auto* filter=state->alphaFilter.GetReference())sprite.NativeSprite().RemoveFilter(filter);
    sprite.LegacyDrawState().reset();
    if(preserveTone)if(auto* tone=ESLTypeCast<ECSToneFilter>(ECSObject::GetEntity(sprite.GetVariableAt(-6)))) {
        auto& state=State(sprite);state.tone=tone->ToneState();
        auto* filter=new LegacyAlphaFilter(sprite.NativeSprite(),nullptr,256,sprite.NativeSprite().GetTransparency(),state.tone);
        state.alphaFilter=filter;sprite.NativeSprite().AddSmartFilter(filter);
    }
}
void SetLegacySpriteBlendEffect(ECSSprite& object,std::shared_ptr<LegacySpriteBlendEffect> effect) {
    GraphicsGuard lock;auto& state=State(object);
    if(state.blendEffect)state.blendDegree=state.blendEffect->GetDegree();
    state.blendEffect=std::move(effect);state.pendingAction.blendEffect=state.blendEffect;
    if(auto* action=static_cast<LegacyAction*>(state.activeAction.GetReference()))action->blendEffect=state.blendEffect;
}
uint32_t GetLegacySpriteEffectDegree(ECSSprite& object) {
    GraphicsGuard lock;auto& state=State(object);
    return state.blendEffect?state.blendEffect->GetDegree():state.blendDegree;
}
SakuraGL::SGLImageObject* GetLegacySpriteFilteredImage(ECSSprite& object,SakuraGL::S3DRenderContextInterface& render,SakuraGL::SGLImageObject* image) {
    GraphicsGuard lock;auto& state=State(object);auto* filter=state.alphaFilter.GetReference();
    if(!filter)return image;
    if(state.blendEffect)filter->degree=state.blendEffect->GetDegree();
    if(!filter->Active())return image;
    filter->Filter(render,image);return filter->ResultImage();
}
bool LegacyEffectAnimationEnabled() {return (animationFlags.load(std::memory_order_relaxed)&4)!=0;}

ESLError HandleLegacySpriteDraw(ECSContext &context,ECSSprite &object,const wchar_t *method,
                               ECSObjArray<ECSObject> &args,bool &handled) {
    auto named=[method](const wchar_t *value){return method&&!std::wcscmp(method,value);};
    handled=named(L"AttachToneFilter")||named(L"FillRect")||named(L"DrawImage")||named(L"AttachImage")||named(L"SetAlphaImage")||named(L"GetParameter")||named(L"SetParameter")||
        named(L"CopyParameters")||named(L"GetBlendDegree")||named(L"SetBlendDegree")||named(L"SetBlendingEnvelope")||named(L"SetBezierCurve")||
        named(L"BeginActivation")||named(L"FlushActivation")||named(L"CancelActivation")||named(L"IsActivation")||
        named(L"ModifyAnimationFlags")||named(L"BeginAnimation")||named(L"EndAnimation")||named(L"IsDuringAnimation")||
        named(L"GetRectangle")||named(L"GetScreenPosition")||named(L"SetScreenPosition")||named(L"IsHitSprite")||
        named(L"GetSpriteAtPoint")||named(L"GetFocus")||named(L"SetFocus")||named(L"KillFocus")||named(L"MoveFocus")||
        named(L"SetCapture")||named(L"ReleaseCapture")||named(L"SendCommand")||named(L"DrawText")||
        named(L"GetVertScrollPos")||named(L"SetVertScrollPos")||named(L"GetVertScrollRange")||named(L"SetVertScrollRange")||
        named(L"GetHorzScrollPos")||named(L"SetHorzScrollPos")||named(L"GetHorzScrollRange")||named(L"SetHorzScrollRange");
    if(!handled) return eslErrSuccess;
    GraphicsGuard lock;
    auto &sprite=object.NativeSprite();
    auto push=[&context](INT64 value=0){return context.PushObject(new ECSInteger(value));};
    auto count=[&](int min,int max){return context.VerifyArgumentCount(args,min,max);};
    auto integer=[&](int &value,int i,int def=0){return context.GetArgumentAsInt(value,args,i,def);};
    ESLError error=eslErrSuccess;
    if(std::wcsstr(method,L"Scroll")) {
        const bool set=method[0]==L'S',vertical=std::wcsstr(method,L"Vert")!=nullptr;
        const bool range=std::wcsstr(method,L"Range")!=nullptr;
        if((error=count(set?2:1,set?3:2)))return error;
        ECSWideString id;if((error=context.GetArgumentAsStr(id,args,set?2:1,L"")))return error;
        auto* item=id.IsEmpty()?&sprite:sprite.GetItemAs(id);
        if(!item)return push(); // Original Get* returns zero for an absent skin item.
        const auto direction=vertical?SGLSprite::scrollVert:SGLSprite::scrollHorz;
        if(!set)return push(range?item->GetScrollRange(direction):item->GetScrollPos(direction));
        int value;if((error=integer(value,1)))return error;
        if(range)item->SetScrollRange(value,direction);else item->SetScrollPos(value,direction);
        object.RecordLegacyItemInteger(id,vertical?(range?L"vscrollrange":L"vscrollpos"):(range?L"hscrollrange":L"hscrollpos"),value);
        return push();
    }
    if(named(L"GetRectangle")) {
        if((error=count(1,2))) return error;
        ECSWideString id;if((error=context.GetArgumentAsStr(id,args,1,L"")))return error;
        SGLRect bounds;LegacyItemRectangle(sprite,id,bounds);
        auto *out=context.CreateUserStructure(L"Rect");
        if(!out)return ESLErrorMsg("Missing Rect class");
        out->SetMemberAsInt(L"left",bounds.left);out->SetMemberAsInt(L"top",bounds.top);
        out->SetMemberAsInt(L"right",bounds.right);out->SetMemberAsInt(L"bottom",bounds.bottom);
        return context.PushObject(*out);
    }
    if(named(L"GetScreenPosition")||named(L"SetScreenPosition")) {
        const bool set=named(L"SetScreenPosition");
        if((error=count(set?4:1,set?4:1)))return error;
        S3DVector screen;double scale=1,aspect=1;
        if(!sprite.GetProjectionScreen(screen,scale,aspect)) {
            const auto size=sprite.GetImageSize();screen=S3DVector(size.w/2.0,size.h/2.0,std::max(size.w,size.h));
        }
        if(set) {
            double x,y,z;
            if((error=context.GetArgumentAsReal(x,args,1,0))||(error=context.GetArgumentAsReal(y,args,2,0))||
               (error=context.GetArgumentAsReal(z,args,3,0)))return error;
            if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))return eslErrInvalidParam;
            sprite.SetProjectionScreen(S3DVector(x,y,z),scale,aspect);return push();
        }
        auto *out=context.CreateUserStructure(L"Vector");if(!out)return ESLErrorMsg("Missing Vector class");
        out->SetMemberAsReal(L"x",screen.x);out->SetMemberAsReal(L"y",screen.y);out->SetMemberAsReal(L"z",screen.z);
        return context.PushObject(*out);
    }
    if(named(L"IsHitSprite")||named(L"GetSpriteAtPoint")) {
        const bool hit=named(L"IsHitSprite");
        if((error=count(3,hit?4:3)))return error;
        int x,y;ECSWideString id;
        if((error=integer(x,1))||(error=integer(y,2))||(hit&&(error=context.GetArgumentAsStr(id,args,3,L""))))return error;
        S2DDVector position(x,y);
        if(!sprite.GlobalToLocal(position))return hit?push():context.PushObject(new ECSString);
        if(!hit) { auto *item=sprite.GetHitSpriteAt(position);return context.PushObject(new ECSString(item?static_cast<const wchar_t *>(item->GetID()):L"")); }
        auto *item=id.IsEmpty()?&sprite:sprite.GetItemAs(id);
        if(!item)return push();
        std::vector<SGLSprite*> path;
        for(auto *p=item;p&&p!=&sprite;p=p->GetParent())path.push_back(p);
        for(auto i=path.rbegin();i!=path.rend();++i)if(!(*i)->GlobalToLocal(position))return push();
        return push(item->IsHitSprite(position.x,position.y)?-1:0);
    }
    if(named(L"GetFocus")) {
        if((error=count(1,1)))return error;
        std::wstring path;auto *parent=&sprite;
        for(;;) {
            SGLSprite *focus=nullptr;
            for(size_t i=0;i<parent->GetChildCount();++i) { auto *child=parent->GetChildAt(i);if(child&&child->HasKeyFocus()){focus=child;break;} }
            if(!focus)break;
            const wchar_t *id=focus->GetID();
            if(id&&*id) {if(!path.empty())path+=L'\\';path+=id;}
            parent=focus;
        }
        return context.PushObject(new ECSString(path.c_str()));
    }
    if(named(L"SetFocus")||named(L"KillFocus")||named(L"SetCapture")||named(L"ReleaseCapture")) {
        if((error=count(1,2)))return error;
        ECSWideString id;if((error=context.GetArgumentAsStr(id,args,1,L"")))return error;
        auto *item=id.IsEmpty()?&sprite:sprite.GetItemAs(id);
        if(!item)return push(eslErrGeneral);
        if(named(L"SetFocus"))return push(item->SetKeyFocus());
        if(named(L"KillFocus"))return push(item->KillKeyFocus());
        if(named(L"SetCapture"))return push(item->SetMouseCapture());
        return push(item->ReleaseMouseCapture());
    }
    if(named(L"MoveFocus")) {
        if((error=count(1,2)))return error;
        int next;if((error=integer(next,1,1)))return error;
        if(next)sprite.MoveNextKeyFocus();else sprite.MovePrevKeyFocus();
        return push();
    }
    if(named(L"SendCommand")) {
        if((error=count(3,4)))return error;
        ECSWideString id,command;
        if((error=context.GetArgumentAsStr(id,args,1,L""))||(error=context.GetArgumentAsStr(command,args,2,L"")))return error;
        auto *item=id.IsEmpty()?&sprite:sprite.GetItemAs(id);
        if(!item)return push(eslErrGeneral);
        SSystem::SXMLDocument nativeResult;
        const auto nativeError=item->InvokeCommands(command,&nativeResult);
        if(nativeError) {
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Sprite.SendCommand rejected by native item id=%ls XML=%ls error=%d",
                id.CharPtr(),command.CharPtr(),int(nativeError));
            return ESLErrorMsg("Sprite.SendCommand XML was rejected by the native control");
        }
        auto *out=ESLTypeCast<ECSString>(context.GetArgumentObjectAs(args,3,L"String"));
        if(out) {
            SSystem::SString result;
            if(nativeResult.GetElementsCount()) nativeResult.FormatDocumentToString(result);
            out->m_varStr=static_cast<const wchar_t*>(result);
        }
        object.RecordLegacyItemString(id,L"command",command);
        return push();
    }
    if(named(L"DrawText")) {
        if((error=count(3,3)))return error;
        auto *parameters=ESLTypeCast<ECSStructure>(context.GetArgumentObjectAs(args,1,L"DrawTextParam"));
        if(!parameters)return ESLErrorMsg("DrawText requires DrawTextParam");
        ECSWideString text;if((error=context.GetArgumentAsStr(text,args,2,L"")))return error;
        auto *buffer=sprite.GetFrameBuffer();if(!buffer)return push();
        auto *area=MemberStruct(*parameters,L"rcArea");auto *position=MemberStruct(*parameters,L"ptCurPos");
        if(!area)return ESLErrorMsg("DrawTextParam.rcArea is missing");
        SGLLetteringContext layout;layout.rectWritable=ReadRect(*area);
        layout.ptStartWriting=SGLPoint(position?position->GetMemberAsInt(L"x",0):0,position?position->GetMemberAsInt(L"y",0):0);
        const int flags=parameters->GetMemberAsInt(L"nFlags",0);
        if(flags&~0x33)return ESLErrorMsg("Unsupported DrawText flags");
        layout.flagVertical=(flags&1)?SGLLetteringContext::writingVertical:SGLLetteringContext::writingHorizontal;
        layout.typeAlignment=(flags&0x30)==0x10?SGLLetteringContext::alignCenter:
            (flags&0x30)==0x20?SGLLetteringContext::alignRight:(flags&0x30)==0x30?SGLLetteringContext::alignLong:SGLLetteringContext::alignLeft;
        layout.pitchLine=parameters->GetMemberAsInt(L"nLineHeight",0);layout.widthIndent=parameters->GetMemberAsInt(L"nIndentWidth",0);
        ECSWideString face=parameters->GetMemberAsStr(L"strFontFace",SGLFontStyle::StandardFont);
        SGLFontStyle style;style.nSize=parameters->GetMemberAsInt(L"nFontSize",16);style.pszFace=face;
        if(flags&2)style.nStyles|=SGLFontStyle::styleNoSmooth;
        SGLFont font;if((error=static_cast<ESLError>(font.SetStyle(style))))return error;
        SGLLetterer letterer;
        const size_t consumed=letterer.WriteLetter(font,layout,text);
        SGLLetterer::Decoration decoration;decoration.rgbaBody=0xff000000u|(uint32_t(parameters->GetMemberAsInt(L"rgbColor",0))&0xffffffu);
        if((error=static_cast<ESLError>(letterer.DecorateLetter(decoration))))return error;
        const int transparency=parameters->GetMemberAsInt(L"nTransparency",0);
        if(transparency<0||transparency>256)return eslErrInvalidParam;
        SGLPaintContext paint;
        if((error=static_cast<ESLError>(paint.AttachTargetImage(buffer->GetImage(),buffer->GetZBuffer()))))return error;
        SGLAffine identity;paint.SetTransformation(identity,transparency);
        error=static_cast<ESLError>(letterer.DrawLetterTo(paint));paint.DetachTargetImage();
        if(error)return error;
        if(position){position->SetMemberAsInt(L"x",layout.ptStartWriting.x);position->SetMemberAsInt(L"y",layout.ptStartWriting.y);}
        sprite.NotifyUpdate();return push(consumed);
    }
    if(named(L"FillRect")) {
        if((error=count(3,5))) return error;
        auto *rectangle=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,1,L"Rect"));
        if(!rectangle) return ESLErrorMsg("FillRect requires Rect");
        int color,transparency,flags;
        if((error=integer(color,2))||(error=integer(transparency,3))||(error=integer(flags,4))) return error;
        if(transparency<0||transparency>256) return eslErrInvalidParam;
        auto *buffer=sprite.GetFrameBuffer();
        if(!buffer) return push(eslErrGeneral);
        const auto rect=ReadRect(*rectangle);
        if(rect.IsEmpty() || transparency==256) return push();
        uint32_t nativeFlags;
        if((error=ToNativeFlags(uint32_t(flags),nativeFlags))) return error;
        SGLPaintContext paint;
        if((error=static_cast<ESLError>(paint.AttachTargetImage(buffer->GetImage(),buffer->GetZBuffer())))) return push(error);
        SGLAffine affine;
        paint.SetTransformation(affine,uint32_t(transparency));
        error=static_cast<ESLError>(paint.FillRectangle(rect.left,rect.top,rect.GetWidth(),rect.GetHeight(),uint32_t(color),0,nativeFlags));
        paint.DetachTargetImage();sprite.NotifyUpdate();
        return push(error);
    }
    if(named(L"DrawImage")) {
        if((error=count(3,5))) return error;
        auto *resource=ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args,1,L"Resource"));
        auto *parameters=ESLTypeCast<ECSStructure>(context.GetArgumentObjectAs(args,2,L"SpriteParam"));
        if(!resource||!resource->GetImage()||!parameters) return ESLErrorMsg("DrawImage requires image Resource and SpriteParam");
        int frame;
        if((error=integer(frame,3))) return error;
        if(frame<0 || size_t(frame)>=resource->GetImage()->GetFrameCount()) return push(eslErrInvalidParam);
        auto *buffer=sprite.GetFrameBuffer();
        if(!buffer) return push(eslErrGeneral);
        std::unique_ptr<SGLImageObject> source(resource->GetImage()->NewReference(nullptr,frame));
        if(!source) return push(eslErrGeneral);
        auto *clip=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,4,L"Rect"));
        SGLImageRect sourceClip;
        if(clip) sourceClip=ReadRect(*clip);
        LegacySpriteParameter param;
        LegacySpriteDrawState temporary;
        if((error=ReadParameter(param,temporary,*parameters))) return error;
        SGLPaintParam paintParam;SGLAffine affine;
        paintParam.nFlags=param.nFlags;paintParam.nTransparency=param.nTransparency;
        paintParam.zOrder=param.vDst.z;paintParam.rgbColorParam=param.rgbColorParam;
        paintParam.SetAffine(affine,param.vDst.x,param.vDst.y,param.vCenter.x,param.vCenter.y,
                            param.vZoom.x,param.vZoom.y,param.zAngle,param.xyCross);
        SGLPaintContext paint;
        if((error=static_cast<ESLError>(paint.AttachTargetImage(buffer->GetImage(),buffer->GetZBuffer())))) return push(error);
        error=static_cast<ESLError>(paint.DrawImage(paintParam,source.get(),clip?&sourceClip:nullptr));
        paint.DetachTargetImage();sprite.NotifyUpdate();
        return push(error);
    }
    if(named(L"AttachImage")) {
        if((error=count(2,4))) return error;
        auto *resource=ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args,1,L"Resource"));
        if(!resource||!resource->GetImage()) return ESLErrorMsg("AttachImage requires an image Resource");
        int frame;
        if((error=integer(frame,2,-1))) return error;
        if(frame<-1 || (frame>=0&&size_t(frame)>=resource->GetImage()->GetFrameCount())) return push(eslErrInvalidParam);
        auto *clip=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,3,L"Rect"));
        SGLImageRect imageClip;
        if(clip) imageClip=ReadRect(*clip);
        std::unique_ptr<SGLImageObject> reference(resource->GetImage()->NewReference(clip?&imageClip:nullptr,frame));
        if(!reference) return push(eslErrGeneral);
        object.Release();
        auto &state=State(object);
        state.attachedImage=std::move(reference);
        if(frame<0) {
            auto *animator=new LegacyAnimator;
            animator->AttachImage(state.attachedImage.get());
            state.animator=animator;
            sprite.SetSpriteImager(animator);
        } else sprite.AttachImage(state.attachedImage.get());
        const auto legacyView=clip?ReadRect(*clip):SGLRect();
        object.RecordLegacyImageSource(resource,frame,clip?&legacyView:nullptr,context);
        return push();
    }
    auto &state=State(object);
    if(named(L"AttachToneFilter")) {
        if((error=count(1,2)))return error;
        auto* tone=ESLTypeCast<ECSToneFilter>(context.GetArgumentObjectAs(args,1,L"ToneFilter"));
        if(!tone&&ECSObject::GetEntity(args.GetAt(1)))return ESLErrorMsg("AttachToneFilter requires ToneFilter or null");
        static_cast<ECSReference*>(object.GetVariableAt(-6))->SetReference(tone,&context);
        state.tone=tone?tone->ToneState():nullptr;
        if(auto* filter=state.alphaFilter.GetReference())filter->SetTone(state.tone);
        else if(state.tone){auto* filter=new LegacyAlphaFilter(sprite,nullptr,256,sprite.GetTransparency(),state.tone);
            state.alphaFilter=filter;sprite.AddSmartFilter(filter);}
        sprite.PostUpdate();return push();
    }
    if(named(L"SetAlphaImage")) {
        if((error=count(3,3)))return error;
        auto* resource=ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args,1,L"Resource"));
        if(!resource)return ESLErrorMsg("SetAlphaImage requires Resource");
        int range;if((error=integer(range,2,256)))return error;
        auto* image=resource->GetImage();
        std::unique_ptr<SGLImageObject> mask;
        if(image) {
            SGLImageInfo info;
            if(image->GetImageInfo(info)||info.depth!=8)return ESLErrorMsg("Legacy alpha mask requires an 8-bit image");
            mask.reset(image->NewReference(nullptr,0));if(!mask)return push(eslErrGeneral);
            if(uint32_t(range)<=256) {
                const auto size=mask->GetImageSize();
                auto* buffer=sprite.GetFrameBuffer();
                SGLImageInfo current;
                const bool same=buffer&&!buffer->GetImage()->GetImageInfo(current)&&current.width==size.w&&current.height==size.h&&current.depth==32;
                if(!same || (current.format!=formatImageDefaultRGBA && !(current.format&0x04000000))) {
                    SGLImage preserved;
                    if(same&&(error=static_cast<ESLError>(preserved.CreateCloneImage(*buffer->GetImage()))))return push(error);
                    if((error=static_cast<ESLError>(sprite.CreateBuffer(size.w,size.h,formatImageDefaultRGBA,32))))return push(error);
                    if(same&&(error=static_cast<ESLError>(sprite.GetFrameBuffer()->GetImage()->CopyImage(&preserved))))return push(error);
                    object.RecordLegacyImageCreation(0x04000001,size.w,size.h);
                }
            }
        }
        if(auto* previous=state.alphaFilter.GetReference()) {
            state.blendDegree=previous->UsesMaskDegree()?previous->degree:sprite.GetTransparency();sprite.RemoveFilter(previous);
        }
        if(mask||state.tone) {
            auto* filter=new LegacyAlphaFilter(sprite,std::move(mask),uint32_t(range),state.blendDegree,state.tone);
            state.alphaFilter=filter;sprite.AddSmartFilter(filter);
        }
        static_cast<ECSReference*>(object.GetVariableAt(-3))->SetReference(image?resource:nullptr,&context);
        sprite.PostUpdate();return push();
    }
    if(named(L"GetParameter")||named(L"SetParameter")) {
        if((error=count(2,2))) return error;
        auto *parameters=ESLTypeCast<ECSStructure>(context.GetArgumentObjectAs(args,1,L"SpriteParam"));
        if(!parameters) return ESLErrorMsg("Sprite parameter requires SpriteParam");
        if(named(L"GetParameter")) { WriteParameter(*parameters,sprite.GetParameter(),state);return context.PushObject(new ECSReference(parameters)); }
        auto param=sprite.GetParameter();
        if((error=ReadParameter(param,state,*parameters))) return error;
        sprite.SetParameter(param);return push();
    }
    if(named(L"CopyParameters")) {
        if((error=count(2,2))) return error;
        auto *source=ESLTypeCast<ECSSprite>(context.GetArgumentObjectAs(args,1,L"Sprite"));
        if(!source) return ESLErrorMsg("CopyParameters requires Sprite");
        sprite.SetParameter(source->NativeSprite().GetParameter());
        const auto &sourceState=State(*source);
        state.legacyFlags=sourceState.legacyFlags;state.flagsSet=sourceState.flagsSet;
        state.dimColor=sourceState.dimColor;state.lightColor=sourceState.lightColor;state.zScale=sourceState.zScale;
        return push();
    }
    if(named(L"GetBlendDegree")||named(L"SetBlendDegree")) {
        const bool set=named(L"SetBlendDegree");
        if((error=count(set?2:1,set?2:1))) return error;
        auto* filter=state.alphaFilter.GetReference();
        if(!set) return push(state.blendEffect?state.blendEffect->GetDegree():filter&&filter->UsesMaskDegree()?filter->degree:sprite.GetTransparency());
        int degree;if((error=integer(degree,1))) return error;
        if(degree<0||degree>256) return eslErrInvalidParam;
        state.blendDegree=degree;
        if(state.blendEffect)state.blendEffect->SetDegree(degree);
        else if(filter&&filter->UsesMaskDegree())filter->SetDegree(degree);
        else {if(filter)filter->degree=degree;sprite.SetTransparency(degree);}
        return push();
    }
    if(named(L"ModifyAnimationFlags")) {
        if((error=count(1,3))) return error;
        int add,remove;
        if((error=integer(add,1))||(error=integer(remove,2))) return error;
        uint32_t old=animationFlags.load(),value;
        do { value=(old&~uint32_t(remove))|uint32_t(add); } while(!animationFlags.compare_exchange_weak(old,value));
        return push(value);
    }
    if(named(L"BeginAnimation")||named(L"EndAnimation")||named(L"IsDuringAnimation")) {
        auto *animator=state.animator.GetReference();
        if(named(L"IsDuringAnimation")) { if((error=count(1,1))) return error;return push(animator&&animator->Running()?-1:0); }
        if(named(L"EndAnimation")) { if((error=count(1,1))) return error;if(animator)animator->Stop();return push(); }
        if((error=count(1,6))) return error;
        int loops,first,milliseconds,rewind,last;
        if((error=integer(loops,1,1))||(error=integer(first,2))||(error=integer(milliseconds,3,-1))||
           (error=integer(rewind,4))||(error=integer(last,5,-1))) return error;
        if(!animator) return push(eslErrGeneral);
        return push(animator->Start(sprite,loops,first,milliseconds,rewind,last));
    }
    if(named(L"SetBlendingEnvelope")) {
        if((error=count(2,2))) return error;
        auto edit=PrepareCurveEdit(state);
        auto *curve=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,1,L"Array"));
        if(curve) {
            SSystem::SArray<double> values;
            if((error=ReadNumberCurve(values,*curve,256))) return error;
            if(values.GetLength()) edit.SetTransparencyCurve(values);
            else edit.m_maskSetElement&=~SGLSprite::flagParamTransparency;
        } else {
            int target;if((error=integer(target,1))) return error;
            if(target<0||target>256) return eslErrInvalidParam;
            auto* filter=state.alphaFilter.GetReference();
            if(state.blendEffect||(filter&&filter->UsesMaskDegree())) {
                const auto degree=state.blendEffect?state.blendEffect->GetDegree():filter->degree;
                SSystem::SArray<double> curve;curve.SetLength(4);
                for(int i=0;i<4;++i)curve.SetAt(i,double(degree)+(double(target)-degree)*i/3);
                edit.SetTransparencyCurve(curve);
            } else edit.SetTransparencyTo(sprite,target);
        }
        CommitCurveEdit(state,edit);return push();
    }
    if(named(L"SetBezierCurve")) {
        if((error=count(2,4))) return error;
        auto edit=PrepareCurveEdit(state);
        auto *positions=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,1,L"Array"));
        auto *angles=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,2,L"Array"));
        auto *zooms=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,3,L"Array"));
        if(positions) {
            const size_t n=positions->m_varArray.GetSize();if(!CurveLength(n)) return eslErrInvalidParam;
            SSystem::SArray<S3DDVector> values;values.SetLength(n);
            for(size_t i=0;i<n;++i) {
                auto *point=ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(positions->m_varArray.GetAt(i)));
                if(!point) return ESLErrorMsg("Position curve needs Point/Vector control points");
                values.SetAt(i,S3DDVector(point->GetMemberAsReal(L"x",0),point->GetMemberAsReal(L"y",0),
                                         point->GetMemberAsReal(L"z",sprite.GetPosition().z)));
            }
            if(n==1) {const auto value=values.At(0);values.SetLength(4);for(size_t i=0;i<4;++i) values.SetAt(i,value);}
            if(n) edit.SetBezierCurve(values);else edit.m_maskSetElement&=~SGLSprite::flagParamPos;
        }
        if(angles) {
            SSystem::SArray<double> values;if((error=ReadNumberCurve(values,*angles))) return error;
            if(values.GetLength()) edit.SetAngleCurve(values);
            else edit.m_maskSetElement&=~SGLSprite::flagParamAngle;
        }
        if(zooms) {
            const size_t n=zooms->m_varArray.GetSize();if(!CurveLength(n)) return eslErrInvalidParam;
            SSystem::SArray<S2DDVector> values;values.SetLength(n);
            for(size_t i=0;i<n;++i) {
                auto *point=ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(zooms->m_varArray.GetAt(i)));
                if(!point) return ESLErrorMsg("Zoom curve needs Point/Vector control points");
                values.SetAt(i,S2DDVector(point->GetMemberAsReal(L"x",0),point->GetMemberAsReal(L"y",0)));
            }
            if(n==1) {const auto value=values.At(0);values.SetLength(4);for(size_t i=0;i<4;++i) values.SetAt(i,value);}
            if(n) edit.SetZoomCurve(values);else edit.m_maskSetElement&=~SGLSprite::flagParamZoom;
        }
        CommitCurveEdit(state,edit);return push();
    }
    if(named(L"BeginActivation")) {
        if((error=count(2,3))) return error;
        int type;if((error=integer(type,2))) return error;
        if(type<0||type>2) return eslErrInvalidParam;
        auto action=std::make_unique<LegacyAction>(state.pendingAction);
        action->alphaFilter=state.alphaFilter.GetReference();action->blendEffect=state.blendEffect;
        action->durations.clear();
        auto *durations=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,1,L"Array"));
        uint64_t total=0;
        if(durations) {
            for(int i=0;i<durations->m_varArray.GetSize();++i) {
                auto *value=ESLTypeCast<ECSInteger>(ECSObject::GetEntity(durations->m_varArray.GetAt(i)));
                if(!value||value->GetValue()<0||uint64_t(value->GetValue())>UINT32_MAX) return eslErrInvalidParam;
                action->durations.push_back(uint32_t(value->GetValue()));total+=uint32_t(value->GetValue());
            }
        } else { int duration;if((error=integer(duration,1))) return error;if(duration<0)return eslErrInvalidParam;total=duration; }
        if(total>UINT32_MAX/2) return eslErrInvalidParam;
        sprite.CancelAction();
        action->m_msecPast=action->m_msecStart=0;action->m_flagPaused=false;
        action->SetActionType(type);action->SetDuration(uint32_t(total));
        auto param=sprite.GetParameter();action->EffectParameter(param,total?0:1);sprite.SetParameter(param);
        if(total&&state.hasAction) {state.activeAction=action.get();sprite.AddAction(action.release());}
        return push();
    }
    if(named(L"FlushActivation")||named(L"CancelActivation")||named(L"IsActivation")) {
        if((error=count(1,1))) return error;
        if(named(L"IsActivation")) return push(sprite.IsAction()?-1:0);
        if(named(L"FlushActivation")) sprite.FlushAction();else sprite.CancelAction();
        state.pendingAction=LegacyAction();state.hasAction=false;return push();
    }
    return ESLErrorMsg("Unhandled Sprite drawing method");
}

ESLError SaveLegacySpriteVisual(ESLFileObject& file,ECSSprite& object) {
    GraphicsGuard lock;LegacySave::Writer out{file};auto& state=State(object);auto& sprite=object.NativeSprite();
    const auto ui=sprite.GetUIFlag();
    uint32_t functions=4;
    const uint64_t nativeFlags[]={SGLSprite::uiFocusable,SGLSprite::uiGroupMember,SGLSprite::uiUnclickable,SGLSprite::uiModalFirst,SGLSprite::uiModalEnd};
    const uint32_t oldFlags[]={1,2,8,16,32};
    for(int i=0;i<5;++i)if(ui&nativeFlags[i])functions|=oldFlags[i];
    out.U32(functions);
    out.U32((sprite.IsEnabled()?1:0)|((ui&SGLSprite::uiDisabledKeyInput)?0:2)|((ui&SGLSprite::uiDisabledMouseWheel)?0:4));
    uint32_t color=0;const bool fill=sprite.GetFillBackColor(color);
    out.I32(fill);out.U32(color);out.I32(object.IsLegacyDynamicModeEnabled());out.I32(sprite.GetPriority());out.I32(sprite.IsVisible());
    const auto& param=sprite.GetParameter();
    const uint32_t flags=state.flagsSet?state.legacyFlags:ToLegacyFlags(param.nFlags);
    const double fixed=(flags&0x40)?65536.0:1.0;
    const double x=std::round(param.vDst.x*fixed),y=std::round(param.vDst.y*fixed);
    if(x<INT32_MIN||x>INT32_MAX||y<INT32_MIN||y>INT32_MAX)return eslErrInvalidParam;
    out.U32(flags);out.I32(int32_t(x));out.I32(int32_t(y));
    out.I32(int32_t(param.vCenter.x));out.I32(int32_t(param.vCenter.y));
    out.F32(param.vZoom.x);out.F32(param.vZoom.y);out.F32(param.zAngle);out.F32(param.xyCross);
    out.U32(state.dimColor);out.U32(state.lightColor);out.U32(param.nTransparency);
    out.F32(param.vDst.z);out.U32(param.rgbColorParam.ui32);out.F32(state.zScale);
    auto* alpha=state.alphaFilter.GetReference();
    out.U32(alpha?alpha->range:256);out.U32(state.blendEffect?state.blendEffect->GetDegree():alpha&&alpha->UsesMaskDegree()?alpha->degree:sprite.GetTransparency());
    S3DVector screen(0,0,1024);double scale=1,aspect=1;
    if(!sprite.GetProjectionScreen(screen,scale,aspect)) {
        const auto size=sprite.GetImageSize();if(size.w&&size.h)screen=S3DVector(size.w*0.5,size.h*0.5,std::min(size.w,size.h));
    }
    out.F32(screen.x);out.F32(screen.y);out.F32(screen.z);
    out.U32(0);out.U32(0); // No legacy per-buffer 3D draw/render flags are enabled by the bridge.
    out.String(sprite.GetID());
    out.U32(0); // Virtual camera is unsupported and cannot have been enabled successfully.
    out.F32(0);out.F32(0);out.F32(0);out.F32(0);out.F32(0);out.F32(1024);out.F32(0);
    out.U32(0);
    return out.error;
}

ESLError LoadLegacySpriteVisual(ESLFileObject& file,ECSSprite& object) {
    GraphicsGuard lock;LegacySave::Reader in{file};auto& state=State(object);auto& visual=state.restored;
    visual.functions=in.U32();visual.enabled=in.U32();visual.fill=in.I32()!=0;visual.fillColor=in.U32();
    visual.dynamic=in.I32()!=0;visual.priority=in.I32();visual.visible=in.I32()!=0;
    state.legacyFlags=in.U32();state.flagsSet=true;
    auto& param=visual.parameter;
    if(const auto error=ToNativeFlags(state.legacyFlags,param.nFlags))return error;
    const double fixed=(state.legacyFlags&0x40)?65536.0:1.0;
    param.vDst.x=in.I32()/fixed;param.vDst.y=in.I32()/fixed;
    param.vCenter.x=in.I32();param.vCenter.y=in.I32();
    param.vZoom.x=in.F32();param.vZoom.y=in.F32();param.zAngle=in.F32();param.xyCross=in.F32();
    state.dimColor=in.U32();state.lightColor=in.U32();param.nTransparency=in.U32();
    param.vDst.z=in.F32();param.rgbColorParam=SGLPalette(in.U32());state.zScale=in.F32();
    visual.alphaRange=in.U32();visual.blendDegree=in.U32();
    visual.screen.x=in.F32();visual.screen.y=in.F32();visual.screen.z=in.F32();
    const auto draw=in.U32(),render=in.U32();visual.id=in.String();const auto camera=in.U32();
    for(int i=0;i<7;++i)in.F32();in.Reserved();
    if(draw||render||camera)return ESLErrorMsg("Saved Sprite needs legacy 3D buffer/camera rendering that is not implemented");
    if((visual.functions&~63u)||(visual.enabled&~7u)||param.nTransparency>256)return eslErrInvalidParam;
    state.blendDegree=visual.blendDegree;state.restoreVisual=!in.error;
    return in.error;
}

ESLError SaveLegacySpriteAnimation(ESLFileObject& file,ECSSprite& object) {
    GraphicsGuard lock;LegacySave::Writer out{file};auto& state=State(object);
    if(auto* animator=state.animator.GetReference())animator->SaveState(out);
    else for(int i=0;i<5;++i)out.U32(0);
    const auto* active=static_cast<LegacyAction*>(state.activeAction.GetReference());
    const auto& action=active?*active:state.pendingAction;
    out.U32(active?action.m_msecPast:0);
    const bool fade=(action.m_maskSetElement&SGLSprite::flagParamTransparency)!=0;
    out.I32(fade);out.U32(action.m_bzTransparency.GetLength());
    for(size_t i=0;i<action.m_bzTransparency.GetLength();++i)out.F64(action.m_bzTransparency.At(i)/256.0);
    out.U32(0); // Legacy colour-fade curve is not implemented by this port.
    const bool move=(action.m_maskSetElement&(SGLSprite::flagParamPos|SGLSprite::flagParamAngle|SGLSprite::flagParamZoom))!=0;
    out.I32(move);out.U32(action.m_bzPos.GetLength());
    for(size_t i=0;i<action.m_bzPos.GetLength();++i){const auto& p=action.m_bzPos.At(i);out.F32(p.x);out.F32(p.y);out.F32(p.z);}
    out.U32(action.m_bzAngle.GetLength());for(size_t i=0;i<action.m_bzAngle.GetLength();++i)out.F64(action.m_bzAngle.At(i));
    out.U32(action.m_bzZoom.GetLength());for(size_t i=0;i<action.m_bzZoom.GetLength();++i){const auto& p=action.m_bzZoom.At(i);out.F32(p.x);out.F32(p.y);}
    out.I32(0);out.U32(0);out.U32(0);out.U32(0); // No camera movement/position/target/roll curves.
    out.U32(action.m_typeAction);out.U32(active?action.m_msecDuration:0);
    out.U32(action.durations.size());for(auto value:action.durations)out.U32(value);out.U32(0);
    return out.error;
}

ESLError LoadLegacySpriteAnimation(ESLFileObject& file,ECSSprite& object) {
    GraphicsGuard lock;LegacySave::Reader in{file};auto& state=State(object);auto& action=state.pendingAction;
    for(auto& field:state.restoredAnimation)field=in.I32();
    action.m_msecPast=in.U32();const bool fade=in.I32()!=0;
    auto count=in.Count();if(!CurveLength(count))return eslErrInvalidParam;
    SSystem::SArray<double> scalar;scalar.SetLength(count);
    for(size_t i=0;i<count;++i)scalar.SetAt(i,in.F64()*256.0);
    if(count==1){const auto value=scalar.At(0);scalar.SetLength(4);for(int i=0;i<4;++i)scalar.SetAt(i,value);}
    if(count&&fade)action.SetTransparencyCurve(scalar);
    if(in.Count()!=0)return ESLErrorMsg("Saved Sprite colour-fade curves have not been ported");
    const bool moving=in.I32()!=0;
    count=in.Count();if(!CurveLength(count))return eslErrInvalidParam;
    SSystem::SArray<S3DDVector> positions;positions.SetLength(count);
    for(size_t i=0;i<count;++i){S3DDVector p;p.x=in.F32();p.y=in.F32();p.z=in.F32();positions.SetAt(i,p);}
    if(count==1){const auto value=positions.At(0);positions.SetLength(4);for(int i=0;i<4;++i)positions.SetAt(i,value);}
    if(count&&moving)action.SetBezierCurve(positions);
    count=in.Count();if(!CurveLength(count))return eslErrInvalidParam;scalar.SetLength(count);
    for(size_t i=0;i<count;++i)scalar.SetAt(i,in.F64());
    if(count==1){const auto value=scalar.At(0);scalar.SetLength(4);for(int i=0;i<4;++i)scalar.SetAt(i,value);}
    if(count&&moving)action.SetAngleCurve(scalar);
    count=in.Count();if(!CurveLength(count))return eslErrInvalidParam;
    SSystem::SArray<S2DDVector> zooms;zooms.SetLength(count);
    for(size_t i=0;i<count;++i){S2DDVector p;p.x=in.F32();p.y=in.F32();zooms.SetAt(i,p);}
    if(count==1){const auto value=zooms.At(0);zooms.SetLength(4);for(int i=0;i<4;++i)zooms.SetAt(i,value);}
    if(count&&moving)action.SetZoomCurve(zooms);
    if(in.I32()||in.Count()||in.Count()||in.Count())return ESLErrorMsg("Saved Sprite camera animation has not been ported");
    action.m_typeAction=in.U32();action.m_msecDuration=in.U32();count=in.Count();action.durations.resize(count);
    uint64_t sum=0;for(auto& value:action.durations){value=in.U32();sum+=value;}in.Reserved();
    if(action.m_typeAction>2||action.m_msecDuration>UINT32_MAX/2||sum>UINT32_MAX/2||
       (count&&action.m_msecDuration&&sum!=action.m_msecDuration))return eslErrInvalidParam;
    state.hasAction=fade||moving;state.restoreAnimation=!in.error;return in.error;
}

void RestoreLegacySpriteVisual(ECSSprite& object) {
    GraphicsGuard lock;auto& state=State(object);if(!state.restoreVisual)return;
    auto& sprite=object.NativeSprite();const auto& visual=state.restored;
    uint64_t flags=sprite.GetUIFlag();
    const uint64_t nativeFlags[]={SGLSprite::uiFocusable,SGLSprite::uiGroupMember,SGLSprite::uiUnclickable,SGLSprite::uiModalFirst,SGLSprite::uiModalEnd};
    const uint32_t oldFlags[]={1,2,8,16,32};
    for(int i=0;i<5;++i){flags&=~nativeFlags[i];if(visual.functions&oldFlags[i])flags|=nativeFlags[i];}
    flags&=~(SGLSprite::uiDisabledKeyInput|SGLSprite::uiDisabledMouseWheel);
    if(!(visual.enabled&2))flags|=SGLSprite::uiDisabledKeyInput;if(!(visual.enabled&4))flags|=SGLSprite::uiDisabledMouseWheel;
    sprite.ModifyUIFlag(flags,~flags);sprite.SetEnable((visual.enabled&1)!=0);sprite.SetVisible(visual.visible);
    sprite.ChangePriority(visual.priority);sprite.SetParameter(visual.parameter);sprite.SetID(visual.id);
    object.EnableLegacyDynamicMode(visual.dynamic);
    sprite.SetFillBackColor(visual.fillColor,visual.fill);
    // SetFillBackColor only notifies a parent in GLS4. A newly restored plain
    // filled buffer needs its own first render even when it has no children.
    if(visual.fill)sprite.PostUpdate();
    if(visual.screen.z!=0)sprite.SetProjectionScreen(visual.screen);
}

ESLError CommitLegacySpriteDrawing(ECSContext& context,ECSSprite& object,ECSResource* source,
                                  int frame,const SGLRect& view,ECSResource* alpha) {
    GraphicsGuard lock;auto& state=State(object);auto& sprite=object.NativeSprite();
    SGLImageObject* image=source?source->GetImage():object.GetImage();
    if(source&&!image)return ESLErrorMsg("Saved Sprite image reference has no decoded image");
    if(image && (source||!sprite.GetFrameBuffer())) {
        if(frame<-1 || (frame>=0&&size_t(frame)>=image->GetFrameCount()))return eslErrInvalidParam;
        SGLImageRect clip=view;
        state.attachedImage.reset(image->NewReference(view.IsEmpty()?nullptr:&clip,frame));
        if(!state.attachedImage)return eslErrGeneral;
        if(frame<0) {
            auto* animator=new LegacyAnimator;animator->AttachImage(state.attachedImage.get());
            state.animator=animator;sprite.SetSpriteImager(animator);
            if(state.restoreAnimation) {
                const auto& values=state.restoredAnimation;
                if(const auto error=animator->Start(sprite,values[0],values[1],values[4],values[2],values[3]))return error;
            }
        }else sprite.AttachImage(state.attachedImage.get());
    } else if(state.restoreAnimation&&state.restoredAnimation[0]) {
        return ESLErrorMsg("Saved Sprite animation has no source image");
    }
    if(alpha) {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&object));args.Add(new ECSReference(alpha));
        args.Add(new ECSInteger(state.restored.alphaRange));bool handled=false;
        const auto error=HandleLegacySpriteDraw(context,object,L"SetAlphaImage",args,handled);
        if(error)return error;
        auto* returned=context.PopObject();INT64 result=0;
        const auto conversion=returned?returned->OperateInteger(result):eslErrGeneral;context.delete_CSObject(returned);
        if(conversion||result)return conversion?conversion:ESLError(result);
    }
    RestoreLegacySpriteVisual(object);
    if(auto* filter=state.alphaFilter.GetReference())filter->SetDegree(state.restored.blendDegree);
    if(state.restoreAnimation&&state.hasAction&&state.pendingAction.m_msecDuration) {
        auto* action=new LegacyAction(state.pendingAction);action->alphaFilter=state.alphaFilter.GetReference();action->blendEffect=state.blendEffect;
        state.activeAction=action;sprite.AddAction(action);
    }
    state.restoreAnimation=state.restoreVisual=false;
    return eslErrSuccess;
}

bool CheckLegacySpriteDraw() {
    auto check=[](bool ok,const char *stage) {
        if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Sprite draw probe FAIL: %s",stage);
        return ok;
    };
    ECSContext context;
    ECSSprite target,source,attached;
    auto invoke=[&](ECSSprite &sprite,const wchar_t *name,ECSObjArray<ECSObject> &args,INT64 expected=0) {
        bool handled=false;
        const auto status=HandleLegacySpriteDraw(context,sprite,name,args,handled);
        if(status||!handled) return false;
        std::unique_ptr<ECSObject> result(context.PopObject());
        auto *integer=ESLTypeCast<ECSInteger>(result.get());
        return integer&&integer->GetValue()==expected;
    };
    auto simple=[&](ECSSprite &sprite,const wchar_t *name,std::initializer_list<INT64> values,INT64 expected=0) {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&sprite));
        for(auto value:values)args.Add(new ECSInteger(value));
        return invoke(sprite,name,args,expected);
    };
    if(!check(!target.NativeSprite().CreateBuffer(4,4)&&!source.NativeSprite().CreateBuffer(2,2),"real image targets"))return false;
    {
        ECSSprite page;SGLSpriteText placeholder;
        SGLSpriteText::TextStyle style;style.context.rectWritable=SGLRect(0,0,1203,191);
        placeholder.SetTextStyle(style);placeholder.SetText(L"");placeholder.SetID(L"ID_MESSAGE");
        placeholder.SetPosition(155,66);page.NativeSprite().SetPosition(202,792);
        page.NativeSprite().AddChild(&placeholder);SGLRect bounds;
        const bool emptyFound=LegacyItemRectangle(page.NativeSprite(),L"ID_MESSAGE",bounds);
        const auto emptyBounds=bounds;
        const bool empty=emptyFound&&bounds.left==357&&bounds.top==858&&bounds.right==1560&&bounds.bottom==1049;
        placeholder.SetText(L"字");
        const bool textFound=LegacyItemRectangle(page.NativeSprite(),L"ID_MESSAGE",bounds);
        const auto centre=placeholder.GetParameter().vCenter;
        const bool withText=textFound&&bounds.left==357&&bounds.top==858&&bounds.right==1560&&bounds.bottom==1049;
        study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady",
            "Legacy text canvas rect: empty=%d:%d,%d,%d,%d populated=%d:%d,%d,%d,%d glyphCentre=%.1f,%.1f",
            emptyFound,emptyBounds.left,emptyBounds.top,emptyBounds.right,emptyBounds.bottom,
            textFound,bounds.left,bounds.top,bounds.right,bounds.bottom,centre.x,centre.y);
        page.NativeSprite().DetachChild(&placeholder);
        if(!check(empty&&withText,"empty and populated skin text retain declared canvas rectangle in parent coordinates"))return false;
    }
    ECSStructure rect;rect.m_pwszTag=L"Rect";
    rect.SetMemberAsInt(L"left",0);rect.SetMemberAsInt(L"top",0);rect.SetMemberAsInt(L"right",3);rect.SetMemberAsInt(L"bottom",3);
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));args.Add(new ECSReference(&rect));args.Add(new ECSInteger(0xff102030));
        if(!check(invoke(target,L"FillRect",args),"FillRect dispatch"))return false;
    }
    SGLPalette pixel;
    if(!check(target.GetImage()&&!target.GetImage()->GetPixelRGBA(pixel,3,3)&&pixel.ui32==0xff102030,"FillRect actual edge pixel"))return false;
    source.GetImage()->FillImage(SGLPalette(0xfff04020));
    ECSStructure param;param.m_pwszTag=L"SpriteParam";
    param.SetMemberAsInt(L"nFlags",1);param.SetMemberAsReal(L"rHorzUnit",1);param.SetMemberAsReal(L"rVertUnit",1);
    param.SetMemberAsReal(L"rCrossingAngle",90);param.SetMemberAsInt(L"nTransparency",0);
    auto *point=new ECSStructure;point->m_pwszTag=L"Point";point->SetMemberAsInt(L"x",1);point->SetMemberAsInt(L"y",1);
    param.AddNewVariable(L"ptDstPos",point);
    {
        alignas(LegacySpriteParameter) unsigned char poisoned[sizeof(LegacySpriteParameter)];
        std::memset(poisoned,0xa5,sizeof(poisoned));
        auto* defaults=new(poisoned) LegacySpriteParameter;LegacySpriteDrawState state;
        const auto error=ReadParameter(*defaults,state,param);
        const bool initialized=!error&&defaults->vDst.x==1&&defaults->vDst.y==1&&defaults->vDst.z==0&&
            defaults->vCenter.x==0&&defaults->vCenter.y==0&&defaults->vZoom.x==1&&defaults->vZoom.y==1&&
            defaults->zAngle==0&&defaults->xyCross==90&&defaults->rgbColorParam.ui32==0;
        defaults->~LegacySpriteParameter();
        if(!check(initialized,"poisoned sparse DrawImage parameters have zero position-Z/centre and explicit native defaults"))return false;
    }
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));args.Add(new ECSReference(&source));args.Add(new ECSReference(&param));
        if(!check(invoke(target,L"DrawImage",args),"DrawImage transform dispatch"))return false;
    }
    if(!check(!target.GetImage()->GetPixelRGBA(pixel,1,1)&&pixel.ui32==0xfff04020,"DrawImage actual source pixel"))return false;
    if(!check(!target.GetImage()->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xff102030,"DrawImage preserves outside pixels"))return false;
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&attached));args.Add(new ECSReference(&source));args.Add(new ECSInteger(0));
        if(!check(invoke(attached,L"AttachImage",args),"AttachImage owns reference"))return false;
    }
    source.Release();
    if(!check(attached.GetImage()&&!attached.GetImage()->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xfff04020,"attached image survives source Release"))return false;
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));args.Add(new ECSReference(&param));
        if(!check(invoke(target,L"SetParameter",args)&&target.NativeSprite().GetPosition().x==1,"SetParameter reaches native sprite"))return false;
    }
    {
        const auto original=target.NativeSprite().GetParameter();auto existing=original;existing.vCenter=S2DDVector(7,9);existing.vDst.z=13;
        target.NativeSprite().SetParameter(existing);
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));args.Add(new ECSReference(&param));
        if(!check(invoke(target,L"SetParameter",args)&&target.NativeSprite().GetParameter().vCenter.x==7&&
            target.NativeSprite().GetParameter().vCenter.y==9&&target.NativeSprite().GetParameter().vDst.z==13,
            "sparse SetParameter inherits existing centre and Z rather than fresh draw defaults"))return false;
        target.NativeSprite().SetParameter(original);
    }
    if(!check(simple(target,L"SetBlendDegree",{0})&&simple(target,L"SetBlendingEnvelope",{256})&&simple(target,L"BeginActivation",{100}),"begin real transparency action"))return false;
    target.NativeSprite().AdvanceTime(50);
    if(!check(target.NativeSprite().GetTransparency()>=120&&target.NativeSprite().GetTransparency()<=136&&target.NativeSprite().IsAction(),"half-time action interpolation"))return false;
    target.NativeSprite().AdvanceTime(50);
    if(!check(target.NativeSprite().GetTransparency()==256&&!target.NativeSprite().IsAction(),"action completion"))return false;
    if(!check(simple(target,L"SetBlendingEnvelope",{0})&&simple(target,L"BeginActivation",{100})&&simple(target,L"FlushActivation",{})&&target.NativeSprite().GetTransparency()==0,"FlushActivation reaches final value"))return false;
    {
        ECSSprite moving;
        auto positionCurve=[&](double from,double to) {
            auto* points=new ECSArray;
            for(int i=0;i<4;++i) {
                auto* point=new ECSStructure;point->m_pwszTag=L"Vector";
                point->SetMemberAsReal(L"x",from+(to-from)*i/3.0);
                point->SetMemberAsReal(L"y",0);point->SetMemberAsReal(L"z",0);
                points->m_varArray.Add(point);
            }
            ECSObjArray<ECSObject> args;args.Add(new ECSReference(&moving));args.Add(points);
            return invoke(moving,L"SetBezierCurve",args);
        };
        auto near=[](double a,double b){return std::abs(a-b)<1.01;};
        if(!check(positionCurve(0,100)&&simple(moving,L"SetBlendDegree",{0})&&
            simple(moving,L"SetBlendingEnvelope",{256})&&simple(moving,L"BeginActivation",{100}),
            "start concurrent real position and blend curves"))return false;
        moving.NativeSprite().AdvanceTime(50);
        if(!check(near(moving.NativeSprite().GetPosition().x,50)&&
            near(moving.NativeSprite().GetTransparency(),128)&&simple(moving,L"SetBlendingEnvelope",{0}),
            "retarget live blend at its actual midpoint"))return false;
        moving.NativeSprite().AdvanceTime(50);
        if(!check(near(moving.NativeSprite().GetPosition().x,75)&&
            near(moving.NativeSprite().GetTransparency(),64)&&moving.NativeSprite().IsAction(),
            "live blend update keeps the remaining movement curve and original duration"))return false;
        if(!check(positionCurve(75,25)&&simple(moving,L"BeginActivation",{100}),
            "retarget live movement while retaining the remaining fade"))return false;
        moving.NativeSprite().AdvanceTime(50);
        if(!check(near(moving.NativeSprite().GetPosition().x,50)&&near(moving.NativeSprite().GetTransparency(),32),
            "new movement and subdivided fade advance together"))return false;
        EMemoryFile animationWire;if(animationWire.Create(512))return false;
        if(!check(!SaveLegacySpriteAnimation(animationWire,moving),"save interrupted action"))return false;
        animationWire.Seek(0,ESLFileObject::FromBegin);ECSSprite resumed;
        resumed.NativeSprite().SetParameter(moving.NativeSprite().GetParameter());
        if(!check(!LoadLegacySpriteAnimation(animationWire,resumed)&&
            !CommitLegacySpriteDrawing(context,resumed,nullptr,0,SGLRect(),nullptr),
            "restore interrupted action using actual saved curves and clock"))return false;
        resumed.NativeSprite().AdvanceTime(50);
        if(!check(near(resumed.NativeSprite().GetPosition().x,25)&&resumed.NativeSprite().GetTransparency()==0&&
            !resumed.NativeSprite().IsAction(),"restored retargeted action reaches both endpoints"))return false;
    }
    SGLImage animation;
    SGLImageInfo info;info.width=2;info.height=2;info.depth=32;info.format=formatImageARGB;
    if(!check(!animation.CreateBuffer(info,SGLImageObject::bufferOnMemory,2,100),"two-frame native animation"))return false;
    animation.SelectFrame(0);animation.FillImage(SGLPalette(0xffff0000));
    animation.SelectFrame(1);animation.FillImage(SGLPalette(0xff0000ff));
    animation.SelectFrame(0);source.NativeSprite().AttachImage(&animation);
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&attached));args.Add(new ECSReference(&source));
        if(!check(invoke(attached,L"AttachImage",args),"attach actual frame sequence"))return false;
    }
    if(!check(simple(attached,L"BeginAnimation",{1,0,100})&&simple(attached,L"IsDuringAnimation",{},-1),"begin actual frame playback"))return false;
    attached.NativeSprite().AdvanceTime(60);
    if(!check(attached.GetImage()&&!attached.GetImage()->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xff0000ff,"animation selects second frame pixels"))return false;
    attached.NativeSprite().AdvanceTime(50);
    if(!check(simple(attached,L"IsDuringAnimation",{},0),"one-loop animation completion"))return false;
    if(!check(simple(attached,L"BeginAnimation",{-1,0,100})&&simple(attached,L"EndAnimation",{})&&
              simple(attached,L"IsDuringAnimation",{},0),"infinite loop explicit stop"))return false;
    ECSSprite textSprite;
    if(!check(!textSprite.NativeSprite().CreateBuffer(80,40),"DrawText buffer"))return false;
    ECSStructure textParam;textParam.m_pwszTag=L"DrawTextParam";
    auto *area=new ECSStructure;area->m_pwszTag=L"Rect";
    area->SetMemberAsInt(L"left",0);area->SetMemberAsInt(L"top",0);area->SetMemberAsInt(L"right",79);area->SetMemberAsInt(L"bottom",39);
    textParam.AddNewVariable(L"rcArea",area);
    auto *cursor=new ECSStructure;cursor->m_pwszTag=L"Point";cursor->SetMemberAsInt(L"x",0);cursor->SetMemberAsInt(L"y",0);
    textParam.AddNewVariable(L"ptCurPos",cursor);textParam.SetMemberAsInt(L"nFontSize",16);textParam.SetMemberAsInt(L"rgbColor",0xffffff);
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&textSprite));args.Add(new ECSReference(&textParam));args.Add(new ECSString(L"文A"));
        if(!check(invoke(textSprite,L"DrawText",args,2)&&cursor->GetMemberAsInt(L"x",0)>0,"DrawText consumes text and advances cursor"))return false;
    }
    bool hasInk=false;
    for(int y=0;y<40&&!hasInk;++y)for(int x=0;x<80;++x) {
        if(!textSprite.GetImage()->GetPixelRGBA(pixel,x,y)&&pixel.argb.Alpha&&pixel.argb.Red){hasInk=true;break;}
    }
    if(!check(hasInk,"legacy RGB font colour retains visible alpha"))return false;
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&textSprite));args.Add(new ECSString(L""));
        args.Add(new ECSString(L"<basic_flag hit_transparency=\"true\"/><input wheel=\"false\"/>"));
        if(!check(invoke(textSprite,L"SendCommand",args)&&
                  (textSprite.NativeSprite().GetUIFlag()&SGLSprite::uiUnclickable)&&
                  (textSprite.NativeSprite().GetUIFlag()&SGLSprite::uiDisabledMouseWheel),"actual original UI command flags"))return false;
    }
    {
        class RepeatButton final : public SGLSpriteButton {
        public:
            int repeats=0,clicks=0;
            void OnButtonPushed(bool repeat) override {if(repeat)++repeats;else ++clicks;}
        } button;
        button.SetID(L"SCROLL_UP");
        target.NativeSprite().AddChild(&button);
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));args.Add(new ECSString(L"SCROLL_UP"));
        args.Add(new ECSString(L"<push_repeat before_repeat=\"300\" interval=\"150\"/>"));
        if(!check(invoke(target,L"SendCommand",args),"native button push_repeat command"))return false;
        button.OnLButtonDown(0,0,0);button.AdvanceTime(299);
        if(!check(button.repeats==0,"repeat waits original 300ms"))return false;
        button.AdvanceTime(1);button.AdvanceTime(149);
        if(!check(button.repeats==1,"repeat waits original 150ms interval"))return false;
        button.AdvanceTime(1);button.OnLButtonUp(0,0,0);button.AdvanceTime(1000);
        if(!check(button.repeats==2&&button.clicks==1,"repeat emits real pushes and stops on release"))return false;
        target.NativeSprite().DetachChild(&button);
    }
    {
        SGLSpriteScrollBar scroll;
        scroll.SetID(L"SCROLL");target.NativeSprite().AddChild(&scroll);
        auto scrollCall=[&](const wchar_t* method,int value,bool set,int expected=0) {
            ECSObjArray<ECSObject> args;args.Add(new ECSReference(&target));
            if(set)args.Add(new ECSInteger(value));args.Add(new ECSString(L"SCROLL"));
            return invoke(target,method,args,expected);
        };
        if(!check(scrollCall(L"SetVertScrollRange",100,true)&&scrollCall(L"SetVertScrollPos",37,true)&&
            scrollCall(L"GetVertScrollRange",0,false,100)&&scrollCall(L"GetVertScrollPos",0,false,37),
            "legacy scroll range/position reach native scrollbar"))return false;
        target.NativeSprite().DetachChild(&scroll);
    }
    {
        ECSSprite mask,masked;
        if(!check(!mask.NativeSprite().CreateBuffer(3,1,formatImageGray,8),"real grayscale mask buffer"))return false;
        mask.GetImage()->SetPixelRGBA(0,0,SGLPalette(0xff000000));
        mask.GetImage()->SetPixelRGBA(1,0,SGLPalette(0xff808080));
        mask.GetImage()->SetPixelRGBA(2,0,SGLPalette(0xffffffff));
        {
            ECSObjArray<ECSObject> args;args.Add(new ECSReference(&masked));args.Add(new ECSReference(&mask));args.Add(new ECSInteger(1));
            if(!check(invoke(masked,L"SetAlphaImage",args),"SetAlphaImage creates real target with mask dimensions"))return false;
        }
        if(!check(masked.GetImage()&&masked.GetImage()->GetImageSize().w==3,"alpha target dimensions"))return false;
        masked.NativeSprite().SetFillBackColor(0,false);masked.GetImage()->FillImage(SGLPalette(0xffffffff));
        if(!check(simple(masked,L"SetBlendDegree",{128})&&simple(masked,L"GetBlendDegree",{},128)&&
            masked.NativeSprite().GetTransparency()==0,"mask degree is separate from overall transparency"))return false;
        auto* filter=State(masked).alphaFilter.GetReference();
        auto refresh=[&] {
            masked.NativeSprite().PostUpdate();masked.NativeSprite().PrepareDrawFrame();masked.NativeSprite().Refresh();
            return filter->ResultImage();
        };
        auto* output=refresh();
        if(!check(output&&!output->GetPixelRGBA(pixel,0,0)&&pixel.argb.Alpha==0,"black mask removes pixel"))return false;
        if(!check(!output->GetPixelRGBA(pixel,1,0)&&pixel.argb.Alpha==128,"middle mask produces half alpha"))return false;
        if(!check(!output->GetPixelRGBA(pixel,2,0)&&pixel.argb.Alpha==255,"white mask preserves pixel"))return false;
        output=refresh();
        if(!check(output&&!output->GetPixelRGBA(pixel,1,0)&&pixel.argb.Alpha==128&&
            !masked.GetImage()->GetPixelRGBA(pixel,1,0)&&pixel.argb.Alpha==255,
            "repeat refresh does not compound alpha or overwrite source"))return false;
        if(!check(simple(masked,L"SetBlendingEnvelope",{256})&&simple(masked,L"BeginActivation",{100}),
            "begin animated mask degree"))return false;
        masked.NativeSprite().AdvanceTime(50);
        if(!check(filter->degree>=190&&filter->degree<=194&&masked.NativeSprite().GetTransparency()==0,
            "mask envelope changes spatial alpha rather than uniform opacity"))return false;
        masked.NativeSprite().AdvanceTime(50);output=refresh();
        if(!check(output&&!output->GetPixelRGBA(pixel,2,0)&&pixel.argb.Alpha==0,
            "mask envelope final degree hides full image"))return false;
        mask.Release();
        if(!check(simple(masked,L"SetBlendDegree",{128})&&refresh()&&
            !filter->ResultImage()->GetPixelRGBA(pixel,1,0)&&pixel.argb.Alpha==128,
            "mask owns native pixels after source resource Release"))return false;
    }
    {
        ECSToneFilter tone;ECSSprite toned,mask;
        if(!check(!toned.NativeSprite().CreateBuffer(1,1,formatImageARGB,32),"tone native canvas"))return false;
        toned.NativeSprite().SetFillBackColor(0,false);toned.GetImage()->FillImage(SGLPalette(0xff123456));
        if(!check(!tone.SetGeneralTone({256,1,0,0,0,0,0,0}),"live tone source"))return false;
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&toned));args.Add(new ECSReference(&tone));
        if(!check(invoke(toned,L"AttachToneFilter",args),"real Sprite tone attachment"))return false;
        auto refresh=[&]{toned.NativeSprite().PostUpdate();toned.NativeSprite().PrepareDrawFrame();toned.NativeSprite().Refresh();return State(toned).alphaFilter.GetReference()->ResultImage();};
        auto* output=refresh();if(!check(output&&!output->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xffed3456,"native dynamic filter renders inverted pixel"))return false;
        output=refresh();if(!check(output&&!output->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xffed3456&&
            !toned.GetImage()->GetPixelRGBA(pixel,0,0)&&pixel.ui32==0xff123456,"tone refresh never compounds or mutates source"))return false;
        if(!check(!mask.NativeSprite().CreateBuffer(1,1,formatImageGray,8),"tone alpha mask"))return false;
        mask.GetImage()->FillImage(SGLPalette(0xff808080));
        ECSObjArray<ECSObject> alpha;alpha.Add(new ECSReference(&toned));alpha.Add(new ECSReference(&mask));alpha.Add(new ECSInteger(1));
        if(!check(invoke(toned,L"SetAlphaImage",alpha)&&simple(toned,L"SetBlendDegree",{128}),"compose tone and alpha in one native filter"))return false;
        output=refresh();if(!check(output&&!output->GetPixelRGBA(pixel,0,0)&&pixel.argb.Alpha==128,"combined tone and mask produces real half alpha"))return false;
        if(!check(ECSObject::GetEntity(toned.GetVariableAt(-6))==&tone,"tone uses original hidden reference index -6"))return false;
        toned.Release();
        if(!check(ECSObject::GetEntity(toned.GetVariableAt(-6))==&tone&&State(toned).tone==tone.ToneState(),"buffer Release preserves attached tone identity"))return false;
    }
    ResetLegacySpriteDrawState(target);ResetLegacySpriteDrawState(attached);
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy Sprite draw probe PASS: fill/draw pixels, transform, image ownership, actual timed action/flush and frame animation");
    return true;
}
