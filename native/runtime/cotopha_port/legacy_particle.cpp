#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_particle.h"
#include "runtime/cotopha_port/legacy_particle_model.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_sprite_draw.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include "platform/log.h"
#include <algorithm>
#include <cmath>
#include <array>

namespace {
using namespace SakuraGL;
using namespace LegacyParticle;
struct Lock {Lock(){SSystem::Lock();}~Lock(){SSystem::Unlock();}};
const wchar_t* methods[]={L"SetParticleImageLimit",L"SetParticleImage",L"SetParticleParameter",
    L"SetParticleGeneratorMask",L"SetParticleRectangle",L"CreateParticle",L"SetParticleGenerator"};
struct IntegerField {const wchar_t* name;uint32_t Param::*member;};
const IntegerField integerFields[]={
    {L"nFlags",&Param::nFlags},{L"nDuration",&Param::nDuration},{L"nAnimationSpeed",&Param::nAnimationSpeed},
    {L"nFadein",&Param::nFadein},{L"nFadeout",&Param::nFadeout},{L"nFadeTransparency",&Param::nFadeTransparency}};
struct RealField {const wchar_t* name;double Param::*member;};
const RealField realFields[]={
    {L"rFadeZoom",&Param::rFadeZoom},{L"rGenWidth",&Param::rGenWidth},{L"rGenHeight",&Param::rGenHeight},
    {L"rGenAngle",&Param::rGenAngle},{L"rGenAngleRange",&Param::rGenAngleRange},
    {L"rGenVelocity",&Param::rGenVelocity},{L"rGenVelocityRange",&Param::rGenVelocityRange},
    {L"rShrink",&Param::rShrink},{L"rRevSpeed",&Param::rRevSpeed},{L"rRevSpeedRange",&Param::rRevSpeedRange},
    {L"rZoom",&Param::rZoom},{L"rZoomRange",&Param::rZoomRange}};
struct FlickField {const wchar_t* name;double Flick::*member;};
const FlickField flickFields[]={{L"rAmplitude",&Flick::rAmplitude},{L"rAmplitudeRange",&Flick::rAmplitudeRange},
    {L"rFrequency",&Flick::rFrequency},{L"rFrequencyRange",&Flick::rFrequencyRange}};
ECSStructureInterface* Structure(ECSObject* object) {
    return ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(object));
}
ECSObject* Member(ECSStructureInterface& parent,const wchar_t* name) {
    auto* object=parent.GetInstanceObject();int index=0;
    return !object||object->GetVariableIndex(index,name)?nullptr:object->GetVariableAt(index);
}
Vec2 ReadVector(ECSStructureInterface& parent,const wchar_t* name) {
    Vec2 out{};if(auto* point=Structure(Member(parent,name))) {
        out.x=float(point->GetMemberAsReal(L"x",0));out.y=float(point->GetMemberAsReal(L"y",0));
    }return out;
}
bool Valid(const Param& value) {
    if((value.nFlags&~1u)||value.nFadeTransparency>256||value.rShrink>1)return false;
    for(const auto& field:realFields)if(!std::isfinite(value.*field.member))return false;
    for(const auto& flick:value.pfFlickness)for(const auto& field:flickFields)
        if(!std::isfinite(flick.*field.member))return false;
    for(auto point:{value.vGenSpeed,value.vStream,value.vGravity})
        if(!std::isfinite(point.x)||!std::isfinite(point.y))return false;
    return std::isfinite(value.rGenSpeedRange);
}
void WriteVector(LegacySave::Writer& out,Vec2 p){out.F32(p.x);out.F32(p.y);}
Vec2 ReadVector(LegacySave::Reader& in){const auto x=in.F32(),y=in.F32();return {x,y};}
void WriteFlicks(LegacySave::Writer& out,const Flick* values) {
    for(int i=0;i<2;++i)for(const auto& f:flickFields)out.F64(values[i].*f.member);
}
void ReadFlicks(LegacySave::Reader& in,Flick* values) {
    for(int i=0;i<2;++i)for(const auto& f:flickFields)values[i].*f.member=in.F64();
}
void WriteParam(LegacySave::Writer& out,const Param& value) {
    for(const auto& field:integerFields)out.U32(value.*field.member);
    for(const auto& field:realFields)out.F64(value.*field.member);
    WriteFlicks(out,value.pfFlickness);
    WriteVector(out,value.vGenSpeed);out.F64(value.rGenSpeedRange);
    WriteVector(out,value.vStream);WriteVector(out,value.vGravity);
}
Param ReadParam(LegacySave::Reader& in) {
    Param value{};
    for(const auto& field:integerFields)value.*field.member=in.U32();
    for(const auto& field:realFields)value.*field.member=in.F64();
    ReadFlicks(in,value.pfFlickness);
    value.vGenSpeed=ReadVector(in);value.rGenSpeedRange=in.F64();
    value.vStream=ReadVector(in);value.vGravity=ReadVector(in);
    if(!Valid(value))in.error=eslErrInvalidParam;
    return value;
}
void WriteParticle(LegacySave::Writer& out,const Particle& p) {
    out.U32(p.iParticleImage);out.U32(p.nPastTime);out.U32(p.nAnimeTime);
    WriteVector(out,p.vShow);WriteVector(out,p.vPos);WriteVector(out,p.vVelocity);WriteVector(out,p.vAcceleration);
    out.U32(0); // Win32 MSVC alignment before the first double, not a host pointer.
    out.F64(p.rRevAngle);out.F64(p.rRevSpeed);out.F64(p.rZoom);
    WriteFlicks(out,p.pfFlickness);WriteVector(out,p.vFlickUnit);out.F64(p.rFlicknessPhase);
}
Particle ReadParticle(LegacySave::Reader& in) {
    Particle p{};p.iParticleImage=in.U32();p.nPastTime=in.U32();p.nAnimeTime=in.U32();
    p.vShow=ReadVector(in);p.vPos=ReadVector(in);p.vVelocity=ReadVector(in);p.vAcceleration=ReadVector(in);
    in.U32(); // Original padding can contain arbitrary bytes.
    p.rRevAngle=in.F64();p.rRevSpeed=in.F64();p.rZoom=in.F64();
    ReadFlicks(in,p.pfFlickness);p.vFlickUnit=ReadVector(in);p.rFlicknessPhase=in.F64();return p;
}
}

struct ECSParticleSprite::Impl {
    struct Image {
        ECSReference source;
        std::shared_ptr<SGLImageObject> image;
        SGLPoint hotspot{0,0};
    };
    struct View {
        std::unique_ptr<SGLSprite> sprite{new SGLSprite};
        std::shared_ptr<SGLImageObject> frame;
        const SGLImageObject* source=nullptr;
        size_t index=SIZE_MAX;
        View(){sprite->SetEnable(false);} // Rendered particles are not independent input controls.
    };
    Model model;
    std::vector<std::unique_ptr<Image>> images;
    std::vector<std::unique_ptr<View>> views;
    ECSReference mask;
    std::array<int32_t,8> generator{{0,0,65536,65536,65536,65536,0,0}};
    Vec2 ray{1,0};
    SGLRect valid{0,0,639,479},bounds{0,0,-1,-1};
    bool restoring=false,failed=false;
    Impl(){model.seed=timeGetTime();images.emplace_back(new Image);}
    void ClearViews(ECSParticleSprite& owner) {
        for(auto& view:views)owner.NativeSprite().DetachChild(view->sprite.get());
        views.clear();
    }
    void Resize(size_t count) {
        images.resize(count);
        for(auto& image:images)if(!image)image.reset(new Image);
    }
    ESLError Reopen(Image& image) {
        auto* resource=ESLTypeCast<ECSResource>(ECSObject::GetEntity(&image.source));
        image.image.reset();
        if(!resource)return image.source.m_pRef?eslErrInvalidParam:eslErrSuccess;
        if(!resource->GetImage())return ESLErrorMsg("Particle image resource has no decoded image");
        image.image.reset(resource->GetImage()->NewReference());
        return image.image?eslErrSuccess:eslErrGeneral;
    }
};

IMPLEMENT_CLASS_INFO(ECSParticleSprite,ECSSprite)
class ECSParticleSprite::ParticleNative final:public LegacyCallbackSprite<SGLSpriteFormed> {
    ECSParticleSprite& owner_;
public:
    explicit ParticleNative(ECSParticleSprite& owner):owner_(owner){}
    void AdvanceTime(uint32_t ms) override {owner_.AdvanceParticles(ms);LegacyCallbackSprite::AdvanceTime(ms);}
    void PrepareDrawFrame() override {owner_.DrawParticles();LegacyCallbackSprite::PrepareDrawFrame();}
};
ECSParticleSprite::ECSParticleSprite():ECSSprite(new ParticleNative(*this)),impl_(new Impl){}
ECSParticleSprite::~ECSParticleSprite(){
    Lock lock;
    if(auto* parent=NativeSprite().GetParent())parent->DetachChild(&NativeSprite());
    impl_->ClearViews(*this);
}
const wchar_t* ECSParticleSprite::GetTypeName() const{return L"ParticleSprite";}
ECSObject* ECSParticleSprite::GetTypeOf(const wchar_t* name){
    return !EWideString::Compare(name,L"ParticleSprite")?this:ECSSprite::GetTypeOf(name);
}
// The original ParticleSprite::Duplicate creates an empty emitter. Typed-array
// prototypes rely on that behavior when constructing ScreenData.
ECSObject* ECSParticleSprite::Duplicate(){return new ECSParticleSprite;}
ESLError ECSParticleSprite::GetFunction(ECSContext& context,int& index,const wchar_t* name){
    for(int i=0;i<7;++i)if(!EWideString::Compare(name,methods[i])){index=4096+i;return eslErrSuccess;}
    return ECSSprite::GetFunction(context,index,name);
}
ESLError ECSParticleSprite::CallFunction(ECSContext& context,int index,ECSObjArray<ECSObject>& args){
    if(index<4096)return ECSSprite::CallFunction(context,index,args);
    const int method=index-4096;if(method<0||method>=7)return eslErrInvalidParam;
    Lock lock;ESLError error=eslErrSuccess;
    auto count=[&](int minimum,int maximum=0){return context.VerifyArgumentCount(args,minimum,maximum);};
    auto result=[&](INT64 value=0){return context.PushObject(context.new_CSInteger(value));};
    if(method==0){
        if((error=count(1,2)))return error;int limit;
        if((error=context.GetArgumentAsInt(limit,args,1,1)))return error;
        if(limit<0||limit>4096)return eslErrInvalidParam;
        impl_->Resize(limit);return result();
    }
    if(method==1){
        if((error=count(1,4)))return error;int slot=0;
        if((error=context.GetArgumentAsInt(slot,args,3,0)))return error;
        if(slot<0||size_t(slot)>=impl_->images.size())return result(eslErrInvalidParam);
        auto* object=ECSObject::GetEntity(args.GetAt(1));
        auto* resource=ESLTypeCast<ECSResource>(object);
        if(object&&!resource)return eslErrInvalidParam;
        auto& image=*impl_->images[slot];image.source.SetReference(resource,&context);
        if((error=impl_->Reopen(image)))return result(error);
        if(auto* point=Structure(args.GetAt(2)))
            image.hotspot=SGLPoint(point->GetMemberAsInt(L"x",0),point->GetMemberAsInt(L"y",0));
        else if(image.image){auto size=image.image->GetImageSize();image.hotspot=SGLPoint(size.w/2,size.h/2);}
        else image.hotspot=SGLPoint(0,0);
        return result();
    }
    if(method==2){
        if((error=count(2)))return error;auto* object=Structure(args.GetAt(1));
        if(!object)return eslErrInvalidParam;Param value{};
        for(const auto& field:integerFields)value.*field.member=object->GetMemberAsInt(field.name,0);
        for(const auto& field:realFields)value.*field.member=object->GetMemberAsReal(field.name,0);
        if(auto* array=ESLTypeCast<ECSArray>(ECSObject::GetEntity(Member(*object,L"pfFlickness"))))
            for(int i=0;i<2;++i)if(auto* flick=Structure(array->m_varArray.GetAt(i)))
                for(const auto& field:flickFields)value.pfFlickness[i].*field.member=flick->GetMemberAsReal(field.name,0);
        value.vGenSpeed=ReadVector(*object,L"vGenSpeed");value.rGenSpeedRange=object->GetMemberAsReal(L"rGenSpeedRange",0);
        value.vStream=ReadVector(*object,L"vStream");value.vGravity=ReadVector(*object,L"vGravity");
        if(!Valid(value))return eslErrInvalidParam;
        impl_->model.param=value;impl_->failed=false;return result();
    }
    if(method==3){
        if((error=count(1,8)))return error;
        if(ECSObject::GetEntity(args.GetAt(1)))return ESLErrorMsg("Particle generator masks are not supported by this Android adapter");
        impl_->mask.SetReference(nullptr,&context);return result();
    }
    if(method==4){
        if((error=count(2)))return error;auto* rect=Structure(args.GetAt(1));
        if(!rect)return eslErrInvalidParam;
        impl_->valid=SGLRect(rect->GetMemberAsInt(L"left",0),rect->GetMemberAsInt(L"top",0),
            rect->GetMemberAsInt(L"right",-1),rect->GetMemberAsInt(L"bottom",-1));return result();
    }
    if((error=count(2)))return error;int amount;
    if((error=context.GetArgumentAsInt(amount,args,1,0)))return error;
    if(amount<0)return eslErrInvalidParam;
    if(method==6){impl_->model.generationCount=amount;impl_->failed=false;return result();}
    const auto position=NativeSprite().GetPosition();
    return result(impl_->model.Generate(amount,float(position.x),float(position.y),impl_->images.size())?0:eslErrInvalidParam);
}

void ECSParticleSprite::AdvanceParticles(uint32_t ms){
    Lock lock;if(impl_->restoring||impl_->failed||!LegacyEffectAnimationEnabled())return;
    std::vector<ImageInfo> images;images.reserve(impl_->images.size());
    for(const auto& image:impl_->images){
        ImageInfo info{};if(image->image){const auto size=image->image->GetImageSize();
            info={uint32_t(size.w),uint32_t(size.h),uint32_t(image->image->GetTotalTime())};}
        images.push_back(info);
    }
    const auto position=NativeSprite().GetPosition();
    if(!impl_->model.Advance(ms,float(position.x),float(position.y),images)){
        impl_->failed=true;study::platform::LogWrite(study::platform::LogPriority::Error,"StudySteady","Particle simulation failed: invalid values or particle count limit");
    }
}
void ECSParticleSprite::DrawParticles(){
    Lock lock;if(impl_->restoring)return;
    const auto& particles=impl_->model.particles;
    while(impl_->views.size()>particles.size()){NativeSprite().DetachChild(impl_->views.back()->sprite.get());impl_->views.pop_back();}
    while(impl_->views.size()<particles.size())impl_->views.emplace_back(new Impl::View);
    const auto origin=NativeSprite().GetPosition();const auto& emitter=impl_->model.param;
    for(size_t i=0;i<particles.size();++i){
        const auto& p=particles[i];auto& view=*impl_->views[i];
        if(view.sprite->GetParent()!=&NativeSprite())NativeSprite().AddChild(view.sprite.get());
        if(p.iParticleImage>=impl_->images.size()||!impl_->images[p.iParticleImage]->image){view.sprite->SetVisible(false);continue;}
        auto& slot=*impl_->images[p.iParticleImage];const auto frame=slot.image->FrameFromMilliSec(p.nAnimeTime);
        if(view.source!=slot.image.get()||view.index!=frame){
            view.frame.reset(slot.image->NewReference(nullptr,frame));view.source=slot.image.get();view.index=frame;
            view.sprite->AttachImage(view.frame.get());
        }
        if(!view.frame){view.sprite->SetVisible(false);continue;}
        double zoom=p.rZoom;uint32_t transparency=0;
        if(emitter.nFadein&&p.nPastTime<emitter.nFadein){
            transparency=uint64_t(emitter.nFadeTransparency)*(emitter.nFadein-p.nPastTime)/emitter.nFadein;
            zoom+=(emitter.rFadeZoom-zoom)*p.nPastTime/emitter.nFadein;
        }else if(emitter.nFadeout&&p.nPastTime>emitter.nDuration-emitter.nFadeout){
            const auto past=p.nPastTime-(emitter.nDuration-emitter.nFadeout);
            transparency=uint64_t(emitter.nFadeTransparency)*past/emitter.nFadeout;
            zoom+=(emitter.rFadeZoom-zoom)*past/emitter.nFadeout;
        }
        auto param=view.sprite->GetParameter();param.vDst=S3DDVector(p.vShow.x-origin.x,p.vShow.y-origin.y,0);
        // Preserve the original renderer's hotspot-x-for-both-axes convention.
        param.vCenter=S2DDVector(slot.hotspot.x,slot.hotspot.x);param.vZoom=S2DDVector(zoom,zoom);
        param.zAngle=p.rRevAngle;param.nFlags=NativeSprite().GetParameter().nFlags;
        param.nTransparency=std::min<uint32_t>(transparency,256);
        view.sprite->SetParameter(param);view.sprite->SetVisible(true);
    }
}

ECSObject* ECSParticleSprite::GetVariableAt(int index){
    if(index==-11)return &impl_->mask;
    if(index<=-12&&uint64_t(-int64_t(index)-12)<impl_->images.size())return &impl_->images[-int64_t(index)-12]->source;
    return ECSSprite::GetVariableAt(index);
}
void ECSParticleSprite::IndexAllMember(){
    Lock lock;ECSSprite::IndexAllMember();
    impl_->mask.IndexAllMember();impl_->mask.m_pParent=this;impl_->mask.m_nIndex=-11;
    for(size_t i=0;i<impl_->images.size();++i){auto& ref=impl_->images[i]->source;
        ref.IndexAllMember();ref.m_pParent=this;ref.m_nIndex=-12-int(i);}
}
void ECSParticleSprite::CleanupAllReference(ECSContext& context){
    Lock lock;impl_->ClearViews(*this);
    for(auto& image:impl_->images){image->source.CleanupAllReference(context);image->image.reset();}
    impl_->mask.CleanupAllReference(context);ECSSprite::CleanupAllReference(context);
}
ESLError ECSParticleSprite::CommitAllReference(ECSContext& context){
    Lock lock;if(const auto error=ECSSprite::CommitAllReference(context))return error;
    if(!impl_->restoring)return eslErrSuccess;
    if(const auto error=impl_->mask.CommitAllReference(context))return error;
    if(impl_->mask.m_pRef)return ESLErrorMsg("Saved Particle generator mask cannot be reconstructed");
    for(auto& image:impl_->images){
        if(const auto error=image->source.CommitAllReference(context))return error;
        if(auto* sprite=ESLTypeCast<ECSSprite>(ECSObject::GetEntity(&image->source)))
            if(sprite->IsLegacyRestorePending())if(const auto error=sprite->CommitAllReference(context))return error;
        if(const auto error=impl_->Reopen(*image))return error;
    }
    impl_->restoring=false;DrawParticles();return eslErrSuccess;
}
ESLError ECSParticleSprite::Save(ESLFileObject& file,ECSContext& context){
    Lock lock;if(impl_->restoring)return ESLErrorMsg("Particle state has not finished restoration");
    if(const auto error=ECSSprite::Save(file,context))return error;
    LegacySave::Writer out{file};out.U32(impl_->images.size());
    for(auto& image:impl_->images){out.Reference(image->source,context);out.I32(image->hotspot.x);out.I32(image->hotspot.y);}
    out.Reference(impl_->mask,context);for(auto value:impl_->generator)out.I32(value);WriteVector(out,impl_->ray);
    out.U32(impl_->model.generationCount);WriteParam(out,impl_->model.param);
    out.Rect(impl_->valid);out.Rect(impl_->bounds);out.U32(impl_->model.particles.size());
    for(const auto& p:impl_->model.particles)WriteParticle(out,p);
    return out.error;
}
ESLError ECSParticleSprite::Load(ESLFileObject& file,ECSContext& context){
    Lock lock;impl_->ClearViews(*this);
    if(const auto error=ECSSprite::Load(file,context))return error;
    LegacySave::Reader in{file};impl_->Resize(in.Count(4096));
    for(auto& image:impl_->images){in.Reference(image->source,context);const auto x=in.I32(),y=in.I32();image->hotspot=SGLPoint(x,y);}
    in.Reference(impl_->mask,context);for(auto& value:impl_->generator)value=in.I32();impl_->ray=ReadVector(in);
    impl_->model.generationCount=in.U32();impl_->model.param=ReadParam(in);
    impl_->valid=in.Rect();impl_->bounds=in.Rect();const auto count=in.Count(65536);
    impl_->model.particles.clear();impl_->model.particles.reserve(count);
    for(uint32_t i=0;i<count&&!in.error;++i)impl_->model.particles.push_back(ReadParticle(in));
    impl_->restoring=!in.error;impl_->failed=false;return in.error;
}

bool CheckLegacyParticleState(ECSEnvironment& environment){
    auto check=[](bool good,const char* stage){
        study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady",
            "Legacy Particle probe %s: %s",good?"OK":"FAIL",stage);return good;
    };
    class Image final:public ECSExecutionImage {
    public:
        Image(){
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
    if(!check(!context.InitializeContext(&image),"isolated object context"))return false;
    auto* scene=new ECSSprite;image.m_csgData.AddVariable(L"scene",scene);
    auto* source=new ECSResource;image.m_csgData.AddVariable(L"source",source);
    if(!check(!source->LoadImageFile(L"particle_light1.eri",&context)&&
        !scene->NativeSprite().CreateBuffer(64,32),"real particle texture and compositing target"))return false;
    const auto imageTime=source->GetImage()->GetTotalTime();
    study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Particle texture metadata: frames=%zu total_ms=%llu",
        source->GetImage()->GetFrameCount(),static_cast<unsigned long long>(imageTime));
    scene->NativeSprite().SetVisible(true);
    std::unique_ptr<ECSParticleSprite> emitter(new ECSParticleSprite);
    auto call=[&](ECSParticleSprite& object,const wchar_t* method,std::initializer_list<ECSObject*> values){
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&object));for(auto* value:values)args.Add(value);
        int index=0;auto error=object.GetFunction(context,index,method);
        if(!error)error=object.CallFunction(context,index,args);
        auto* result=!error?context.PopObject():nullptr;INT64 status=-1;
        if(!error)error=result?result->OperateInteger(status):eslErrGeneral;
        context.delete_CSObject(result);return !error&&!status;
    };
    if(!check(call(*emitter,L"SetParticleImage",{new ECSReference(source)}),"actual script particle image attachment"))return false;
    emitter->NativeSprite().SetPosition(32,16);emitter->NativeSprite().SetVisible(true);
    scene->NativeSprite().AddChild(&emitter->NativeSprite());
    emitter->impl_->model.param.nDuration=1000;
    // This real single-frame ERI has a 33 ms animation duration. Without
    // looping, the legacy model correctly deletes it at AdvanceTime(40),
    // leaving an empty save and testing expiry rather than reconstruction.
    emitter->impl_->model.param.nFlags=Model::pfAnimationLoop;
    if(!check(call(*emitter,L"CreateParticle",{new ECSInteger(1)}),"actual script particle generation"))return false;
    emitter->DrawParticles();
    if(!check(emitter->impl_->views.size()==1&&emitter->impl_->views[0]->frame&&
        emitter->impl_->views[0]->sprite->GetParent()==&emitter->NativeSprite(),"real native particle child and texture"))return false;
    auto alphaPixels=[&](){
        scene->GetImage()->FillImage(SGLPalette(0));
        scene->NativeSprite().PostUpdate();scene->NativeSprite().PrepareDrawFrame();scene->NativeSprite().Refresh();
        size_t count=0;auto* result=scene->GetImage();SGLImageInfo info;
        if(result)if(auto* bytes=result->LockBuffer(info,SGLImageObject::lockRead)){
            for(uint32_t y=0;y<info.height;++y)for(uint32_t x=0;x<info.width;++x)count+=bytes[y*info.pitchLine+x*info.pitchPixel+3]!=0;
            result->UnlockBuffer(SGLImageObject::lockRead);
        }return count;
    };
    Lock graphics;
    const auto before=alphaPixels();
    if(!check(before>0&&before<=36,"particle renders real alpha into parent buffer"))return false;
    emitter->NativeSprite().SetPosition(48,20);emitter->DrawParticles();
    const auto local=emitter->impl_->views[0]->sprite->GetPosition();
    if(!check(local.x==-16&&local.y==-4,"existing particles keep world position when emitter moves"))return false;
    emitter->NativeSprite().AdvanceTime(40);
    if(!check(emitter->impl_->model.particles.size()==1&&
        emitter->impl_->model.particles[0].nPastTime==40&&
        emitter->impl_->model.particles[0].nAnimeTime==(imageTime?40%imageTime:40),
        "save fixture contains a live aged particle with wrapped animation time"))return false;
    const auto savedParticle=emitter->impl_->model.particles[0];
    image.m_csgData.IndexAllMember();emitter->IndexAllMember();
    EMemoryFile saved;if(!check(!saved.Create(1024)&&!emitter->Save(saved,context),"original fixed particle record save"))return false;
    const auto bytes=saved.GetLength();emitter.reset();
    if(!check(scene->NativeSprite().GetChildCount()==0,"destroyed emitter detaches renderer children"))return false;
    emitter.reset(new ECSParticleSprite);saved.SeekLarge(0,ESLFileObject::FromBegin);
    if(!check(!emitter->Load(saved,context)&&!emitter->CommitAllReference(context),"restore native particle and static image/parent references"))return false;
    const auto after=alphaPixels();
    study::platform::LogPrint(study::platform::LogPriority::Debug,"StudySteady","Particle restored state: count=%zu age=%u parent=%d alpha=%zu/%zu visible=%d transparency=%u views=%zu",
        emitter->impl_->model.particles.size(),emitter->impl_->model.particles.empty()?0:emitter->impl_->model.particles[0].nPastTime,
        emitter->NativeSprite().GetParent()==&scene->NativeSprite(),after,before,emitter->NativeSprite().IsVisible(),
        emitter->NativeSprite().GetTransparency(),emitter->impl_->views.size());
    if(!check(emitter->impl_->model.particles.size()==1&&
        emitter->impl_->model.param.nFlags==Model::pfAnimationLoop&&
        emitter->impl_->model.particles[0].nPastTime==savedParticle.nPastTime&&
        emitter->impl_->model.particles[0].nAnimeTime==savedParticle.nAnimeTime&&
        emitter->impl_->model.particles[0].vPos.x==savedParticle.vPos.x&&
        emitter->impl_->model.particles[0].vPos.y==savedParticle.vPos.y&&
        emitter->NativeSprite().GetParent()==&scene->NativeSprite()&&after==before,
        "particle position, age and rendered pixels survive reconstruction"))return false;
    image.m_csgData.IndexAllMember();emitter->IndexAllMember();
    EMemoryFile again;if(!check(!again.Create(1024)&&!emitter->Save(again,context),"restored emitter can save again"))return false;
    EMemoryFile shortFile;shortFile.Open(saved.GetBuffer(),saved.GetLength()-1);ECSParticleSprite partial;
    if(!check(partial.Load(shortFile,context)!=eslErrSuccess,"truncated particle fails"))return false;
    emitter->NativeSprite().AdvanceTime(960);emitter->DrawParticles();
    if(!check(emitter->impl_->model.particles.empty()&&emitter->impl_->views.empty(),"restored particle expires at original duration"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy Particle probe PASS: %lu-byte wire, real alpha=%zu, image/parent restore, world position, lifetime and truncated record",
        static_cast<unsigned long>(bytes),before);
    return true;
}
