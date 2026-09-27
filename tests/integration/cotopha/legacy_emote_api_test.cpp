// Synthetic VM declarations and memory files only; no PSB, DLL, game, or GL context.
#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_emote.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "platform/sdl/system.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Bytes=std::vector<std::uint8_t>;
void Require(bool value,const char* why) { if (!value) throw std::runtime_error(why); }

struct Fixture {
    ECSExecutionImage image;
    ECSContext context;
    explicit Fixture(bool extended) {
        auto* cls=new ECSClassInfo;
        cls->SetName(L"EmoteSprite");cls->SetGlobalName(L"EmoteSprite");
        cls->SetAttribute(ECSTypeInfo::flagNativeObject);
        if (extended) {
            auto* method=new ECSClassInfo::MemberFunction;
            method->SetName(L"DefaultTimeline");method->SetGlobalName(L"EmoteSprite::DefaultTimeline");
            method->SetAttribute(ECSTypeInfo::flagNativeObject);
            method->m_wstrClass=L"EmoteSprite";
            cls->AddFunction(method);
        }
        image.AddClassInfo(cls);
        context.m_pcsxi=&image;
    }
    ~Fixture() {
        ShutdownLegacySpriteCallbacksForImage(image);
        context.m_pcsxi=nullptr;
    }
};

INT64 Invoke(ECSContext& context,ECSEmoteSprite& actor,const wchar_t* name,
             std::initializer_list<ECSObject*> parameters={}) {
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&actor));
    for (auto* p:parameters) args.Add(p);
    int index=-1;
    Require(!actor.GetFunction(context,index,name),"Emote method did not resolve");
    Require(!actor.CallFunction(context,index,args),"Emote method raised a VM error");
    std::unique_ptr<ECSObject> result(context.PopObject());
    INT64 status=-1;
    Require(result && !result->OperateInteger(status),"Emote method did not return Integer");
    return status;
}
void Reject(ECSContext& context,ECSEmoteSprite& actor,const wchar_t* name,
            std::initializer_list<ECSObject*> parameters={}) {
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&actor));
    for (auto* p:parameters) args.Add(p);
    int index=-1;
    Require(!actor.GetFunction(context,index,name),"invalid-call method did not resolve");
    Require(actor.CallFunction(context,index,args)!=eslErrSuccess,"invalid Emote argument was accepted");
}

Bytes Contents(EMemoryFile& file) {
    const auto* begin=static_cast<const std::uint8_t*>(file.GetBuffer());
    return {begin,begin+file.GetLength()};
}
Bytes Save(ECSEmoteSprite& actor,ECSContext& context) {
    EMemoryFile file;Require(!file.Create(1024),"memory file creation failed");
    Require(!actor.Save(file,context),"Emote Save failed");
    return Contents(file);
}
void Load(ECSEmoteSprite& actor,ECSContext& context,const Bytes& bytes) {
    EMemoryFile file;Require(!file.Create(bytes.size()),"load memory file creation failed");
    Require(file.Write(bytes.data(),bytes.size())==bytes.size(),"fixture write failed");
    file.SeekLarge(0,ESLFileObject::FromBegin);
    Require(!actor.Load(file,context),"Emote Load failed");
    Require(file.GetPosition()==file.GetLength(),"Emote Load did not consume precisely one record");
    Require(!actor.CommitAllReference(context),"empty-Player record commit failed");
}
void RejectWire(ECSContext& context,const Bytes& bytes) {
    ECSEmoteSprite actor;EMemoryFile file;
    Require(!file.Create(bytes.size()),"invalid-wire memory file creation failed");
    Require(file.Write(bytes.data(),bytes.size())==bytes.size(),"invalid fixture write failed");
    file.SeekLarge(0,ESLFileObject::FromBegin);
    Require(actor.Load(file,context)!=eslErrSuccess,"invalid Emote wire accepted");
}

// A real base record and null device reference isolate the extension under test.
// The suffix below is an independent fixed-width fixture for the original wire,
// not a call to the production Emote serializer or its scalar/string helpers.
Bytes Prefix(ECSContext& context) {
    EMemoryFile file;Require(!file.Create(1024),"prefix memory file creation failed");
    ECSSprite sprite;ECSReference nullDevice;
    Require(!sprite.Save(file,context) && !nullDevice.Save(file,context),"base/null-reference prefix failed");
    return Contents(file);
}
void U32(Bytes& bytes,std::uint32_t value) {
    for (unsigned i=0;i<4;++i) bytes.push_back(static_cast<std::uint8_t>(value>>(8*i)));
}
void U64(Bytes& bytes,std::uint64_t value) {
    for (unsigned i=0;i<8;++i) bytes.push_back(static_cast<std::uint8_t>(value>>(8*i)));
}
void Text(Bytes& bytes,const std::u16string& value) {
    U32(bytes,static_cast<std::uint32_t>(value.size()));
    for (char16_t c:value) {bytes.push_back(c&255);bytes.push_back(c>>8);}
}
struct State {
    std::uint32_t width=320,height=240;
    std::uint32_t scale=0x3f400000,x=0x41480000,y=0xc0e00000; // .75, 12.5, -7
    std::uint32_t flags=0;
    std::uint64_t weight=0x3fe4000000000000ULL; // .625
    std::u16string fallback=u"idle-\u03a9",current;
    std::vector<std::u16string> queue;
};
Bytes Wire(const Bytes& prefix,bool extended,const State& state) {
    Bytes bytes=prefix;
    U32(bytes,state.width);U32(bytes,state.height);
    U32(bytes,state.scale);U32(bytes,state.x);U32(bytes,state.y);
    // Original newer Save 0xC8C3A0 / Load 0xC8ADD0: after the unchanged
    // 20-byte transform, reserved=0, flags, F64 weight, path, default, current,
    // queue. Older records have only path/current/queue after the transform.
    if (extended) {U32(bytes,0);U32(bytes,state.flags);U64(bytes,state.weight);}
    Text(bytes,u""); // Empty filename makes Commit independent of Player/PSB.
    if (extended) Text(bytes,state.fallback);
    Text(bytes,state.current);
    U32(bytes,static_cast<std::uint32_t>(state.queue.size()));
    for (const auto& name:state.queue) Text(bytes,name);
    return bytes;
}
void Replace32(Bytes& bytes,std::size_t offset,std::uint32_t value) {
    for (unsigned i=0;i<4;++i) bytes.at(offset+i)=static_cast<std::uint8_t>(value>>(8*i));
}

void Run() {
    struct Runtime {Runtime(){ECotophaScript::Initialize(0);}~Runtime(){ECotophaScript::Release();}} runtime;
    Fixture oldAbi(false),newAbi(true);
    ECSEmoteSprite actor;
    const wchar_t* names[]={L"LoadPlayer",L"ReleasePlayer",L"SetScreenSize",L"SetScale",L"SetCoord",
        L"SetPhysWeight",L"EnablePhysAnimation",L"PlayTimeline",L"PostTimeline",L"DefaultTimeline",
        L"SkipTimeline",L"IsPlayingTimeline",L"AttachVoiceSync"};
    std::set<int> methods;
    for (const auto* name:names) {
        int index=-1;Require(!actor.GetFunction(newAbi.context,index,name),"one of 13 Emote APIs failed to resolve");
        Require(index>=4096 && methods.insert(index).second,"Emote method dispatch aliases another interface");
    }
    int unknown=-1;Require(actor.GetFunction(newAbi.context,unknown,L"MissingEmoteMethod")!=eslErrSuccess,
                           "unknown native method was accepted");
    auto& context=newAbi.context;
    Require(Invoke(context,actor,L"SetScreenSize",{new ECSInteger(320),new ECSInteger(240)})==0,"screen configuration failed");
    Require(Invoke(context,actor,L"SetScale",{new ECSReal(.75)})==0,"scale configuration without Player failed");
    Require(Invoke(context,actor,L"SetCoord",{new ECSReal(12.5),new ECSReal(-7)})==0,"coordinate configuration failed");
    Require(Invoke(context,actor,L"DefaultTimeline",{new ECSString(L"idle-\u03a9")})==0,"default timeline without Player failed");
    Require(Invoke(context,actor,L"SetPhysWeight",{new ECSReal(.625)})==0,"physics weight without Player failed");
    Require(Invoke(context,actor,L"EnablePhysAnimation",{new ECSInteger(0)})==0,"physics disable without Player failed");
    Require(Invoke(context,actor,L"SkipTimeline")==0,"skip without Player failed");
    State state;
    const auto prefix=Prefix(context);
    const auto modern=Wire(prefix,true,state),legacy=Wire(prefix,false,state);
    Require(Save(actor,context)==modern,"configured modern wire differs from original field order/widths");
    // Save never depends on which method was resolved or called previously.
    Require(Save(actor,oldAbi.context)==legacy,"old metadata did not select the unextended wire");
    Require(Save(actor,context)==modern,"ABI selection was incorrectly latched to the prior Save");
    Require(modern.size()-legacy.size()==20+2*state.fallback.size(),"new wire has an unexpected extension length");
    Require(Invoke(context,actor,L"EnablePhysAnimation",{new ECSInteger(-1)})==0,"physics re-enable without Player failed");
    state.flags=1;Require(Save(actor,context)==Wire(prefix,true,state),"enabled state was not serialized");

    Reject(context,actor,L"DefaultTimeline");
    Reject(context,actor,L"SetPhysWeight");
    Reject(context,actor,L"SetPhysWeight",{new ECSReal(std::numeric_limits<double>::infinity())});
    Reject(context,actor,L"SetPhysWeight",{new ECSReal(std::numeric_limits<double>::quiet_NaN())});
    Reject(context,actor,L"EnablePhysAnimation",{new ECSInteger(1),new ECSInteger(2)});
    Reject(context,actor,L"SkipTimeline",{new ECSInteger(1)});
    Reject(context,actor,L"SetScreenSize",{new ECSInteger(0),new ECSInteger(240)});
    Reject(context,actor,L"SetScale",{new ECSReal(std::numeric_limits<double>::infinity())});
    Reject(context,actor,L"SetCoord",{new ECSReal(0),new ECSReal(std::numeric_limits<double>::quiet_NaN())});
    Reject(context,actor,L"LoadPlayer",{new ECSString(L"not a device"),new ECSString(L"unused.psb")});

    // Distinct text lengths catch default/current/queue shifts. The exact F64
    // value is deliberately not representable as float and must survive Load.
    state.flags=0;state.weight=0x3ff0000000000001ULL;
    state.current=u"main-A";state.queue={u"queued-first",u"B",u"last-\u03a9"};
    const auto newRecord=Wire(prefix,true,state),oldRecord=Wire(prefix,false,state);
    ECSEmoteSprite loadedNew,loadedOld;
    Load(loadedNew,context,newRecord);
    Require(Save(loadedNew,context)==newRecord,"modern record did not round-trip F64/default/current/queue exactly");
    Load(loadedOld,oldAbi.context,oldRecord);
    Require(Save(loadedOld,oldAbi.context)==oldRecord,"legacy record gained extension bytes or shifted queue data");
    Require(Save(loadedNew,oldAbi.context)==oldRecord,"same object did not serialize correctly in old ABI context");
    Require(Save(loadedNew,context)==newRecord,"old ABI serialization mutated independent modern state");
    {
        std::unique_ptr<ECSObject> copy(loadedNew.Duplicate());
        auto* duplicated=ESLTypeCast<ECSEmoteSprite>(copy.get());
        Require(duplicated && Save(*duplicated,context)==newRecord,"Duplicate lost unloaded default/physics/queue state");
    }
    ECSEmoteSprite moved;
    Require(!moved.Move(context,new ECSReference(&loadedNew)),"Move failed for an unloaded synthetic actor");
    Require(Save(moved,context)==newRecord,"Move lost extended state");
    Require(!loadedNew.Release(),"Release failed");
    State released=state;released.scale=0x3f800000;released.x=released.y=0;
    released.flags=1;released.weight=0x3ff0000000000000ULL;
    released.fallback.clear();released.current.clear();released.queue.clear();
    Require(Save(loadedNew,context)==Wire(prefix,true,released),"Release did not reset default/physics/timeline state");

    auto corrupt=newRecord;Replace32(corrupt,prefix.size()+20,1);RejectWire(context,corrupt);
    corrupt=newRecord;Replace32(corrupt,prefix.size()+24,2);RejectWire(context,corrupt);
    corrupt=newRecord;Replace32(corrupt,prefix.size()+32,0x7ff00000);RejectWire(context,corrupt); // nonfinite F64
    corrupt=newRecord;Replace32(corrupt,prefix.size(),0);RejectWire(context,corrupt);
    corrupt=newRecord;corrupt.pop_back();RejectWire(context,corrupt);
    corrupt=oldRecord;corrupt.pop_back();RejectWire(oldAbi.context,corrupt);
}
}

int main() {
    if (!SDL_Init(0)) return 2;
    namespace fs=std::filesystem;
    fs::path root;bool initialized=false,created=false;int result=0;
    try {
        root=fs::temp_directory_path()/("entis-emote-api-test-"+
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        created=fs::create_directory(root);Require(created,"isolated fixture directory creation failed");
        for (const char* name:{"assets","storage","local","game"}) fs::create_directory(root/name);
        study::platform::sdl::SystemPaths paths;
        paths.assetsRoot=(root/"assets").string();paths.storageRoot=(root/"storage").string();
        paths.localRoot=(root/"local").string();paths.gameRoot=(root/"game").string();
        Require(study::platform::sdl::ConfigureSystemPaths(paths),"isolated fixture paths rejected");
        SakuraGL::Initialize();initialized=true;
        Run();
        std::puts("Legacy Emote API PASS: 13 methods, unloaded configuration, original old/new wire, exact F64 and timeline queue round trips, copy/move/release, malformed arguments and records");
    } catch (const std::exception& error) {
        std::fprintf(stderr,"Legacy Emote API FAIL: %s\n",error.what());result=1;
    }
    if (initialized) SakuraGL::Finalize();
    SDL_Quit();
    if (created) {std::error_code ignored;fs::remove_all(root,ignored);}
    return result;
}
