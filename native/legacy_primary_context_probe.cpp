#include "legacy_compat/gls.h"
#include "legacy_primary_context_probe.h"
#include "platform/log.h"
#include <cstring>

bool CheckLegacyPrimaryReferenceState(ECSEnvironment& environment) {
    class Image final:public ECSExecutionImage {
    public:Image(){
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }} image;
    image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope{ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}}scope{context,previous};
    auto check=[](bool good,const char* step){study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady","Primary reference probe %s: %s",good?"OK":"FAIL",step);return good;};
    if(!check(!context.InitializeContext(&image),"isolated actual ECS context"))return false;
    context.m_stack.CreateNameBlock(context.GetConstantString(L"DispatchCommand"));
    auto* receiver=new ECSArray;receiver->m_varArray.Add(new ECSInteger(11));receiver->m_varArray.Add(new ECSInteger(42));
    context.m_stack.CreateNewVariable(L"uisave",receiver);
    context.m_stack.IndexAllMember();
    EMemoryFile saved;if(saved.Create(128))return false;
    ECSReference live(receiver);if(!check(!live.Save(saved,context),"serialize local receiver reference"))return false;
    auto load=[&](ECSReference& ref){saved.SeekLarge(0,ESLFileObject::FromBegin);return ref.Load(saved,context);};
    ECSReference restored;
    if(!check(!load(restored)&&!restored.CommitAllReference(context)&&ECSObject::GetEntity(&restored)==receiver,"first real local reference resolution"))return false;
    if(!check(!restored.CommitAllReference(context)&&ECSObject::GetEntity(&restored)==receiver,"second commit retains receiver instead of Stack root"))return false;
    int member=0;INT64 value=0;
    if(!check(!restored.GetVariableIndex(member,1)&&restored.GetVariableAt(member)&&!restored.GetVariableAt(member)->OperateInteger(value)&&value==42,"this[1] remains actual receiver member"))return false;
    EMemoryFile ownWire;if(ownWire.Create(256))return false;
    ECSReference owned;auto* content=new ECSArray;
    auto* child=new ECSReference(receiver);
    content->m_varArray.Add(child);
    if(owned.Move(context,content)||owned.Save(ownWire,context))return false;
    ownWire.SeekLarge(0,ESLFileObject::FromBegin);ECSReference copy;
    if(!check(!copy.Load(ownWire,context)&&!copy.CommitAllReference(context),"actual owned graph initial commit"))return false;
    auto* array=ESLTypeCast<ECSArray>(copy.m_pOwnObj);
    auto* pending=new ECSReference;if(load(*pending)){delete pending;return false;}
    array->m_varArray.Add(pending);
    if(!check(!copy.CommitAllReference(context),"repeat owner recursively commits newly loaded child"))return false;
    const bool backlink=copy.m_pNextBackRef!=&copy&&copy.m_pPrevBackRef!=&copy;
    if(!backlink){ // Keep a failing regression probe's destructor finite.
        copy.m_pNextBackRef=copy.m_pPrevBackRef=nullptr;copy.m_pOwnObj->m_pBackRef=&copy;
    }
    if(!check(backlink&&copy.m_pOwnObj->m_pBackRef==&copy&&ECSObject::GetEntity(pending)==receiver&&
        ECSObject::GetEntity(array->m_varArray.GetAt(0))==receiver,"owner backlink has no self-cycle; old and new children both retain receiver"))return false;
    // The original Array/Hash traversal tolerates absent static cache targets,
    // then the game's OnContextLoaded script rebuilds that cache. It must still
    // visit later children and the saved default element.
    auto* cache = new ECSHash;
    image.m_csgData.AddVariable(L"cache",cache);
    auto fillCache=[&](int value) {
        auto* data=new ECSArray;auto* leaf=new ECSInteger(value);
        data->m_varArray.Add(leaf);cache->m_varArray.Add(L"scene",data);
        image.m_csgData.IndexAllMember();return leaf;
    };
    auto* oldLeaf=fillCache(17);ECSReference cached(oldLeaf);EMemoryFile cacheWire;
    if(cacheWire.Create(128)||cached.Save(cacheWire,context))return false;
    cache->m_varArray.RemoveAll();
    auto pendingCache=[&]() -> ECSReference* {
        auto* ref=new ECSReference;cacheWire.SeekLarge(0,ESLFileObject::FromBegin);
        if(ref->Load(cacheWire,context)){delete ref;return nullptr;}return ref;
    };
    ECSArray frame;auto* missing=pendingCache();if(!missing)return false;
    frame.m_varArray.Add(missing);auto* later=new ECSReference;
    if(load(*later)){delete later;return false;}frame.m_varArray.Add(later);
    auto* prototype=new ECSReference;if(load(*prototype)){delete prototype;return false;}
    frame.SetDefaultElement(prototype);
    ECSHash table;auto* hashMissing=pendingCache();if(!hashMissing)return false;
    table.m_varArray.Add(L"cached",hashMissing);
    if(!check(!frame.CommitAllReference(context)&&!table.CommitAllReference(context)&&
        !ECSObject::GetEntity(missing)&&!ECSObject::GetEntity(hashMissing)&&
        ECSObject::GetEntity(later)==receiver&&ECSObject::GetEntity(prototype)==receiver,
        "cold static cache follows original container policy and later/default references still resolve"))return false;
    auto* reloadedLeaf=fillCache(99);
    if(!check(!frame.CommitAllReference(context)&&!table.CommitAllReference(context)&&
        ECSObject::GetEntity(missing)==reloadedLeaf&&ECSObject::GetEntity(hashMissing)==reloadedLeaf,
        "pending cache paths retain identity and can resolve after real cache reconstruction"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Primary reference probe PASS: actual saved local this[1], repeat nonowned target, no owner self-cycle, recursive pending-child commit, original cold-cache container semantics");
    return true;
}

#include "legacy_script_file.h"
#include <vector>

bool CheckLegacyContextLoadFailure(ECSEnvironment& environment) {
    class Image final:public ECSExecutionImage {
    public:Image(){
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }} image;
    // Exercise an actual failing virtual restore stage after processor, graph,
    // references and heap have all been replaced; no fabricated File result.
    class Context final:public ECSContext {
    public:bool failExtended=false;unsigned extendedCalls=0;
        ESLError LoadExtendedData(EMCFile& file) override {
            ++extendedCalls;
            return failExtended?ESLErrorMsg("Primary restore probe: extended data rejected"):
                ECSContext::LoadExtendedData(file);
        }
    } context;
    image.AttachCSEnvironment(&environment);
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope{ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}}scope{context,previous};
    auto check=[](bool good,const char* step){study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady","Primary restore failure probe %s: %s",good?"OK":"FAIL",step);return good;};
    if(!check(!context.InitializeContext(&image),"isolated actual context"))return false;
    context.m_stack.CreateNameBlock(context.GetConstantString(L"SavedFrame"));
    context.m_stack.CreateNewVariable(L"marker",new ECSInteger(77));
    context.m_ip=9;context.SetStatus(ECSContext::xsExecution);
    const auto savedStackSize=context.m_stack.m_varArray.GetSize();
    ECSFile save;
    if(!check(!save.CreateMemoryFile(8192)&&!save.SaveContext(nullptr,true,context),"real raw File.SaveContext includes graph and nonempty heap"))return false;
    auto* memory=ESLTypeCast<EMemoryFile>(save.GetFileInterface());
    if(!memory)return false;
    std::vector<uint8_t> wire(static_cast<const uint8_t*>(memory->GetBuffer()),static_cast<const uint8_t*>(memory->GetBuffer())+memory->GetLength());
    auto invoke=[&](const std::vector<uint8_t>& bytes){
        ECSFile file;
        if(file.CreateMemoryFile(bytes.size())||file.Write(bytes.data(),bytes.size())!=bytes.size())return eslErrGeneral;
        file.Seek(0,ESLFileObject::FromBegin);
        ECSObjArray<ECSObject> arguments;arguments.Add(new ECSReference(&file));
        return file.Call_LoadContext(context,arguments);
    };
    context.m_ip=0;context.failExtended=true;
    const auto failed=invoke(wire);
    if(!check(failed&&std::strcmp(GetESLErrorMsg(failed),"Primary restore probe: extended data rejected")==0&&
        context.GetStatus()==ECSContext::xsHalt&&context.m_ip==9&&context.extendedCalls==1&&
        context.m_stack.m_varArray.GetSize()==savedStackSize,
        "destructive failure propagates original error, halts saved IP and never pushes a result"))return false;
    context.m_ip=0;context.SetStatus(ECSContext::xsExecution);
    const std::vector<uint8_t> invalidHeader(16,0);
    const auto early=invoke(invalidHeader);
    std::unique_ptr<ECSObject> result(context.PopObject());INT64 value=0;
    if(!check(!early&&result&&!result->OperateInteger(value)&&value!=0&&context.GetStatus()==ECSContext::xsExecution&&
        context.m_ip==0&&context.extendedCalls==1&&context.m_stack.m_varArray.GetSize()==savedStackSize,
        "invalid outer file still returns an Integer error without changing live execution"))return false;
    context.failExtended=false;
    const auto restored=invoke(wire);
    result.reset(context.PopObject());value=-1;
    if(!check(!restored&&result&&!result->OperateInteger(value)&&value==0&&context.GetStatus()==ECSContext::xsExecution&&
        context.m_ip==9&&context.extendedCalls==2&&context.m_stack.m_varArray.GetSize()==savedStackSize,
        "successful actual restore keeps original saved continuation and result contract"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Primary restore failure probe PASS: %zu-byte actual File wire; early failure intact, late failure halted with original error/no push, success resumed",wire.size());
    return true;
}
