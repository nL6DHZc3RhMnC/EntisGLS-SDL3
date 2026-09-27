#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_emote.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include "platform/log.h"
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {
bool Check(bool good,const char* stage) {
    study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady",
        "Emote Sprite save probe %s: %s",good?"OK":"FAIL",stage);
    return good;
}
bool Status(ESLError error,const char* stage) {
    if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Emote Sprite save probe %s: %s",stage,GetESLErrorMsg(error));
    return Check(!error,stage);
}
class StateImage final:public ECSExecutionImage {
public:
    StateImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
struct GraphicsLock {GraphicsLock(){SSystem::Lock();}~GraphicsLock(){SSystem::Unlock();}};
class TrackedActor final:public ECSEmoteSprite {
    int& destroyed_;
public:
    explicit TrackedActor(int& count):destroyed_(count){}
    ~TrackedActor() override {++destroyed_;}
};
ECSObject* Named(ECSGlobal& root,const wchar_t* name) {
    int index=0;return root.GetVariableIndex(index,name)?nullptr:ECSObject::GetEntity(root.GetVariableAt(index));
}
bool Invoke(ECSContext& context,ECSObject& object,const wchar_t* method,
            std::initializer_list<ECSObject*> values,INT64& status) {
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&object));
    for(auto* value:values)args.Add(value);
    int index=0;auto error=object.GetFunction(context,index,method);
    if(!error)error=object.CallFunction(context,index,args);
    auto* result=!error?context.PopObject():nullptr;
    if(!error)error=result?result->OperateInteger(status):eslErrGeneral;
    context.delete_CSObject(result);
    if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Emote Sprite save probe call %ls: %s",method,GetESLErrorMsg(error));
    return !error;
}
bool Call(ECSContext& context,ECSObject& object,const wchar_t* method,std::initializer_list<ECSObject*> values={}) {
    INT64 status=-1;const bool ok=Invoke(context,object,method,values,status);
    if(ok&&status)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Emote Sprite save probe call %ls returned %lld",method,static_cast<long long>(status));
    return ok&&status==0;
}
bool Playing(ECSContext& context,ECSEmoteSprite& actor,const wchar_t* name,bool expected) {
    INT64 status=1;return Invoke(context,actor,L"IsPlayingTimeline",{new ECSString(name)},status)&&status==(expected?-1:0);
}
struct Wire {
    uint32_t width=0,height=0;
    float scale=0,x=0,y=0;
    std::wstring path,timeline;
    std::vector<std::wstring> queue;
    size_t pathOffset=0,queueOffset=0;
    bool sameDevice=false;
};
// Consume the actual Sprite prefix; inspect the derived wire without private fields.
bool Inspect(EMemoryFile& file,ECSContext& context,ECSEmoteDevice& device,Wire& wire) {
    if(file.SeekLarge(0,ESLFileObject::FromBegin)!=0)return false;
    ECSSprite prefix;
    if(!Status(prefix.Load(file,context),"decode Sprite base prefix for wire audit"))return false;
    ECSReference reference;LegacySave::Reader in{file};in.Reference(reference,context);
    if(in.error||!Status(reference.CommitAllReference(context),"wire static device reference"))return false;
    wire.sameDevice=ECSObject::GetEntity(&reference)==&device;
    wire.width=in.U32();wire.height=in.U32();wire.scale=in.F32();wire.x=in.F32();wire.y=in.F32();
    wire.pathOffset=static_cast<size_t>(file.GetPosition());
    const auto path=in.String(),timeline=in.String();
    wire.path=static_cast<const wchar_t*>(path);wire.timeline=static_cast<const wchar_t*>(timeline);
    wire.queueOffset=static_cast<size_t>(file.GetPosition());
    const auto count=in.Count();wire.queue.clear();
    for(uint32_t i=0;i<count&&!in.error;++i){const auto name=in.String();wire.queue.emplace_back(static_cast<const wchar_t*>(name));}
    return Status(in.error,"derived fixed-width fields and UTF-16 queue")&&Check(file.GetPosition()==file.GetLength(),"derived record has no unexpected trailing extension");
}
bool SaveOne(ECSEmoteSprite& actor,EMemoryFile& file,ECSContext& context) {
    return Status(file.Create(2048),"individual record buffer")&&Status(actor.Save(file,context),"actual EmoteSprite::Save");
}
bool Expected(const Wire& wire,const wchar_t* active,std::initializer_list<std::wstring> queue) {
    return wire.sameDevice&&wire.width==1024&&wire.height==768&&std::fabs(wire.scale-.25f)<.00001f&&
        wire.x==17&&wire.y==-80&&wire.path==L"haz_a.psb"&&wire.timeline==active&&wire.queue==std::vector<std::wstring>(queue);
}
size_t RenderAlpha(ECSEmoteSprite& actor) {
    // Real NativeSprite hooks reach the worker GLES framebuffer and SDK image.
    actor.NativeSprite().AdvanceTime(0);actor.NativeSprite().PrepareDrawFrame();
    auto* image=actor.GetImage();SakuraGL::SGLImageInfo info;size_t alpha=0;
    if(image)if(auto* bytes=image->LockBuffer(info,SakuraGL::SGLImageObject::lockRead)){
        if(info.width==1024&&info.height==768)for(uint32_t y=0;y<info.height;++y)for(uint32_t x=0;x<info.width;++x)
            alpha+=bytes[y*info.pitchLine+x*4+3]!=0;
        image->UnlockBuffer(SakuraGL::SGLImageObject::lockRead);
    }
    return alpha;
}
bool RunSaveProbe(ECSContext& caller,ECSEnvironment& environment) {
    int destroyed=0;
    StateImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    // EmoteSprite is an extension declared by the game's CSX, not an entry in
    // GLS3's built-in type table. Preserve the actual declaration gate in this
    // otherwise empty test image; do not bypass CreateObject with a test factory.
    const auto* declaration=caller.GetClassInfoAs(L"EmoteSprite");
    if(!Check(declaration&&(declaration->GetAttribute()&ECSTypeInfo::flagNativeObject),
        "real CSX declares EmoteSprite as a native class"))return false;
    {
        std::unique_ptr<ECSObject> production(caller.CreateObject(csvtObject,L"EmoteSprite"));
        if(!Check(ESLTypeCast<ECSEmoteSprite>(production.get())!=nullptr,
            "real CSX production factory constructs native EmoteSprite"))return false;
    }
    auto* nativeDeclaration=new ECSClassInfo;
    nativeDeclaration->SetName(declaration->GetName());
    nativeDeclaration->SetGlobalName(declaration->GetGlobalName());
    nativeDeclaration->SetAttribute(declaration->GetAttribute());
    image.AddClassInfo(nativeDeclaration);
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {
        ECSContext& context;ECSExecutionImage& image;ECSContext* previous;ECSWindow* window=nullptr;
        ~Scope(){if(window)window->CloseDisplay();ShutdownLegacySpriteCallbacksForImage(image);context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } scope{context,image,previous};
    if(!Status(context.InitializeContext(&image),"isolated context, caller globals untouched"))return false;
    auto* window=new ECSWindow;image.m_csgData.AddVariable(L"screen",window);scope.window=window;
    if(!Call(context,*window,L"CreateDisplay"))return false;
    auto* device=new ECSEmoteDevice(environment);image.m_csgData.AddVariable(L"emDevice",device);
    if(!Call(context,*device,L"Initialize",{new ECSReference(window)}))return false;
    GraphicsLock graphics;
    auto* actor=new TrackedActor(destroyed);image.m_csgGlobal.AddVariable(L"actor",actor);
    if(!Call(context,*actor,L"LoadPlayer",{new ECSReference(device),new ECSString(L"haz_a.psb")})||
       !Call(context,*actor,L"SetScreenSize",{new ECSInteger(1024),new ECSInteger(768)})||
       !Call(context,*actor,L"SetScale",{new ECSReal(.18),new ECSReal(0)})||
       !Call(context,*actor,L"SetCoord",{new ECSReal(0),new ECSReal(-100),new ECSReal(0)}))return false;
    window->NativeSprite().AddChild(&actor->NativeSprite());
    actor->NativeSprite().SetPosition(31,43);actor->NativeSprite().ChangePriority(-7);
    actor->NativeSprite().SetID(L"EMOTE_SAVE_ACTOR");actor->NativeSprite().SetVisible(true);
    const auto firstAlpha=RenderAlpha(*actor);
    if(!Check(firstAlpha>10000,"original actor has actual rendered alpha"))return false;
    // Original wire stores requested targets before their transition finishes.
    if(!Call(context,*actor,L"SetScale",{new ECSReal(.25),new ECSReal(60)})||
       !Call(context,*actor,L"SetCoord",{new ECSReal(17),new ECSReal(-80),new ECSReal(60)})||
       !Call(context,*actor,L"PlayTimeline",{new ECSString(L"腕切替A")})||
       !Call(context,*actor,L"PostTimeline",{new ECSString(L"腕切替B")})||
       !Call(context,*actor,L"PostTimeline",{new ECSString(L"腕切替C")}))return false;
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();
    EMemoryFile single;Wire before;
    if(!SaveOne(*actor,single,context)||!Inspect(single,context,*device,before)||
       !Check(Expected(before,L"腕切替A",{L"腕切替B",L"腕切替C"}),"target transform + active/queued timeline wire + static device identity"))return false;
    std::vector<uint8_t> one(static_cast<const uint8_t*>(single.GetBuffer()),static_cast<const uint8_t*>(single.GetBuffer())+single.GetLength());
    EMemoryFile forbidden;if(!Status(forbidden.Create(32),"device negative fixture")||
        !Check(device->Save(forbidden,context)!=eslErrSuccess&&forbidden.GetLength()==0,"static device has no serialized object body"))return false;
    EMemoryFile global;
    if(!Status(global.Create(4096),"global graph buffer")||!Status(image.m_csgGlobal.Save(global,context),"serialize global actor with actual static-root references"))return false;
    const auto graphBytes=global.GetLength();
    image.m_csgGlobal.CleanupAllReference(context);image.m_csgGlobal.RemoveAllVariable();
    if(!Check(destroyed==1&&window->NativeSprite().GetChildCount()==0,"original actor destroyed and detached; static roots survive"))return false;
    global.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(image.m_csgGlobal.Load(global,context),"recreate actual EmoteSprite via object factory")||
       !Status(image.m_csgGlobal.CommitAllReference(context),"actual EmoteSprite::Commit reopens NOA player"))return false;
    auto* restored=ESLTypeCast<ECSEmoteSprite>(Named(image.m_csgGlobal,L"actor"));
    if(!Check(restored&&Named(image.m_csgData,L"screen")==window&&Named(image.m_csgData,L"emDevice")==device&&
        ECSObject::GetEntity(restored->GetVariableAt(-11))==device,"same static window/device instances and restored device reference"))return false;
    const auto position=restored->NativeSprite().GetPosition();
    if(!Check(restored->NativeSprite().GetParent()==&window->NativeSprite()&&window->NativeSprite().GetChildCount()==1&&
        position.x==31&&position.y==43&&restored->NativeSprite().GetPriority()==-7&&
        !std::wcscmp(restored->NativeSprite().GetID(),L"EMOTE_SAVE_ACTOR"),"Commit preserves restored Sprite parent/position/priority/ID"))return false;
    if(!Check(Playing(context,*restored,L"腕切替A",true)&&Playing(context,*restored,L"腕切替B",false),"active timeline restarts; queued timeline has not started"))return false;
    const auto restoredAlpha=RenderAlpha(*restored);
    if(!Check(restoredAlpha>10000,"restored actor renders real GLES pixels through NativeSprite"))return false;
    // SaveContext rebuilds these ownership chains before every save. Loading
    // only resolves the references and does not perform this indexing step.
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();
    EMemoryFile second;Wire after;
    if(!SaveOne(*restored,second,context)||!Inspect(second,context,*device,after)||
       !Check(Expected(after,L"腕切替A",{L"腕切替B",L"腕切替C"}),"restored target fields and queue survive second Save"))return false;
    EMemoryFile globalAgain;if(!Status(globalAgain.Create(4096),"second graph buffer")||
       !Status(image.m_csgGlobal.Save(globalAgain,context),"reconstructed global graph saves again"))return false;
    restored->NativeSprite().AdvanceTime(500);
    EMemoryFile advanced;Wire next;
    if(!Check(Playing(context,*restored,L"腕切替A",false)&&Playing(context,*restored,L"腕切替B",true),"real completion starts first queued timeline")||
       !SaveOne(*restored,advanced,context)||!Inspect(advanced,context,*device,next)||
       !Check(Expected(next,L"腕切替B",{L"腕切替C"}),"advanced queue preserves its remaining order"))return false;
    {
        EMemoryFile truncated;truncated.Open(one.data(),one.size()-1);ECSEmoteSprite incomplete;
        if(!Check(incomplete.Load(truncated,context)!=eslErrSuccess,"truncated original EmoteSprite record explicitly fails"))return false;
    }
    {
        auto bad=one;const wchar_t missing[]=L"bad_z.psb";
        if(!Check(StudySteadyLegacyWire::Read32(bad.data()+before.pathOffset)==9,"missing-asset fixture uses actual filename field"))return false;
        for(size_t i=0;i<9;++i){bad[before.pathOffset+4+2*i]=static_cast<uint8_t>(missing[i]);bad[before.pathOffset+5+2*i]=0;}
        EMemoryFile missingFile;missingFile.Open(bad.data(),bad.size());ECSEmoteSprite missingActor;
        if(!Status(missingActor.Load(missingFile,context),"missing asset record remains structurally valid")||
           !Check(missingActor.CommitAllReference(context)!=eslErrSuccess,"missing PSB fails during actual player reconstruction"))return false;
    }
    if(!Check(Named(image.m_csgGlobal,L"actor")==restored&&window->NativeSprite().GetChildCount()==1&&
        RenderAlpha(*restored)>10000,"negative fixtures leave the good actor and static scene usable"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
        "Emote Sprite save probe PASS: graph_bytes=%llu first_alpha=%zu restored_alpha=%zu; global destroy/rebuild, static refs, original transform/queue wire, second save, real render, truncated/missing errors",
        static_cast<unsigned long long>(graphBytes),firstAlpha,restoredAlpha);
    return true;
}
}

bool CheckLegacyEmoteWindow(ECSContext& caller,ECSEnvironment& environment) {
    const auto* originalImage=caller.m_pcsxi;const auto originalStatus=caller.GetStatus();
    auto* originalPrimary=ECotophaScript::GetPrimaryContext();
    const auto globals=originalImage?originalImage->m_csgGlobal.m_varArray.GetSize():0;
    const auto statics=originalImage?originalImage->m_csgData.m_varArray.GetSize():0;
    bool passed=false;
    try {passed=RunSaveProbe(caller,environment);}catch(const std::exception& error){
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Emote Sprite save probe exception: %s",error.what());
    }
    return Check(caller.m_pcsxi==originalImage&&caller.GetStatus()==originalStatus&&ECotophaScript::GetPrimaryContext()==originalPrimary&&
        (!originalImage||(originalImage->m_csgGlobal.m_varArray.GetSize()==globals&&originalImage->m_csgData.m_varArray.GetSize()==statics)),
        "caller context, globals, statics and primary-context pointer preserved")&&passed;
}
