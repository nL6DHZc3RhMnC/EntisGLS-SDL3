#include "legacy_compat/gls.h"
#include "legacy_sprite_callbacks.h"
#include "legacy_sprite.h"
#include "platform/log.h"
#include <array>
#include <cstring>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace {
struct GraphicsGuard { GraphicsGuard(){SSystem::Lock();} ~GraphicsGuard(){SSystem::Unlock();} };
struct CallbackExecution {
    ECSContext context;
    bool entered=false;
};
std::map<ECSExecutionImage*,std::shared_ptr<CallbackExecution>> executions;
std::map<const SakuraGL::SGLSprite*,std::weak_ptr<LegacySpriteCallbackState>> sprites;
constexpr const wchar_t* names[]={L"SetHitTestProcedure",L"SetTimerProcedure",L"SetMouseInterface",L"SetKeyInterface"};
constexpr const wchar_t* types[]={L"SpriteHitTestProcedure",L"SpriteTimerProcedure",L"SpriteMouseInterface",L"SpriteKeyInterface"};
void Report(ESLError error,const char* stage,int kind,int ordinal,uint32_t ip) {
    study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
        "Cotopha sprite callback %s: kind=%d ordinal=%d ip=0x%08x error=%s",
        stage,kind,ordinal,ip,GetESLErrorMsg(error));
}
}

struct LegacySpriteCallbackState {
    ECSSprite* owner=nullptr;
    ECSExecutionImage* image=nullptr;
    std::array<ECSReference,4> hooks;
    bool enabled=true;
    ~LegacySpriteCallbackState() {
        // Reference ownership can migrate from a disappearing temporary to a
        // callback backlink. ECSReference's destructor expects that ownership
        // to have already been discharged. Context graph cleanup normally does
        // this with its ECSContext; native-only destruction needs this fallback.
        for(auto& hook:hooks) hook.SetReference(nullptr,nullptr);
    }
};

ESLError HandleLegacySpriteCallbacks(ECSContext& context,ECSSprite& sprite,
                                    const wchar_t* name,ECSObjArray<ECSObject>& args,bool& handled) {
    int kind=0;
    while(kind<4 && std::wcscmp(name,names[kind])) ++kind;
    handled=kind<4;
    if(!handled) return eslErrSuccess;
    auto error=context.VerifyArgumentCount(args,2);
    if(error) return error;
    if(!context.m_pcsxi) return ESLErrorMsg("Sprite callback requires an initialized execution image");
    GraphicsGuard guard;
    auto& state=sprite.LegacyCallbackState();
    if(!state) {
        state=std::make_shared<LegacySpriteCallbackState>();state->owner=&sprite;
        state->image=context.m_pcsxi;sprites[&sprite.NativeSprite()]=state;
    }
    if(state->image!=context.m_pcsxi) return ESLErrorMsg("Sprite callback cannot cross execution images");
    ECSObject* hook=context.GetArgumentObjectAs(args,1,types[kind]);
    if(hook) {
        ECS_CAST_INTERFACE cast;
        error=hook->OperateCastInterface(cast,types[kind]);
        if(!error) state->hooks[kind].SetReferenceCastInterface(cast.pCastObject,&context,cast);
    } else {
        // Null is the legacy unsubscribe operation. A non-null incompatible value
        // is a type error, rather than silently discarding the existing callback.
        if(ECSObject::GetEntity(args.GetAt(1))) return ESLErrorMsg("Sprite callback interface type mismatch");
        state->hooks[kind].SetReference(nullptr,&context);
    }
    state->enabled=true;
    return context.PushObject(context.new_CSInteger(error));
}

bool InvokeLegacySpriteCallback(const SakuraGL::SGLSprite* native,LegacySpriteCallback category,
                               int ordinal,double a,double b,double c,bool* attached) {
    GraphicsGuard guard;
    if(attached) *attached=false;
    const auto found=sprites.find(native);
    if(found==sprites.end()) return false;
    auto state=found->second.lock();
    const int kind=int(category);
    if(!state || !state->enabled || !state->owner || !state->image
        || !ECSObject::IsValidObject(state->hooks[kind].m_pRef)) return false;
    if(attached) *attached=true;
    auto& slot=executions[state->image];
    if(!slot) {
        auto fresh=std::make_shared<CallbackExecution>();
        const auto error=fresh->context.InitializeContext(state->image,false);
        if(error) {Report(error,"initialize",kind,ordinal,0);return false;}
        slot=std::move(fresh);
    }
    auto execution=slot; // Retain the context if a callback releases its sprite.
    auto& context=execution->context;
    if(execution->entered) {
        Report(ESLErrorMsg("recursive sprite callback is not supported by the legacy runtime"),"reentry",kind,ordinal,context.m_ip);
        return false;
    }
    struct Entered { bool& value; Entered(bool& v):value(v){value=true;} ~Entered(){value=false;} } entered(execution->entered);
    ECS_FUNCTION_POINTER function;
    auto error=state->hooks[kind].GetFunctionPointer(context,function,ordinal);
    if(!error && function.m_ftType!=ECS_FUNCTION_POINTER::funcScriptCall)
        error=ESLErrorMsg("Sprite callback must resolve to a Cotopha object function");
    if(error) {Report(error,"resolve",kind,ordinal,context.m_ip);return false;}

    ECSArray arguments;
    auto* self=context.new_CSReference();
    self->SetReferenceCastInterface(function.m_castThis.pCastObject,&context,function.m_castThis);
    arguments.m_varArray.Add(self);
    auto* sprite=context.new_CSReference();sprite->SetReference(state->owner,&context);
    arguments.m_varArray.Add(sprite);
    auto add=[&](INT64 value){arguments.m_varArray.Add(context.new_CSInteger(value));};
    auto position=[&](double x,double y){
        // GLS3 callbacks receive coordinates in their parent's coordinate space;
        // GLS4 has already transformed input into this sprite's local space.
        SakuraGL::S2DDVector point(x,y);native->LocalToGlobal(point);
        add(static_cast<int32_t>(point.x));add(static_cast<int32_t>(point.y));
    };
    const bool voidResult=category==LegacySpriteCallback::Timer
        || (category==LegacySpriteCallback::Mouse && ordinal==1);
    if(category==LegacySpriteCallback::HitTest) position(a,b);
    else if(category==LegacySpriteCallback::Timer || category==LegacySpriteCallback::Key) add(static_cast<int32_t>(a));
    else if(ordinal==2) {add(static_cast<int32_t>(a)/SakuraGL::SGLSprite::WheelDeltaUnit);position(b,c);}
    else if(ordinal!=1) position(a,b);

    const auto savedIP=context.m_ip;
    const auto stackSize=context.m_stack.m_varArray.GetSize();
    context.PushObject(context.new_CSInteger(-1));
    error=context.CallFunction(static_cast<DWORD>(function.m_varFunc.addrScript),arguments.m_varArray);
    bool result=false;
    ECSObject* returned=nullptr;
    if(!error && context.m_stack.m_varArray.GetSize()>stackSize) returned=context.PopObject();
    if(!error && !voidResult) {
        int boolean=0;
        error=returned?returned->OperateBoolean(boolean):ESLErrorMsg("Sprite callback returned no Boolean value");
        result=!error && boolean!=0;
    }
    if(error) Report(error,"execute",kind,ordinal,context.m_ip);
    context.delete_CSObject(returned);
    context.m_stack.RemoveBetween(context,stackSize);
    context.m_arg.RemoveBetween(context);
    arguments.RemoveBetween(context);
    context.m_ip=savedIP;
    return result;
}

void ResetLegacySpriteCallbacks(ECSSprite& sprite) {
    GraphicsGuard guard;
    sprites.erase(&sprite.NativeSprite());
    auto state=std::move(sprite.LegacyCallbackState());
    if(state) {state->enabled=false;state->owner=nullptr;state->image=nullptr;}
}
void IndexLegacySpriteCallbacks(ECSSprite& sprite) {
    GraphicsGuard guard;auto state=sprite.LegacyCallbackState();if(!state)return;
    for(int i=0;i<4;++i) {auto& ref=state->hooks[i];ref.IndexAllMember();ref.m_pParent=&sprite;ref.m_nIndex=-7-i;}
}
ECSObject* GetLegacySpriteCallbackAt(ECSSprite& sprite,int index) {
    GraphicsGuard guard;auto state=sprite.LegacyCallbackState();
    return state && index<=-7 && index>=-10?&state->hooks[-7-index]:nullptr;
}
ESLError SaveLegacySpriteCallbacks(ESLFileObject& file,ECSSprite& sprite,ECSContext& context) {
    GraphicsGuard guard;auto state=sprite.LegacyCallbackState();ECSReference empty;
    for(int i=0;i<4;++i)if(const auto error=(state?state->hooks[i]:empty).Save(file,context))return error;
    return eslErrSuccess;
}
ESLError LoadLegacySpriteCallbacks(ESLFileObject& file,ECSSprite& sprite,ECSContext& context) {
    GraphicsGuard guard;
    ResetLegacySpriteCallbacks(sprite);
    auto state=std::make_shared<LegacySpriteCallbackState>();state->owner=&sprite;state->image=context.m_pcsxi;state->enabled=false;
    for(auto& hook:state->hooks)if(const auto error=hook.Load(file,context))return error;
    sprite.LegacyCallbackState()=state;sprites[&sprite.NativeSprite()]=state;return eslErrSuccess;
}
void CleanupLegacySpriteCallbacks(ECSSprite& sprite,ECSContext& context) {
    GraphicsGuard guard;auto state=sprite.LegacyCallbackState();if(!state)return;
    state->enabled=false;for(auto& hook:state->hooks)hook.CleanupAllReference(context);
}
ESLError CommitLegacySpriteCallbacks(ECSSprite& sprite,ECSContext& context) {
    GraphicsGuard guard;auto state=sprite.LegacyCallbackState();if(!state)return eslErrSuccess;
    state->enabled=false;
    for(auto& hook:state->hooks){auto error=hook.CommitAllReference(context);if(error)return error;}
    state->image=context.m_pcsxi;state->enabled=true;return eslErrSuccess;
}
void ShutdownLegacySpriteCallbacksForImage(ECSExecutionImage& image) {
    GraphicsGuard guard;
    for(auto& entry:sprites)if(auto state=entry.second.lock())if(state->image==&image){state->enabled=false;state->image=nullptr;}
    executions.erase(&image);
}

namespace {
class CallbackProbeImage final : public ECSExecutionImage {
public:
    DWORD addresses[4][2]{};
    CallbackProbeImage() {
        std::vector<BYTE> code;
        auto byte=[&](unsigned value){code.push_back(BYTE(value));};
        auto word=[&](uint32_t value){for(int n=0;n<4;++n)byte(value>>(n*8));};
        auto string=[&](const wchar_t* text){word(std::wcslen(text));while(*text){byte(*text);byte(*text++>>8);}};
        for(unsigned count=0;count<4;++count)for(unsigned result=0;result<2;++result) {
            addresses[count][result]=code.size();
            byte(csicEnter);string(L"callbackProbe");word(count+2);
            byte(csvtReference);string(L"this");byte(csvtReference);string(L"sprite");
            constexpr const wchar_t* parameters[]={L"a",L"b",L"c"};
            for(unsigned i=0;i<count;++i){byte(csvtInteger);string(parameters[i]);}
            for(unsigned i=0;i<count;++i) {
                byte(csicLoad);byte(csomGlobal);byte(csvtInteger);word(i);
                byte(csicLoad);byte(csomStack);byte(csvtString);string(parameters[i]);
                byte(csicStore);byte(0xff);byte(1);
            }
            byte(csicLoad);byte(csomImmediate);byte(csvtInteger);word(result?42:0);
            byte(csicExReturn);byte(1);
        }
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnEntryPoint=UINT32_MAX;m_exiHeader.fnStaticInitialize=UINT32_MAX;
        m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(code.size()),code.data(),code.size());m_bufImage.Flush(code.size());
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,code.size()));m_dwImageSize=code.size();
    }
};
class CallbackProbeHook final : public ECSInteger {
public:
    CallbackProbeImage& image;
    int kind,calls=0,lastOrdinal=-1;
    bool answer=true;
    ECSContext* lastContext=nullptr;
    bool* destroyed=nullptr;
    CallbackProbeHook(CallbackProbeImage& image,int kind):image(image),kind(kind){}
    ~CallbackProbeHook() override {if(destroyed)*destroyed=true;}
    ECSObject* GetTypeOf(const wchar_t* name) override {return !std::wcscmp(name,types[kind])?this:ECSInteger::GetTypeOf(name);}
    ESLError GetFunctionPointer(ECSContext& context,ECS_FUNCTION_POINTER& function,int ordinal) override {
        ++calls;lastOrdinal=ordinal;lastContext=&context;
        int count=kind==0?2:kind==1||kind==3?1:ordinal==1?0:ordinal==2?3:2;
        function.m_ftType=ECS_FUNCTION_POINTER::funcScriptCall;
        function.m_castThis=ECS_CAST_INTERFACE(this);
        function.m_varFunc.addrScript=image.addresses[count][answer?1:0];return eslErrSuccess;
    }
};
}

bool CheckLegacySpriteCallbacks() {
    auto check=[](bool value,const char* message){
        study::platform::LogPrint(value?study::platform::LogPriority::Info:study::platform::LogPriority::Error,"StudySteady",
            "Legacy sprite callback probe %s: %s",value?"PASS":"FAIL",message);return value;
    };
    ECSEnvironment environment;
    CallbackProbeImage image;image.AttachCSEnvironment(&environment);
    ECSContext mainContext;
    struct Release {
        ECSContext& context;ECSExecutionImage& image;ECSContext* primary=ECotophaScript::GetPrimaryContext();
        ~Release(){ShutdownLegacySpriteCallbacksForImage(image);context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(primary);}
    } release{mainContext,image};
    if(!check(!mainContext.InitializeContext(&image),"initialize actual object bytecode"))return false;
    ECSInteger* values[3];
    for(int i=0;i<3;++i){values[i]=new ECSInteger(-1);wchar_t name[]={wchar_t(L'a'+i),0};image.m_csgGlobal.AddVariable(name,values[i]);}
    mainContext.PushObject(mainContext.new_CSInteger(777));
    ECSSprite sprite;
    sprite.NativeSprite().SetPosition(10,20);
    CallbackProbeHook hit(image,0),timer(image,1),mouse(image,2),key(image,3);
    auto bind=[&](int kind,ECSObject* object){
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&sprite));args.Add(new ECSReference(object));
        bool handled=false;auto error=HandleLegacySpriteCallbacks(mainContext,sprite,names[kind],args,handled);
        if(error || !handled)return false;
        std::unique_ptr<ECSObject> result(mainContext.PopObject());INT64 value=-1;
        return result && !result->OperateInteger(value) && value==0;
    };
    if(!check(bind(0,&hit)&&bind(1,&timer)&&bind(2,&mouse)&&bind(3,&key),"bind four typed interfaces"))return false;
    if(!check(sprite.NativeSprite().OnMouseMove(3,4,0)&&mouse.calls==1&&mouse.lastOrdinal==0
        &&values[0]->GetValue()==13&&values[1]->GetValue()==24,"mouse executes script with parent coordinates"))return false;
    const auto count=mouse.calls;
    if(!check(sprite.NativeSprite().OnButtonUp(5,6,0)&&mouse.calls==count+1&&mouse.lastOrdinal==4,
        "left button up executes correct virtual method"))return false;
    sprite.NativeSprite().OnMouseLeave(0);
    if(!check(mouse.lastOrdinal==1,"mouse leave callback without coordinate arguments"))return false;
    if(!check(sprite.NativeSprite().OnMouseWheel(512,3,4,0)&&mouse.lastOrdinal==2
        &&values[0]->GetValue()==2&&values[1]->GetValue()==13&&values[2]->GetValue()==24,
        "wheel delta converts to legacy notches"))return false;
    sprite.NativeSprite().AdvanceTime(17);
    if(!check(timer.calls==1&&values[0]->GetValue()==17,"timer executes actual object function"))return false;
    if(!check(sprite.NativeSprite().IsHitSprite(2,3)&&hit.calls==1,"hit test true result"))return false;
    hit.answer=false;
    if(!check(!sprite.NativeSprite().IsHitSprite(2,3)&&hit.calls==2,"hit test false does not fall through"))return false;
    bool workerResult=false;
    std::thread worker([&]{workerResult=sprite.NativeSprite().OnKeyDown(65,0)&&sprite.NativeSprite().OnKeyUp(65,0);});
    worker.join();
    if(!check(workerResult&&key.calls==2&&key.lastOrdinal==1&&key.lastContext!=&mainContext
        &&mainContext.m_stack.m_varArray.GetSize()==1&&values[0]->GetValue()==65,
        "worker input uses separate callback stack and preserves main stack"))return false;
    IndexLegacySpriteCallbacks(sprite);
    if(!check(GetLegacySpriteCallbackAt(sprite,-9)&&GetLegacySpriteCallbackAt(sprite,-9)->m_pParent==&sprite
        &&GetLegacySpriteCallbackAt(sprite,-9)->m_nIndex==-9,"callback references have stable legacy indices"))return false;
    const auto before=mouse.calls;
    if(!check(bind(2,nullptr),"clear interface using null"))return false;
    sprite.NativeSprite().OnMouseMove(3,4,0);
    if(!check(mouse.calls==before,"unbound interface receives no event"))return false;
    if(!check(bind(2,&mouse),"rebind interface"))return false;
    sprite.Release();
    if(!check(sprite.NativeSprite().OnMouseMove(3,4,0)&&mouse.calls==before+1,
        "buffer Release preserves script callback binding"))return false;
    const auto countAfterRelease=mouse.calls;
    ResetLegacySpriteCallbacks(sprite);ResetLegacySpriteCallbacks(sprite);
    sprite.NativeSprite().OnMouseMove(3,4,0);
    if(!check(mouse.calls==countAfterRelease,"repeated callback reset removes native routing"))return false;
    bool destroyed=false;
    {
        ECSReference owner;
        auto* owned=new CallbackProbeHook(image,2);owned->destroyed=&destroyed;
        owner.SetOwnObject(owned,&mainContext);
        if(!check(bind(2,owned),"bind temporarily owned callback object"))return false;
        owner.SetOwnObject(nullptr,&mainContext);
        if(!check(!destroyed,"callback reference accepts transferred ownership"))return false;
        ResetLegacySpriteCallbacks(sprite);
        if(!check(destroyed,"callback reset destroys transferred owned object"))return false;
    }
    if(!check(bind(2,&mouse),"bind before image shutdown"))return false;
    ShutdownLegacySpriteCallbacksForImage(image);
    sprite.NativeSprite().OnMouseMove(3,4,0);
    return check(mouse.calls==countAfterRelease,"image shutdown stops callbacks before context destruction");
}
