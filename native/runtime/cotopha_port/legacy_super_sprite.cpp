#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_super_sprite.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include "runtime/cotopha_port/legacy_tone_filter.h"
#include "runtime/cotopha_port/legacy_super_raster.h"
#include "runtime/cotopha_port/legacy_super_shading.h"
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include "platform/log.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <vector>

IMPLEMENT_CLASS_INFO(ECSSuperSprite, ECSSprite)
namespace {
struct EffectLock { EffectLock() { SSystem::Lock(); } ~EffectLock() { SSystem::Unlock(); } };
ESLError ValidateEffect(const std::array<uint32_t,33>& words) {
    if(words[0]==6)return LegacySuperRaster::Validate(words);
    if (words[0] > 5&&words[0]!=10&&words[0]!=11) return ESLErrorMsg("Saved SuperSprite effect is not implemented");
    if (words[0] == 1 && (!words[8] || !words[9] || words[8] > 8192 || words[9] > 8192 ||
        uint64_t(words[8]) * words[9] > 33554432 || words[2] > INT32_MAX))
        return eslErrInvalidParam;
    return eslErrSuccess;
}
}
// GLS3 draw_FilteredImage applies a tone table directly to destination pixels.
// It does not consume normal sprite transparency, the alpha mask, or its tone
// attachment. Keep a private result so a later frame never brightens twice.
class ECSSuperSprite::ColorEffect final : public LegacySpriteBlendEffect {
    SSystem::SSmartReference<SakuraGL::SGLSprite> owner_;
    uint32_t kind_,degree_=UINT32_MAX;
    ECSToneFilter tone_;
    SakuraGL::SGLImage result_,nativeFrame_,rgbaFrame_;
public:
    ColorEffect(SakuraGL::SGLSprite& owner,uint32_t kind,uint32_t degree):owner_(&owner),kind_(kind){SetDegree(degree);}
    uint32_t GetDegree() const override {return degree_;}
    void SetDegree(uint32_t degree) override {
        if(degree_==degree)return;degree_=degree;
        const int amount=std::clamp(int32_t(degree),-256,256)*(kind_>=4?-1:1);
        const int type=(kind_==3||kind_==5)?2:0;
        tone_.SetGeneralTone({amount,type,amount,type,amount,type,0,0});
        if(auto* owner=owner_.GetReference())owner->NotifyUpdate();
    }
    SakuraGL::SGLImageObject* ResultImage(){return &result_;}
    SakuraGL::SGLImageObject* Filter(SakuraGL::SGLImageObject* source) {
        using namespace SakuraGL;
        if(!source)return nullptr;
        SGLImageInfo info{};const char* stage="source-info";
        auto fail=[&](int error)->SGLImageObject* {
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
                "SuperSprite color filter failed stage=%s code=%d source=%ux%u format=%08x depth=%u frame=%zu degree=%u",
                stage,error,info.width,info.height,info.format,info.depth,source->GetSelectedFrame(),degree_);
            return nullptr;
        };
        auto error=source->GetImageInfo(info);if(error)return fail(error);
        const auto color=info.format&formatImageTypeMask;
        if(!info.width||!info.height||uint64_t(info.width)*info.height>0x4000000u)return fail(sglErrInvalidParam);
        if((color!=formatImageRGB&&color!=formatImageBGR&&color!=formatImageGray)||
           (info.format&(formatImageFlagSideBySide|formatImageFlagPalette|formatImageFlagClipping))||
           !info.depth||info.depth>32||(info.depth%8))return fail(eslErrNotSupported);
        auto ensure=[&](SGLImage& image,uint32_t format,uint32_t depth) {
            SGLImageInfo current{};image.GetImageInfo(current);
            if(current.width==info.width&&current.height==info.height&&current.format==format&&current.depth==depth)return sglErrSuccess;
            return image.CreateImage(info.width,info.height,format,depth,SGLImageObject::bufferOnMemory);
        };
        // CopyImage and ReadFrameBuffer do not convert formats in GLS4.
        // Read only the selected frame in its real layout, normalize channel
        // order/depth while retaining its alpha mode, then convert that mode.
        // This covers 24-bit ERI, ABGR surfaces and GPU-readable snapshots.
        stage="allocate-native";if((error=ensure(nativeFrame_,info.format,info.depth)))return fail(error);
        const auto rgbaFormat=formatImageRGB|(info.format&(formatImageFlagAlpha|formatImageFlagNoProductOfAlpha));
        stage="allocate-rgb";if((error=ensure(rgbaFrame_,rgbaFormat,32)))return fail(error);
        const auto outputFormat=formatImageARGB|formatImageFlagNoProductOfAlpha;
        stage="allocate-output";if((error=ensure(result_,outputFormat,32)))return fail(error);
        struct Lock {
            SGLImageObject& image;int flags;SGLImageBuffer buffer;uint8_t* pixels;
            Lock(SGLImageObject& value,int access):image(value),flags(access),pixels(image.LockBuffer(buffer,flags)){buffer.ptrBuffer=pixels;}
            ~Lock(){if(pixels)image.UnlockBuffer(flags);}
            SGLError Unlock(){if(!pixels)return sglErrFailed;pixels=nullptr;return image.UnlockBuffer(flags);}
        };
        {
            Lock from(nativeFrame_,SGLImageObject::lockRead|SGLImageObject::lockWrite);
            Lock intermediate(rgbaFrame_,SGLImageObject::lockRead|SGLImageObject::lockWrite);
            Lock to(result_,SGLImageObject::lockWrite);
            stage="lock-staging";if(!from.pixels||!intermediate.pixels||!to.pixels)return fail(sglErrFailed);
            stage="read-current";if((error=source->ReadFrameBuffer(from.buffer,from.pixels,source->GetSelectedFrame())))return fail(error);
            stage="convert-channels";if((error=sglConvertImageBuffer(intermediate.buffer,from.buffer)))return fail(error);
            stage="convert-alpha";if((error=sglConvertImageBuffer(to.buffer,intermediate.buffer)))return fail(error);
            stage="unlock";if((error=from.Unlock()))return fail(error);
            if((error=intermediate.Unlock()))return fail(error);if((error=to.Unlock()))return fail(error);
        }
        stage="tone";
        if((error=SGLError(ApplyLegacyToneImage(result_,*tone_.ToneState(),(info.format&formatImageFlagAlpha)!=0))))return fail(error);
        return &result_;
    }
};
class ECSSuperSprite::SuperNative final : public LegacyCallbackSprite<SakuraGL::SGLSpriteFormed> {
    ECSSuperSprite& owner_;
public:
    explicit SuperNative(ECSSuperSprite& owner) : owner_(owner) {}
    void AdvanceTime(uint32_t milliseconds) override {
        owner_.AdvanceEffect(milliseconds);
        LegacyCallbackSprite::AdvanceTime(milliseconds);
    }
    void Draw(SakuraGL::S3DRenderContextInterface& render,const Virtual3DParam* virtual3D=nullptr,
              Stereo3DView view=SakuraGL::SGLSprite::s3dMonoview) const override {
        EffectLock lock;
        if(owner_.colorEffect_){if(IsVisible())owner_.DrawColor(render);}
        else if(owner_.rasterEffect_||owner_.shadingEffect_) {
            if(!IsVisible())return;
            auto* source=GetLegacySpriteFilteredImage(owner_,render,owner_.NativeSprite().GetAttachedImage());
            if(!source)return;
            SakuraGL::SGLError error=SakuraGL::sglErrSuccess;
            if(owner_.rasterEffect_) {
                const auto& parameter=GetParameter();
                error=owner_.rasterEffect_->Draw(render,source,
                    int32_t(std::lround(parameter.vDst.x-parameter.vCenter.x)),
                    int32_t(std::lround(parameter.vDst.y-parameter.vCenter.y)));
            } else {
                auto* result=owner_.shadingEffect_->Filter(source);if(!result)return;
                SakuraGL::SGLPaintParam parameter;SakuraGL::SGLAffine affine;
                if(!GetPaintParam(parameter,affine,virtual3D,view))return;
                parameter.nTransparency=owner_.shadingEffect_->GetDrawTransparency(parameter.nTransparency);
                error=render.DrawImage(parameter,result);
            }
            if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","SuperSprite effect draw failed kind=%u code=%d",owner_.effectWords_[0],int(error));
        } else LegacyCallbackSprite::Draw(render,virtual3D,view);
    }
    bool GetRectangle(SakuraGL::SGLRect& rectangle) const override {
        EffectLock lock;
        if(owner_.rasterEffect_) {
            const auto& parameter=GetParameter();const auto size=GetImageSize();
            return owner_.rasterEffect_->GetRectangle(rectangle,
                int32_t(std::lround(parameter.vDst.x-parameter.vCenter.x)),
                int32_t(std::lround(parameter.vDst.y-parameter.vCenter.y)),size.w,size.h);
        }
        return LegacyCallbackSprite::GetRectangle(rectangle);
    }
    void PrepareDrawFrame() override {
        owner_.DrawEffect();
        LegacyCallbackSprite::PrepareDrawFrame();
    }
};
ECSSuperSprite::ECSSuperSprite() : ECSSprite(new SuperNative(*this)) {}
ECSSuperSprite::~ECSSuperSprite() { ClearEffect(); }
void ECSSuperSprite::ClearEffect() {
    EffectLock lock;
    SetLegacySpriteBlendEffect(*this,nullptr);colorEffect_.reset();rasterEffect_.reset();shadingEffect_.reset();
    if (tileSource_ && NativeSprite().GetAttachedImage() == tileImage_.get())
        NativeSprite().AttachImage(tileSource_.get());
    tile_ = false; tileImage_.reset(); dirty_ = false;
    elapsed_ = 0; scrollX_ = 0; scrollY_ = 0;
    width_ = height_ = interval_ = 0; speedX_ = speedY_ = 0;
    effectWords_.fill(0); effectImage_.SetReference(nullptr,nullptr); restoreEffect_ = false;
}
ESLError ECSSuperSprite::Release() {
    ClearEffect();
    const auto error = ECSSprite::Release();
    tileSource_.reset();
    return error;
}
ECSObject* ECSSuperSprite::GetVariableAt(int index) {
    return index == -11 ? &effectImage_ : ECSSprite::GetVariableAt(index);
}
void ECSSuperSprite::IndexAllMember() {
    ECSSprite::IndexAllMember();
    effectImage_.IndexAllMember(); effectImage_.m_pParent = this; effectImage_.m_nIndex = -11;
}
void ECSSuperSprite::CleanupAllReference(ECSContext& context) {
    EffectLock lock;effectImage_.CleanupAllReference(context);ECSSprite::CleanupAllReference(context);
}
ESLError ECSSuperSprite::Save(ESLFileObject& file, ECSContext& context) {
    EffectLock lock;
    if (restoreEffect_) return ESLErrorMsg("SuperSprite cannot save before effect restoration is committed");
    if (const auto error = ValidateEffect(effectWords_)) return error;
    if (tile_ && !m_image && !legacyReferences_[0].m_pRef)
        return ESLErrorMsg("SuperSprite tile image has no restorable Resource provenance");
    if (const auto error = ECSSprite::Save(file,context)) return error;
    LegacySave::Writer out{file};out.U32(132);
    for (size_t i=0;i<effectWords_.size();++i) out.U32(i==12?0:effectWords_[i]);
    out.Reference(effectImage_,context);return out.error;
}
ESLError ECSSuperSprite::Load(ESLFileObject& file, ECSContext& context) {
    EffectLock lock;
    if (const auto error = ECSSprite::Load(file,context)) return error;
    LegacySave::Reader in{file};const auto count=in.Count(132);
    std::array<uint8_t,132> bytes{};in.Bytes(bytes.data(),count);
    if (in.error) return in.error;
    std::array<uint32_t,33> words{};
    for(size_t i=0;i<words.size();++i)words[i]=StudySteadyLegacyWire::Read32(bytes.data()+i*4);
    if(const auto error=ValidateEffect(words))return error;
    // Win32 saved a process address here. These image effects never use it;
    // resources are restored exclusively through their actual ECS references.
    words[12]=0;
    in.Reference(effectImage_,context);if(in.error)return in.error;
    effectWords_=words;restoreEffect_=true;return eslErrSuccess;
}
void ECSSuperSprite::ConnectEffectDegree(const std::array<uint32_t,33>& words,uint32_t degree) {
    if(words[0]>=2&&words[0]<=5) {
        colorEffect_=std::make_shared<ColorEffect>(NativeSprite(),words[0],degree);
        SetLegacySpriteBlendEffect(*this,colorEffect_);
    } else if(words[0]==6) {
        rasterEffect_=std::make_shared<LegacySuperRaster>(NativeSprite(),words,degree);
        SetLegacySpriteBlendEffect(*this,rasterEffect_);
    } else if(words[0]==10||words[0]==11) {
        shadingEffect_=std::make_shared<LegacySuperShadingState>(NativeSprite(),words,degree);
        SetLegacySpriteBlendEffect(*this,shadingEffect_);
    }
    interval_=int32_t(words[2])>0?words[2]:0;
}
ESLError ECSSuperSprite::ApplyEffect(const std::array<uint32_t,33>& words,ECSResource* mask,ECSContext* context) {
    if(const auto error=ValidateEffect(words))return error;
    std::unique_ptr<SakuraGL::SGLImageObject> source;
    if(words[0]==1) {
        auto* image=tile_?tileSource_.get():NativeSprite().GetAttachedImage();
        if(!image||!image->GetImageWidth()||!image->GetImageHeight())
            return ESLErrorMsg("TileImage requires a restored attached image");
        source.reset(image->NewReference());if(!source)return eslErrGeneral;
    }
    // ClearEffect also clears the reference; protect a potentially owned mask
    // until it has been transferred to the new parameter state.
    ECSReference keep(mask);
    const auto degree=GetLegacySpriteEffectDegree(*this);
    ClearEffect();effectWords_=words;effectWords_[12]=0;
    effectImage_.SetReference(mask,context);
    if(words[0]==1) {
        tileSource_=std::move(source);tile_=true;dirty_=true;
        NativeSprite().AttachImage(tileSource_.get());
        width_=words[8];height_=words[9];interval_=words[2];
        speedX_=int32_t(words[10]);speedY_=int32_t(words[11]);
    }
    ConnectEffectDegree(words,degree);
    NativeSprite().NotifyUpdate();NativeSprite().Refresh();return eslErrSuccess;
}
ESLError ECSSuperSprite::CommitAllReference(ECSContext& context) {
    EffectLock lock;
    // SGLSprite::AddAction immediately evaluates the restored action at its
    // saved clock (UpdateAllActions(0)). Connect effect degree before the base
    // Commit starts that action; otherwise it would overwrite the separately
    // saved ordinary transparency with the degree curve's current value.
    if(restoreEffect_&&!colorEffect_&&!rasterEffect_&&!shadingEffect_)
        ConnectEffectDegree(effectWords_,GetLegacySpriteEffectDegree(*this));
    if(const auto error=ECSSprite::CommitAllReference(context))return error;
    if(!restoreEffect_)return eslErrSuccess;
    if(const auto error=effectImage_.CommitAllReference(context))return error;
    auto* entity=ECSObject::GetEntity(&effectImage_);
    auto* mask=ESLTypeCast<ECSResource>(entity);
    if(entity&&!mask)return ESLErrorMsg("Saved SuperSprite effect image is not a Resource");
    // Reapply the saved parameters, as GLS3 does. Interval remainder and scroll
    // positions are deliberately reset; neither exists in the original record.
    const auto words=effectWords_;
    return ApplyEffect(words,mask,&context);
}

// GLS3's constructor selects etNothing and draw_Normal calls ECSSprite::MTDraw.
// Therefore its ordinary image/buffer, child and animation behavior is exactly
// the real Sprite backend. Additional effects are enabled only as implemented.
const wchar_t* ECSSuperSprite::GetTypeName() const { return L"SuperSprite"; }
ECSObject* ECSSuperSprite::GetTypeOf(const wchar_t* type) {
    return !EWideString::Compare(type, L"SuperSprite") ? this : ECSSprite::GetTypeOf(type);
}
ECSObject* ECSSuperSprite::Duplicate() {
    EffectLock lock;
    auto* copy = new ECSSuperSprite;
    if (copy->CopySprite(*this)) { delete copy; return nullptr; }
    copy->effectWords_=effectWords_;
    copy->effectImage_.SetReference(effectImage_.m_pRef,nullptr);
    copy->legacyReferences_[0].SetReference(legacyReferences_[0].m_pRef,nullptr);
    copy->legacyImageFrame_=legacyImageFrame_;copy->legacyImageView_=legacyImageView_;
    if (tile_) {
        copy->tile_ = true; copy->dirty_ = true; copy->width_ = width_; copy->height_ = height_;
        copy->interval_ = interval_; copy->elapsed_ = elapsed_;
        copy->speedX_ = speedX_; copy->speedY_ = speedY_; copy->scrollX_ = scrollX_; copy->scrollY_ = scrollY_;
        if (tileSource_) copy->tileSource_.reset(tileSource_->NewReference());
        copy->NativeSprite().AttachImage(copy->tileSource_.get());
    }
    if(colorEffect_||rasterEffect_||shadingEffect_) {
        if(copy->ApplyEffect(effectWords_,ESLTypeCast<ECSResource>(ECSObject::GetEntity(&effectImage_)),nullptr)){delete copy;return nullptr;}
        const auto degree=GetLegacySpriteEffectDegree(*this);
        if(copy->colorEffect_)copy->colorEffect_->SetDegree(degree);
        if(copy->rasterEffect_)copy->rasterEffect_->SetDegree(degree);
        if(copy->shadingEffect_)copy->shadingEffect_->SetDegree(degree);
    }
    return copy;
}
ESLError ECSSuperSprite::GetFunction(ECSContext& context, int& index, const wchar_t* name) {
    if (!EWideString::Compare(name, L"SetEffectParameter")) { index = 6144; return eslErrSuccess; }
    if (!EWideString::Compare(name, L"SetMeshWarpEffect")) { index = 6145; return eslErrSuccess; }
    return ECSSprite::GetFunction(context, index, name);
}
ESLError ECSSuperSprite::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    if (index < 6144) return ECSSprite::CallFunction(context, index, args);
    if (index == 6145) return ESLErrorMsg("SuperSprite mesh warp effect is not connected");
    if (index != 6144) return ESLErrorMsg("Invalid SuperSprite method index");
    const auto error = context.VerifyArgumentCount(args, 2, 3);
    if (error) return error;
    auto* parameter = ESLTypeCast<ECSStructure>(context.GetArgumentObjectAs(args, 1, L"EffectParam"));
    if (!parameter) return ESLErrorMsg("SetEffectParameter requires EffectParam");
    const EWideString type = parameter->GetMemberAsStr(L"strType", L"");
    uint32_t kind=0;
    if(!type.CompareNoCase(L"TileImage"))kind=1;
    else if(!type.CompareNoCase(L"FilterWhite"))kind=2;
    else if(!type.CompareNoCase(L"FilterLight"))kind=3;
    else if(!type.CompareNoCase(L"FilterBlack"))kind=4;
    else if(!type.CompareNoCase(L"FilterDark"))kind=5;
    else if(!type.CompareNoCase(L"RasterScroll"))kind=6;
    else if(!type.CompareNoCase(L"ShadingOff"))kind=10;
    else if(!type.CompareNoCase(L"ShadingLight"))kind=11;
    else if(!type.IsEmpty()&&type.CompareNoCase(L"Nothing")) {
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","SuperSprite effect unavailable: %ls",type.CharPtr());
        return ESLErrorMsg("Requested SuperSprite effect is not connected");
    }
    std::array<uint32_t,33> words{};words[0]=kind;
    const wchar_t* integers[]={L"nFlags",L"nInterval",L"nDegreeStep",L"nShakingWidth",L"nMeshSize",L"nMeshDivision",L"nFrequency"};
    for(size_t i=0;i<7;++i)words[1+i]=uint32_t(parameter->GetMemberAsInt(integers[i],0));
    auto pair=[&](const wchar_t* name,size_t offset,const wchar_t* x,const wchar_t* y) {
        auto* value=ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(parameter->GetMemberAs(name)));
        if(value){words[offset]=uint32_t(value->GetMemberAsInt(x,0));words[offset+1]=uint32_t(value->GetMemberAsInt(y,0));}
    };
    pair(L"sizeView",8,L"w",L"h");pair(L"ptSpeed",10,L"x",L"y");
    words[13]=uint32_t(parameter->GetMemberAsInt(L"nAlphaRange",1));
    words[14]=uint32_t(parameter->GetMemberAsInt(L"nMilliSecPerDegree",1000));
    pair(L"ptSmashPoint",15,L"x",L"y");
    auto real=[&](size_t index,double value) {
        const float narrowed=float(value);if(!std::isfinite(narrowed))return false;
        std::memcpy(&words[index],&narrowed,4);return true;
    };
    const wchar_t* reals[]={L"rSmashDelay",L"rSmashPower",L"rRandomPower",L"rDeceleration"};
    for(size_t i=0;i<4;++i)if(!real(17+i,parameter->GetMemberAsReal(reals[i],0)))return eslErrInvalidParam;
    const wchar_t* vectors[]={L"vVelocity",L"vGravity",L"vRevSpeed",L"vRevRandom"};
    for(size_t i=0;i<4;++i) {
        auto* value=ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(parameter->GetMemberAs(vectors[i])));
        if(value)for(size_t j=0;j<3;++j)if(!real(21+3*i+j,value->GetMemberAsReal(j==0?L"x":j==1?L"y":L"z",0)))return eslErrInvalidParam;
    }
    auto* mask=ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args,2,L"Resource"));
    if(args.GetSize()>2&&ECSObject::GetEntity(args.GetAt(2))&&!mask)return eslErrInvalidParam;
    EffectLock lock;
    const auto applied=ApplyEffect(words,mask,&context);
    if(applied)return applied;
    return context.PushObject(context.new_CSInteger(0));
}

void ECSSuperSprite::AdvanceEffect(uint32_t milliseconds) {
    EffectLock lock;
    if(rasterEffect_){rasterEffect_->Advance(milliseconds,LegacyEffectAnimationEnabled());return;}
    auto* degreeEffect=colorEffect_?static_cast<LegacySpriteBlendEffect*>(colorEffect_.get()):static_cast<LegacySpriteBlendEffect*>(shadingEffect_.get());
    if(degreeEffect) {
        if(!interval_)return;
        elapsed_+=milliseconds;const auto steps=elapsed_/interval_;elapsed_%=interval_;
        if(!steps)return;
        if(!LegacyEffectAnimationEnabled()){degreeEffect->SetDegree(0);return;}
        int64_t degree=int64_t(int32_t(degreeEffect->GetDegree()))+int64_t(int32_t(effectWords_[3]))*int64_t(steps);
        if(degree>=768)degree=512+(degree-512)%256;
        degreeEffect->SetDegree(uint32_t(degree));return;
    }
    if (!tile_ || !tileSource_ || !interval_) return;
    elapsed_ += milliseconds;
    const auto steps = elapsed_ / interval_; elapsed_ %= interval_;
    if (!steps || !LegacyEffectAnimationEnabled()) return;
    auto wrap = [](int position, int speed, uint64_t steps, int size) {
        const int64_t delta = int64_t(speed) * int64_t(steps % uint64_t(size));
        const auto positive = ((int64_t(position) + delta) % size + size) % size;
        return int(positive ? positive - size : 0);
    };
    scrollX_ = wrap(scrollX_, speedX_, steps, tileSource_->GetImageWidth());
    scrollY_ = wrap(scrollY_, speedY_, steps, tileSource_->GetImageHeight());
    dirty_ = true; NativeSprite().Refresh();
}
void ECSSuperSprite::DrawColor(SakuraGL::S3DRenderContextInterface& render) {
    EffectLock lock;using namespace SakuraGL;
    if(!colorEffect_)return;
    auto* sourceImage=GetLegacySpriteFilteredImage(*this,render,NativeSprite().GetAttachedImage());
    auto* result=colorEffect_->Filter(sourceImage);if(!result)return;
    // Original color effects use only destination minus rotation centre and
    // copy RGBA through the LUT. Rotation, scaling and alpha blending belong
    // to the normal draw path, not GLS3 draw_FilteredImage.
    const auto& source=NativeSprite().GetParameter();SGLPaintParam param;SGLAffine affine;
    param.SetAffine(affine,source.vDst.x-source.vCenter.x,source.vDst.y-source.vCenter.y,0,0,1,1,0,90);
    param.nFlags=paintNoBlendAlpha;param.nTransparency=0;
    if(const auto error=render.DrawImage(param,result))
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","SuperSprite color draw failed code=%d position=%.1f,%.1f size=%ux%u",
            int(error),source.vDst.x-source.vCenter.x,source.vDst.y-source.vCenter.y,result->GetImageWidth(),result->GetImageHeight());
}
void ECSSuperSprite::DrawEffect() {
    EffectLock lock;
    if (!tile_ || !dirty_ || !tileSource_) return;
    using namespace SakuraGL;
    if (!tileImage_ || tileImage_->GetImageWidth() != width_ || tileImage_->GetImageHeight() != height_) {
        auto image = std::make_unique<SGLImage>();
        if (image->CreateImage(width_, height_, formatImageDefaultRGBA, 32)) return;
        tileImage_ = std::move(image);
    }
    tileImage_->FillImage(SGLPalette(uint32_t(0)));
    SGLPaintContext paint;
    if (paint.AttachTargetImage(tileImage_.get(), nullptr)) return;
    const int w = tileSource_->GetImageWidth(), h = tileSource_->GetImageHeight();
    SGLPaintParam parameter;
    parameter.nFlags = paintNoBlendAlpha;
    bool success = true;
    for (int y = scrollY_; y < int(height_) && success; y += h) for (int x = scrollX_; x < int(width_); x += w) {
        SGLAffine affine;
        parameter.SetAffine(affine, x, y, 0, 0, 1, 1, 0, 90);
        if (paint.DrawImage(parameter, tileSource_.get())) { success = false; break; }
    }
    paint.DetachTargetImage();
    if (success) {
        NativeSprite().AttachImage(tileImage_.get());
        dirty_ = false;
    } else study::platform::LogWrite(study::platform::LogPriority::Error, "StudySteady", "SuperSprite tile draw failed");
}

bool CheckLegacySuperSprite(ECSEnvironment& environment) {
    using namespace SakuraGL;
    auto check = [](bool value, const char* detail) {
        if (!value) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "SuperSprite probe FAIL: %s", detail);
        return value;
    };
    auto source = std::make_unique<SGLImage>();
    if (source->CreateImage(2, 2, formatImageDefaultRGBA, 32)) return false;
    source->FillImage(SGLPalette(0xff102030));
    source->SetPixelRGBA(1, 0, SGLPalette(0xfff02040));
    source->SetPixelRGBA(0, 1, SGLPalette(0xff2050f0));
    source->SetPixelRGBA(1, 1, SGLPalette(0xff40f080));
    ECSSuperSprite sprite;
    sprite.tileSource_.reset(source->NewReference()); source.reset();
    sprite.tile_ = true; sprite.dirty_ = true; sprite.width_ = 5; sprite.height_ = 3;
    sprite.interval_ = 16; sprite.speedX_ = sprite.speedY_ = -1;
    sprite.NativeSprite().PrepareDrawFrame();
    auto pixel = [&](int x, int y) {
        SGLPalette value; auto* image = sprite.NativeSprite().GetAttachedImage();
        return image && !image->GetPixelRGBA(value, x, y) ? value.ui32 : 0;
    };
    if (!check(pixel(4, 2) == 0xff102030 && pixel(3, 1) == 0xff40f080, "repeated pixels and clipped viewport edge")) return false;
    sprite.NativeSprite().AdvanceTime(15); sprite.NativeSprite().PrepareDrawFrame();
    if (!check(pixel(0, 0) == 0xff102030, "interval boundary holds before scroll")) return false;
    sprite.NativeSprite().AdvanceTime(1); sprite.NativeSprite().PrepareDrawFrame();
    if (!check(pixel(0, 0) == 0xff40f080 && pixel(4, 2) == 0xff40f080, "16ms diagonal wrap")) return false;
    sprite.NativeSprite().AdvanceTime(32); sprite.NativeSprite().PrepareDrawFrame();
    if (!check(pixel(0, 0) == 0xff40f080, "multiple interval modulo")) return false;
    sprite.Release();
    class StateImage final:public ECSExecutionImage {
    public:
        StateImage(){
            const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
            std::memset(&m_exiHeader,0,sizeof(m_exiHeader));m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
            m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
            m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
            std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
            m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
        }
    } image;
    image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSExecutionImage& image;ECSContext* previous;
        ~Scope(){ShutdownLegacySpriteCallbacksForImage(image);context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } scope{context,image,previous};
    auto status=[&](ESLError error,const char* detail){
        if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","SuperSprite probe %s: %s",detail,GetESLErrorMsg(error));
        return check(!error,detail);
    };
    if(!status(context.InitializeContext(&image),"isolated serialization context"))return false;
    EffectLock lock;
    auto call=[&](ECSSuperSprite& object,const wchar_t* method,std::initializer_list<ECSObject*> values){
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&object));for(auto* value:values)args.Add(value);
        int index=0;auto error=object.GetFunction(context,index,method);
        if(!error)error=object.CallFunction(context,index,args);
        auto* result=!error?context.PopObject():nullptr;INT64 returned=-1;
        if(!error)error=result?result->OperateInteger(returned):eslErrGeneral;
        context.delete_CSObject(result);
        if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","SuperSprite probe call %ls: %s",method,GetESLErrorMsg(error));
        return !error&&!returned;
    };
    auto named=[](ECSGlobal& root,const wchar_t* name){int index=0;
        return root.GetVariableIndex(index,name)?nullptr:ECSObject::GetEntity(root.GetVariableAt(index));};
    auto inspect=[&](EMemoryFile& file,std::array<uint32_t,33>& words,size_t& offset){
        file.SeekLarge(0,ESLFileObject::FromBegin);ECSSprite prefix;
        if(!status(prefix.Load(file,context),"inspect original Sprite base"))return false;
        offset=file.GetPosition();LegacySave::Reader in{file};
        if(in.U32()!=132)return false;
        for(auto& word:words)word=in.U32();ECSReference mask;in.Reference(mask,context);
        return !in.error&&file.GetPosition()==file.GetLength();
    };
    auto* scene=new ECSSprite;image.m_csgData.AddVariable(L"scene",scene);
    auto* resource=new ECSResource;image.m_csgGlobal.AddVariable(L"source",resource);
    if(!status(resource->LoadImageFile(L"particle_light1.eri",&context),"actual NOA tile source"))return false;
    auto* tiled=new ECSSuperSprite;image.m_csgGlobal.AddVariable(L"tiled",tiled);
    auto* nothing=new ECSSuperSprite;image.m_csgGlobal.AddVariable(L"nothing",nothing);
    if(!status(ESLError(nothing->NativeSprite().CreateBuffer(32,16)),"etNothing real Sprite buffer"))return false;
    const auto sourceWidth=resource->GetImage()->GetImageWidth(),sourceHeight=resource->GetImage()->GetImageHeight();
    if(!check(sourceWidth>=2&&sourceHeight>=2,"actual tile source dimensions"))return false;
    const int cx=int(sourceWidth)/2-1,cy=int(sourceHeight)/2-1;
    ECSStructure clip;clip.m_pwszTag=L"Rect";
    clip.SetMemberAsInt(L"left",cx);clip.SetMemberAsInt(L"top",cy);
    clip.SetMemberAsInt(L"right",cx+1);clip.SetMemberAsInt(L"bottom",cy+1);
    if(!check(call(*tiled,L"AttachImage",{new ECSReference(resource),new ECSInteger(0),new ECSReference(&clip)}),"script attaches real cropped image"))return false;
    ECSStructure parameter;parameter.m_pwszTag=L"EffectParam";
    parameter.AddNewVariable(L"strType",new ECSString(L"TileImage"));
    parameter.SetMemberAsInt(L"nInterval",16);parameter.SetMemberAsInt(L"nFlags",2);
    auto* size=new ECSStructure;size->m_pwszTag=L"Size";size->SetMemberAsInt(L"w",5);size->SetMemberAsInt(L"h",3);
    parameter.AddNewVariable(L"sizeView",size);
    auto* speed=new ECSStructure;speed->m_pwszTag=L"Point";speed->SetMemberAsInt(L"x",-1);speed->SetMemberAsInt(L"y",-1);
    parameter.AddNewVariable(L"ptSpeed",speed);
    if(!check(call(*tiled,L"SetEffectParameter",{new ECSReference(&parameter),new ECSReference(resource)}),"actual TileImage parameter and resource reference"))return false;
    scene->NativeSprite().AddChild(&tiled->NativeSprite());scene->NativeSprite().AddChild(&nothing->NativeSprite());
    tiled->NativeSprite().SetID(L"SAVED_TILE");tiled->NativeSprite().SetPosition(7,11);
    tiled->NativeSprite().AdvanceTime(21);tiled->NativeSprite().PrepareDrawFrame();
    if(!check(tiled->elapsed_==5&&tiled->scrollX_==-1&&tiled->scrollY_==-1,"live tile scroll before save"))return false;
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();
    EMemoryFile individual;std::array<uint32_t,33> words{};size_t effectOffset=0;
    if(!status(individual.Create(1024),"wire buffer")||!status(tiled->Save(individual,context),"original SuperSprite Save")||
       !check(inspect(individual,words,effectOffset)&&words[0]==1&&words[1]==2&&words[2]==16&&words[8]==5&&words[9]==3&&
           int32_t(words[10])==-1&&int32_t(words[11])==-1&&words[12]==0&&words[13]==1&&words[14]==1000,
           "132-byte Win32 parameter layout and zero process pointer"))return false;
    std::vector<uint8_t> original(static_cast<const uint8_t*>(individual.GetBuffer()),static_cast<const uint8_t*>(individual.GetBuffer())+individual.GetLength());
    EMemoryFile graph;if(!status(graph.Create(2048),"graph buffer")||!status(image.m_csgGlobal.Save(graph,context),"save real tile and etNothing globals"))return false;
    const auto graphBytes=graph.GetLength();
    image.m_csgGlobal.CleanupAllReference(context);image.m_csgGlobal.RemoveAllVariable();
    if(!check(scene->NativeSprite().GetChildCount()==0,"destroy original graph and detach actual sprites"))return false;
    graph.SeekLarge(0,ESLFileObject::FromBegin);
    if(!status(image.m_csgGlobal.Load(graph,context),"factory recreate SuperSprite and reopen resource")||
       !status(image.m_csgGlobal.CommitAllReference(context),"commit base and effect resource references"))return false;
    tiled=ESLTypeCast<ECSSuperSprite>(named(image.m_csgGlobal,L"tiled"));
    nothing=ESLTypeCast<ECSSuperSprite>(named(image.m_csgGlobal,L"nothing"));
    resource=ESLTypeCast<ECSResource>(named(image.m_csgGlobal,L"source"));
    if(!check(tiled&&nothing&&resource&&tiled->tile_&&!nothing->tile_&&tiled->elapsed_==0&&tiled->scrollX_==0&&tiled->scrollY_==0&&
       ECSObject::GetEntity(tiled->GetVariableAt(-2))==resource&&ECSObject::GetEntity(tiled->GetVariableAt(-11))==resource&&
       tiled->NativeSprite().GetParent()==&scene->NativeSprite()&&scene->NativeSprite().GetChildCount()==2&&
       nothing->NativeSprite().GetFrameBuffer()&&nothing->NativeSprite().GetFrameBuffer()->GetImage()->GetImageWidth()==32,
       "same static parent, real image references, etNothing buffer, original reset-on-load semantics"))return false;
    auto tilePixel=[&](int x,int y){SGLPalette value;auto* current=tiled->NativeSprite().GetAttachedImage();
        return current&&!current->GetPixelRGBA(value,x,y)?value.ui32:0;};
    SGLPalette expected;
    if(!status(ESLError(resource->GetImage()->GetPixelRGBA(expected,cx,cy)),"actual source pixel"))return false;
    tiled->NativeSprite().PrepareDrawFrame();
    if(!check(tilePixel(0,0)==expected.ui32&&tilePixel(4,2)==expected.ui32,"restored TileImage renders exact real source pixels"))return false;
    tiled->NativeSprite().AdvanceTime(15);
    if(!check(tiled->scrollX_==0&&tiled->elapsed_==15,"restored interval holds before boundary"))return false;
    tiled->NativeSprite().AdvanceTime(1);tiled->NativeSprite().PrepareDrawFrame();
    resource->GetImage()->GetPixelRGBA(expected,cx+1,cy+1);
    if(!check(tiled->scrollX_==-1&&tilePixel(0,0)==expected.ui32,"restored interval advances actual tile pixels"))return false;
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();
    EMemoryFile again;if(!status(again.Create(2048),"second save buffer")||!status(image.m_csgGlobal.Save(again,context),"second save of reconstructed graph"))return false;
    auto badLoad=[&](std::vector<uint8_t> bytes,const char* detail){EMemoryFile file;file.Open(bytes.data(),bytes.size());ECSSuperSprite invalid;
        return check(invalid.Load(file,context)!=eslErrSuccess,detail);};
    auto truncated=original;truncated.pop_back();if(!badLoad(std::move(truncated),"truncated effect reference fails"))return false;
    auto large=original;StudySteadyLegacyWire::Write32(large.data()+effectOffset,133);
    if(!badLoad(std::move(large),"oversized effect parameter fails"))return false;
    auto unknown=original;StudySteadyLegacyWire::Write32(unknown.data()+effectOffset+4,9);
    if(!badLoad(std::move(unknown),"unimplemented mesh effect fails explicitly"))return false;
    auto dimension=original;StudySteadyLegacyWire::Write32(dimension.data()+effectOffset+4+32,0);
    if(!badLoad(std::move(dimension),"zero TileImage dimension fails"))return false;
    {
        ECSSuperSprite empty;EMemoryFile file;std::array<uint32_t,33> emptyWords{};size_t offset=0;
        if(!status(file.Create(1024),"missing-image fixture")||!status(empty.Save(file,context),"canonical default etNothing saves")||
           !check(inspect(file,emptyWords,offset)&&std::all_of(emptyWords.begin(),emptyWords.end(),[](uint32_t word){return word==0;}),"fresh etNothing has deterministic zeroed parameter blob"))return false;
        std::vector<uint8_t> bytes(static_cast<const uint8_t*>(file.GetBuffer()),static_cast<const uint8_t*>(file.GetBuffer())+file.GetLength());
        StudySteadyLegacyWire::Write32(bytes.data()+offset+4,1);StudySteadyLegacyWire::Write32(bytes.data()+offset+4+32,5);StudySteadyLegacyWire::Write32(bytes.data()+offset+4+36,3);
        EMemoryFile missing;missing.Open(bytes.data(),bytes.size());ECSSuperSprite invalid;
        if(!status(invalid.Load(missing,context),"missing-source record is structurally valid")||
           !check(invalid.CommitAllReference(context)!=eslErrSuccess,"TileImage with no restored source fails Commit"))return false;
    }
    if(!check(scene->NativeSprite().GetChildCount()==2&&named(image.m_csgGlobal,L"tiled")==tiled,"negative fixtures preserve restored graph"))return false;
    {
        // Drive the script API, native renderer and real action clock together.
        ECSSuperSprite color;SGLSprite destination;
        if(!check(!color.NativeSprite().CreateBuffer(2,1)&&!destination.CreateBuffer(2,1),"color transition real canvases"))return false;
        color.NativeSprite().SetFillBackColor(0xff102030,true);
        // The SDK's SetFillBackColor only notifies an existing parent. This
        // unattached fixture owns a fresh canvas and must request its fill.
        color.NativeSprite().PostUpdate();
        destination.SetFillBackColor(0xff010203,true);
        color.NativeSprite().SetTransparency(211);
        ECSStructure effect;effect.m_pwszTag=L"EffectParam";effect.AddNewVariable(L"strType",new ECSString(L"FilterLight"));
        if(!check(call(color,L"SetEffectParameter",{new ECSReference(&effect)})&&
                  call(color,L"SetBlendDegree",{new ECSInteger(0)}),"FilterLight selected through actual native API"))return false;
        auto renderPixel=[&](ECSSuperSprite& target,uint32_t& value,uint32_t expected) {
            destination.AddChild(&target.NativeSprite());destination.PostUpdate();
            destination.PrepareDrawFrame();destination.BeforeDraw();
            SGLPalette pixel{},sourcePixel{},filteredPixel{};SGLImageInfo sourceInfo{};
            auto* source=target.GetImage();auto* result=target.colorEffect_?target.colorEffect_->ResultImage():nullptr;
            const auto sourceError=source?source->GetPixelRGBA(sourcePixel,0,0):sglErrFailed;
            const auto filterError=result?result->GetPixelRGBA(filteredPixel,0,0):sglErrFailed;
            if(source)source->GetImageInfo(sourceInfo);
            const auto error=destination.GetFrameBuffer()->GetImage()->GetPixelRGBA(pixel,0,0);
            const auto& parameter=target.NativeSprite().GetParameter();
            study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady",
                "SuperSprite pixel degree=%u source=%08x/%d format=%08x/%u filtered=%08x/%d target=%08x/%d expected=%08x pos=%.1f,%.1f center=%.1f,%.1f transparency=%u",
                GetLegacySpriteEffectDegree(target),sourcePixel.ui32,int(sourceError),sourceInfo.format,sourceInfo.depth,
                filteredPixel.ui32,int(filterError),pixel.ui32,int(error),expected,parameter.vDst.x,parameter.vDst.y,
                parameter.vCenter.x,parameter.vCenter.y,parameter.nTransparency);
            destination.AfterDraw();destination.FinishDrawFrame();
            destination.DetachChild(&target.NativeSprite());value=pixel.ui32;return !error;
        };
        uint32_t value=0;
        if(!check(renderPixel(color,value,0xff102030)&&value==0xff102030,"FilterLight identity renders original pixels independently of transparency"))return false;
        if(!check(call(color,L"SetBlendingEnvelope",{new ECSInteger(256)})&&call(color,L"BeginActivation",{new ECSInteger(100)}),"real effect-degree activation"))return false;
        color.NativeSprite().AdvanceTime(50);
        if(!check(GetLegacySpriteEffectDegree(color)==128&&color.NativeSprite().GetTransparency()==211&&
                  renderPixel(color,value,0xff90a0b0)&&value==0xff90a0b0,"half transition adds degree to RGB without fading alpha"))return false;
        SGLPalette unchanged;
        if(!check(!color.GetImage()->GetPixelRGBA(unchanged,0,0)&&unchanged.ui32==0xff102030&&
                  renderPixel(color,value,0xff90a0b0)&&value==0xff90a0b0,"effect refresh leaves source pixels intact"))return false;
        {
            ECSToneFilter preTone;
            if(!check(!preTone.SetGeneralTone({256,1,0,0,0,0,0,0})&&
                      call(color,L"AttachToneFilter",{new ECSReference(&preTone)})&&
                      renderPixel(color,value,0xffffa0b0)&&value==0xffffa0b0,
                      "base tone is applied before additive color transition"))return false;
            if(!check(call(color,L"AttachToneFilter",{new ECSReference})&&
                      renderPixel(color,value,0xff90a0b0)&&value==0xff90a0b0,
                      "clearing base tone retains independent transition"))return false;
        }
        EMemoryFile saved;if(!status(saved.Create(1024),"color-state buffer")||!status(color.Save(saved,context),"save color LUT kind, separate degree and ongoing action"))return false;
        saved.SeekLarge(0,ESLFileObject::FromBegin);ECSSuperSprite restored;
        if(!status(restored.Load(saved,context),"load color effect")||!status(restored.CommitAllReference(context),"commit color effect and reconnect restored action"))return false;
        study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","SuperSprite restored kind=%u degree=%u transparency=%u action=%d",
            restored.effectWords_[0],GetLegacySpriteEffectDegree(restored),restored.NativeSprite().GetTransparency(),restored.NativeSprite().IsAction());
        if(!check(restored.effectWords_[0]==3&&GetLegacySpriteEffectDegree(restored)==128&&
                  restored.NativeSprite().GetTransparency()==211,"restored action startup keeps separate degree and transparency")||
           !check(renderPixel(restored,value,0xff90a0b0)&&value==0xff90a0b0,
                  "restored transition matches exact intermediate pixels"))return false;
        restored.NativeSprite().AdvanceTime(100);
        if(!check(GetLegacySpriteEffectDegree(restored)==256&&renderPixel(restored,value,0xffffffff)&&value==0xffffffff&&
                  restored.NativeSprite().GetTransparency()==211,"restored action reaches white without becoming transparent"))return false;
        EMemoryFile again;if(!status(again.Create(1024),"color second buffer")||!status(restored.Save(again,context),"restored transition can save again"))return false;
        SGLImage partial;partial.CreateImage(1,1,formatImageARGB|formatImageFlagNoProductOfAlpha,32);partial.FillImage(SGLPalette(0x701020f0));
        auto dumpPixel=[](SGLImageObject* item,const char* stage) {
            SGLImageInfo info{};SGLPalette raw{},rgba{};SGLError metadata=sglErrFailed,rawError=sglErrFailed,rgbaError=sglErrFailed;
            if(item){metadata=item->GetImageInfo(info);rawError=item->GetPixel(raw,0,0);rgbaError=item->GetPixelRGBA(rgba,0,0);}
            study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","SuperSprite format pixel %s metadata=%d format=%08x/%u raw=%08x/%d premulRGBA=%08x/%d",
                stage,int(metadata),info.format,info.depth,raw.ui32,int(rawError),rgba.ui32,int(rgbaError));
        };
        auto* filtered=restored.colorEffect_->Filter(&partial);dumpPixel(&partial,"partial-source");dumpPixel(filtered,"partial-filtered");
        // GetPixelRGBA converts to the SDK's premultiplied ARGB. Check the
        // explicitly straight output's raw bytes when comparing straight RGB.
        bool formatsPass=check(filtered&&!filtered->GetPixel(unchanged,0,0)&&unchanged.ui32==0x70ffffff,"color LUT preserves partial source alpha");
        class ReadOnlyImage final:public SGLImage {
        public:
            size_t reads=0,lastFrame=SIZE_MAX;
            uint8_t* LockBuffer(SGLImageInfo&,int,const SGLImageRect*) override {return nullptr;}
            SGLError ReadFrameBuffer(const SGLImageInfo& info,uint8_t* pixels,size_t frame,int side) override {
                ++reads;lastFrame=frame;return SGLImage::ReadFrameBuffer(info,pixels,frame,side);
            }
        } readOnly;
        if(!check(!readOnly.CreateImage(1,1,formatImageBGR,24,SGLImageObject::bufferOnMemory,2,100),
                  "read-only multiframe BGR24 source"))return false;
        // Fill via a normal reference: the public source intentionally cannot
        // be LockBuffer'ed, just like a GPU/snapshot readback-only object.
        readOnly.SelectFrame(0);std::unique_ptr<SGLImageObject> first(readOnly.NewReference(nullptr,0));
        readOnly.SelectFrame(1);std::unique_ptr<SGLImageObject> selected(readOnly.NewReference(nullptr,1));
        if(!check(first&&selected&&!first->SetPixelRGBA(0,0,SGLPalette(0xffc03010))&&
                  !selected->SetPixelRGBA(0,0,SGLPalette(0xff102030)),"distinct source frame pixels"))return false;
        restored.colorEffect_->SetDegree(128);filtered=restored.colorEffect_->Filter(&readOnly);
        dumpPixel(selected.get(),"BGR24-source");dumpPixel(filtered,"BGR24-filtered");
        formatsPass=check(filtered&&!filtered->GetPixelRGBA(unchanged,0,0)&&unchanged.ui32==0xff90a0b0&&
                  readOnly.reads==1&&readOnly.lastFrame==1&&readOnly.GetSelectedFrame()==1,
                  "readback-only BGR24 converts exactly the selected frame to RGB32")&&formatsPass;
        SGLImage abgr;abgr.CreateImage(1,1,formatImageABGR|formatImageFlagNoProductOfAlpha,32);
        abgr.SetPixel(0,0,SGLPalette(0x70302010));filtered=restored.colorEffect_->Filter(&abgr);
        dumpPixel(&abgr,"ABGR32-source");dumpPixel(filtered,"ABGR32-filtered");
        formatsPass=check(filtered&&!filtered->GetPixel(unchanged,0,0)&&unchanged.ui32==0x7090a0b0,
                  "ABGR32 channel conversion preserves straight alpha and LUT colors")&&formatsPass;
        const wchar_t* variants[]={L"FilterWhite",L"FilterBlack",L"FilterDark"};
        const uint32_t expectedPixels[]={0xff878f97,0xff081018,0xff000000};
        for(size_t i=0;i<3;++i){
            effect.SetMemberAsStr(L"strType",variants[i]);
            if(!check(call(color,L"CancelActivation",{})&&call(color,L"SetEffectParameter",{new ECSReference(&effect)})&&
               call(color,L"SetBlendDegree",{new ECSInteger(128)})&&renderPixel(color,value,expectedPixels[i])&&value==expectedPixels[i],
               "shared original brightness/additive filter variant math"))return false;
        }
        effect.SetMemberAsStr(L"strType",L"Nothing");
        if(!check(call(color,L"SetEffectParameter",{new ECSReference(&effect)})&&
                  call(color,L"SetBlendDegree",{new ECSInteger(0)})&&renderPixel(color,value,0xff102030)&&value==0xff102030,
                  "clearing effect restores ordinary rendering and blend semantics"))return false;
        if(!formatsPass)return false;
    }
    for(const auto kind:{6u,10u,11u}) {
        ECSSuperSprite original;original.NativeSprite().CreateBuffer(2,2);
        original.NativeSprite().SetFillBackColor(0xff102030,true);original.NativeSprite().PostUpdate();
        original.NativeSprite().SetTransparency(57);
        ECSStructure effect;effect.m_pwszTag=L"EffectParam";
        effect.AddNewVariable(L"strType",new ECSString(kind==6?L"RasterScroll":kind==10?L"ShadingOff":L"ShadingLight"));
        effect.SetMemberAsInt(L"nFlags",2);effect.SetMemberAsInt(L"nInterval",16);
        effect.SetMemberAsInt(L"nDegreeStep",3);effect.SetMemberAsInt(L"nShakingWidth",2);
        effect.SetMemberAsInt(L"nMeshSize",4);effect.SetMemberAsInt(L"nFrequency",1);
        if(!check(call(original,L"SetEffectParameter",{new ECSReference(&effect)})&&
                  call(original,L"SetBlendDegree",{new ECSInteger(128)}),"new effect native parameter and separate degree"))return false;
        original.NativeSprite().AdvanceTime(21);
        if(!check(GetLegacySpriteEffectDegree(original)==131,"new effect advances real interval degree"))return false;
        EMemoryFile saved;if(!status(saved.Create(1024),"new effect wire buffer")||
            !status(original.Save(saved,context),"new effect Save"))return false;
        saved.SeekLarge(0,ESLFileObject::FromBegin);ECSSuperSprite copy;
        if(!status(copy.Load(saved,context),"new effect Load")||!status(copy.CommitAllReference(context),"new effect Commit")||
           !check(copy.effectWords_[0]==kind&&copy.effectWords_[1]==2&&GetLegacySpriteEffectDegree(copy)==131&&
                  copy.NativeSprite().GetTransparency()==57,"new effect preserves original type flags degree and opacity"))return false;
        if(kind==6&&!check(copy.rasterEffect_&&copy.rasterEffect_->IntervalCounter()==0,"RasterScroll interval remainder resets on restore"))return false;
        if(kind!=6&&!check(copy.shadingEffect_&&copy.elapsed_==0,"Shading interval remainder resets on restore"))return false;
        copy.NativeSprite().AdvanceTime(15);
        if(!check(GetLegacySpriteEffectDegree(copy)==131,"restored new effect holds before interval"))return false;
        copy.NativeSprite().AdvanceTime(1);
        if(!check(GetLegacySpriteEffectDegree(copy)==134,"restored new effect advances at boundary"))return false;
        EMemoryFile second;if(!status(second.Create(1024),"new effect second buffer")||!status(copy.Save(second,context),"new effect repeat save"))return false;
    }
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","SuperSprite probe PASS: graph_bytes=%llu; real FilterLight RGB/alpha pixels, independent animated degree, state restore, brightness variants, real tiled pixels, original 132-byte wire, etNothing, resource/base refs, destroy/rebuild, interval reset, second save, unsupported/truncated/missing errors",static_cast<unsigned long long>(graphBytes));
    return true;
}
