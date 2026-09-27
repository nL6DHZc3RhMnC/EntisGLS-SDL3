#include "legacy_compat/gls.h"
#include "legacy_thread_state_probe.h"
#include "../tools/legacy_serialization.h"
#include "platform/log.h"
#include <algorithm>
#include <cstring>
#include <vector>

namespace {
bool Check(bool ok,const char* stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy thread state probe FAIL: %s",stage);
    return ok;
}
class StateImage final:public ECSExecutionImage {
public:
    StateImage() {
        // Empty object function followed by a cooperative infinite jump at 11.
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0,6,0xfb,0xff,0xff,0xff};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
class FailingString final:public ECSString {
    bool& called_;
public:
    explicit FailingString(bool& called):ECSString(L"partial"),called_(called){}
    ESLError Save(ESLFileObject& file,ECSContext&) override {
        called_=true;const uint8_t partial[]={0x12,0x34};file.Write(partial,2);return eslErrNotSupported;
    }
};
class ShortWriter final:public EMemoryFile {
    size_t remaining_;
public:
    explicit ShortWriter(size_t limit):remaining_(limit){Create(256);}
    unsigned long Write(const void* bytes,unsigned long count) override {
        const auto n=std::min<size_t>(count,remaining_);remaining_-=n;return EMemoryFile::Write(bytes,n);
    }
};
class InvalidReference final:public ECSReference {
public:
    InvalidReference(){m_omBaseMode=static_cast<CSObjectMode>(0x7fffffff);}
};
bool Rewind(EMemoryFile& file){return file.SeekLarge(0,ESLFileObject::FromBegin)==0;}
bool CopyPrefix(EMemoryFile& out,const EMemoryFile& in,size_t count) {
    return !out.Create(count+1)&&(!count||out.Write(in.GetBuffer(),count)==count)&&Rewind(out);
}
bool Integer(ECSObject* object,INT64 expected) {
    auto* value=ESLTypeCast<ECSInteger>(object);return value&&value->GetValue()==expected;
}
bool String(ECSObject* object,const wchar_t* expected) {
    auto* value=ESLTypeCast<ECSString>(object);return value&&value->m_varStr==expected;
}
void EmptyObjectThread(ECSThread& thread) {
    // These records deliberately have no naked/VA heap dependency. The normal
    // Context probe separately exercises heap relocation and processor records.
    thread.GetContext().ReleaseContext(false);
}
bool ThreadFailures(ECSContext& context,EMemoryFile& valid) {
    for(int location=0;location<4;++location) {
        ECSThread source(context);EmptyObjectThread(source);bool called=false;
        auto* failure=new FailingString(called);
        if(location==0)static_cast<ECSReference*>(source.GetVariableAt(-1))->SetOwnObject(failure,&context);
        if(location==1)source.GetContext().m_pRetObj=failure;
        if(location==2)source.GetContext().m_stack.PushObject(failure);
        if(location==3) {
            auto* nested=new ECSArray;nested->m_varArray.Add(failure);source.GetContext().m_arg.m_varArray.Add(nested);
        }
        EMemoryFile output;output.Create(256);const auto error=source.Save(output,context);
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy thread nested save location=%d called=%d error=%s",location,called,GetESLErrorMsg(error));
        if(!Check(called&&error==eslErrNotSupported,"nested procedure/return/stack/argument save error propagates"))return false;
    }
    const auto length=valid.GetLength();
    std::vector<size_t> cuts={0,1,11,12,13,17,31,47,63,100,1024,2083,length-8,length-4,length-1};
    for(const auto cut:cuts) {
        if(cut>=length)continue;
        ECSThread target(context);EMemoryFile partial;
        if(!CopyPrefix(partial,valid,cut))return false;
        // A truncated record claiming to be executing must still never launch.
        if(cut>=4)StudySteadyLegacyWire::Write32(static_cast<uint8_t*>(partial.GetBuffer()),ECSContext::xsExecution);
        const auto error=target.Load(partial,context);target.OnFinishedLoad(context);
        const bool clean=error&&!target.IsThreadRunning()&&!target.Handle()&&
            target.GetContext().GetStatus()==ECSContext::xsHalt&&
            target.GetContext().m_stack.m_varArray.GetSize()==0&&target.GetContext().m_arg.m_varArray.GetSize()==0;
        if(!clean)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy thread truncated cut=%zu/%lu error=%s",cut,length,GetESLErrorMsg(error));
        if(!Check(clean,"truncated thread fails halted and cannot be started"))return false;
        if(!Check(Rewind(valid)&&!target.Load(valid,context)&&!target.CommitAllReference(context),"clean retry after failed thread load"))return false;
    }
    ECSThread target(context);
    if(!Check(Rewind(valid)&&!target.Load(valid,context),"load complete thread before failed reference commit"))return false;
    // A direct procedure error is fatal. Ordinary Array/Hash child errors are
    // deliberately tolerated by the original game (cold static script cache).
    static_cast<ECSReference*>(target.GetVariableAt(-1))->SetOwnObject(new InvalidReference,&context);
    const auto commitError=target.CommitAllReference(context);target.OnFinishedLoad(context);
    if(!Check(commitError&&!target.IsThreadRunning()&&!target.Handle()&&
        target.GetContext().GetStatus()==ECSContext::xsHalt&&target.GetContext().m_arg.m_varArray.GetSize()==0,
        "direct invalid procedure reference propagates through Thread and prevents launch"))return false;
    ECSThread live(context);ECSObjArray<ECSObject> args;
    if(!Check(!live.BeginThread(11,args),"start actual thread for direct-load boundary"))return false;
    Rewind(valid);const auto loadError=live.Load(valid,context);const bool retained=live.IsThreadRunning();
    const auto consumed=valid.GetPosition();const auto abortError=live.AbortThread(2000);
    return Check(loadError&&retained&&consumed==0&&!abortError,"direct Load rejects live thread without reading or changing it");
}

bool StackWire(ECSContext& context) {
    ECSStack source,restored;
    const wchar_t* outer=L"外\U0001f680";
    const wchar_t inner[]={L'内',wchar_t(0xd800),L'部',0};
    if(!Check(!source.CreateNameBlock(context.GetConstantString(outer))&&
        !source.CreateNewVariable(context.GetConstantString(L"alpha")->CharPtr(),new ECSInteger(71))&&
        !source.CreateNameBlock(context.GetConstantString(inner))&&
        !source.CreateNewVariable(context.GetConstantString(L"beta")->CharPtr(),new ECSString(L"値\U0001f680")),
        "construct nested stack namespaces"))return false;
    source.m_block.GetAt(0)->m_dwCatchAddr=0x111;source.m_block.GetAt(1)->m_dwCatchAddr=0x222;
    source.m_block.GetAt(1)->m_dwFlags=ECSStack::sfTryBlock;
    EMemoryFile array,wire;array.Create(256);wire.Create(256);
    if(!Check(!source.ECSArray::Save(array,context)&&!source.Save(wire,context),"save named stack with original field order"))return false;
    const auto* bytes=static_cast<const uint8_t*>(wire.GetBuffer());size_t cursor=array.GetLength();
    if(!Check(cursor+4<=wire.GetLength()&&StudySteadyLegacyWire::Read32(bytes+cursor)==2,"Win32 stack block count"))return false;
    cursor+=4;std::vector<size_t> cuts={array.GetLength()-1,cursor-1};
    const uint8_t names[2][6]={{0x16,0x59,0x3d,0xd8,0x80,0xde},{0x85,0x51,0x00,0xd8,0xe8,0x90}};
    for(unsigned index=0;index<2;++index) {
        if(!Check(cursor+30<=wire.GetLength()&&StudySteadyLegacyWire::Read32(bytes+cursor)==(index?ECSStack::sfTryBlock:0)&&
            StudySteadyLegacyWire::Read32(bytes+cursor+4)==3&&!std::memcmp(bytes+cursor+8,names[index],6)&&
            StudySteadyLegacyWire::Read32(bytes+cursor+14)==index&&StudySteadyLegacyWire::Read32(bytes+cursor+18)==1&&
            StudySteadyLegacyWire::Read32(bytes+cursor+22)==(index?0x222:0x111)&&StudySteadyLegacyWire::Read32(bytes+cursor+26)==1,
            "UTF16 unit count/bytes and flags, bound, variable count, catch, tag-count offsets"))return false;
        cuts.insert(cuts.end(),{cursor+7,cursor+9,cursor+13,cursor+17,cursor+25,cursor+29});
        cursor+=30;const auto tagUnits=StudySteadyLegacyWire::Read32(bytes+cursor);cursor+=4+2*tagUnits;
    }
    if(!Check(cursor==wire.GetLength()&&Rewind(wire)&&!restored.Load(wire,context),"complete stack record consumes exact wire"))return false;
    int alpha=-1,beta=-1;
    if(!Check(restored.m_block.GetSize()==2&&*restored.m_block.GetAt(0)->m_pwstrName==outer&&
        *restored.m_block.GetAt(1)->m_pwstrName==inner&&restored.m_block.GetAt(1)->m_dwCatchAddr==0x222&&
        !restored.GetVariableIndex(alpha,L"alpha")&&!restored.GetVariableIndex(beta,L"beta")&&alpha==0&&beta==1&&
        Integer(restored.GetVariableAt(alpha),71)&&String(restored.GetVariableAt(beta),L"値\U0001f680"),
        "multiple namespace directory restored with non-BMP and unpaired surrogate names"))return false;
    EMemoryFile second;second.Create(256);
    if(!Check(!restored.Save(second,context)&&second.GetLength()==wire.GetLength()&&
        !std::memcmp(second.GetBuffer(),wire.GetBuffer(),wire.GetLength()),"stack reload/resave preserves all wire bytes"))return false;
    cuts.push_back(wire.GetLength()-1);
    for(const auto cut:cuts) {
        EMemoryFile partial;if(!CopyPrefix(partial,wire,cut))return false;
        if(!Check(restored.Load(partial,context)&&restored.m_block.GetSize()==0&&restored.m_varArray.GetSize()==0,
            "stack short record fails and clears partially constructed blocks/values"))return false;
    }
    ShortWriter output(wire.GetLength()-1);
    return Check(source.Save(output,context)!=0,"stack final tag short write cannot report success");
}

template<class Container>
bool DefaultCommit(ECSContext& context) {
    Container source,restored;source.SetDefaultElement(new ECSSprite);
    EMemoryFile wire;wire.Create(256);
    if(!Check(!source.Save(wire,context)&&Rewind(wire)&&!restored.Load(wire,context),
        "container serializes native Sprite default prototype"))return false;
    auto* prototype=ESLTypeCast<ECSSprite>(restored.m_pDefObj);
    if(!Check(prototype&&prototype->IsLegacyRestorePending()&&!restored.CommitAllReference(context)&&
        !prototype->IsLegacyRestorePending(),"container Commit finalizes actual native default state"))return false;
    EMemoryFile second;second.Create(256);
    if(!Check(!restored.Save(second,context),"native default prototype can be saved again after commit"))return false;
    restored.SetDefaultElement(new InvalidReference);
    return Check(restored.CommitAllReference(context)!=0,"invalid default reference error propagates from container");
}

bool ContainerCommit(ECSContext& context) {
    if(!DefaultCommit<ECSArray>(context)||!DefaultCommit<ECSHash>(context))return false;
    ECSHash hash;
    hash.m_varArray.Add(ECSWideString(L"mapLayer"),new InvalidReference);
    return Check(!hash.CommitAllReference(context)&&!ECSObject::GetEntity(hash.m_varArray.GetObjectAt(0)),
        "ordinary Hash entry reference failure follows original continue policy");
}

bool HashWire(ECSContext& context) {
    ECSHash source,restored;
    const wchar_t lone[]={L'内',wchar_t(0xd800),L'部',0};
    source.m_varArray.SetAt(0,new ETaggedElement<ECSWideString,ECSObject>(ECSWideString(L"鍵\U0001f680"),new ECSInteger(12)));
    source.m_varArray.SetAt(1,nullptr);
    source.m_varArray.SetAt(2,new ETaggedElement<ECSWideString,ECSObject>(ECSWideString(lone),new ECSString(L"value")));
    source.SetDefaultElement(new ECSInteger(44));
    EMemoryFile wire,value;wire.Create(256);value.Create(32);
    if(!Check(!source.Save(wire,context)&&!context.SaveObject(value,source.m_varArray.GetObjectAt(0)),"save Hash Unicode keys and null slot"))return false;
    const auto* bytes=static_cast<const uint8_t*>(wire.GetBuffer());
    const uint8_t first[]={0x75,0x93,0x3d,0xd8,0x80,0xde},second[]={0x85,0x51,0,0xd8,0xe8,0x90};
    const size_t hole=14+value.GetLength(),next=hole+4;
    if(!Check(StudySteadyLegacyWire::Read32(bytes)==3&&StudySteadyLegacyWire::Read32(bytes+4)==3&&
        !std::memcmp(bytes+8,first,6)&&StudySteadyLegacyWire::Read32(bytes+hole)==UINT32_MAX&&
        StudySteadyLegacyWire::Read32(bytes+next)==3&&!std::memcmp(bytes+next+4,second,6),
        "Hash Win32 UTF16 counts/bytes and -1 empty-entry sentinel"))return false;
    if(!Check(Rewind(wire)&&!restored.Load(wire,context)&&!restored.CommitAllReference(context)&&
        restored.m_varArray.GetSize()==3&&!restored.m_varArray.GetAt(1)&&
        restored.m_varArray.GetAt(0)->Tag()==L"鍵\U0001f680"&&restored.m_varArray.GetAt(2)->Tag()==lone&&
        Integer(restored.m_varArray.GetObjectAt(0),12)&&String(restored.m_varArray.GetObjectAt(2),L"value")&&
        Integer(restored.m_pDefObj,44),"Hash values, default and unpaired-surrogate key round trip"))return false;
    EMemoryFile again;again.Create(256);
    if(!Check(!restored.Save(again,context)&&again.GetLength()==wire.GetLength()&&
        !std::memcmp(again.GetBuffer(),wire.GetBuffer(),wire.GetLength()),"Hash resave preserves complete fixed wire"))return false;
    for(const size_t cut:{size_t(3),size_t(7),size_t(9),hole-1,hole+3,next+5,size_t(wire.GetLength()-1)}) {
        EMemoryFile partial;if(!CopyPrefix(partial,wire,cut))return false;
        if(!Check(restored.Load(partial,context)&&restored.m_varArray.GetSize()==0&&!restored.m_pDefObj,
            "Hash truncated key/value/default fails and clears incomplete ownership"))return false;
    }
    ShortWriter output(wire.GetLength()-1);
    return Check(source.Save(output,context)!=0,"Hash short default write propagates");
}
}

bool CheckLegacyThreadState(ECSEnvironment& environment) {
    StateImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    const auto previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} scope{context,previous};
    if(!Check(!context.InitializeContext(&image),"initialize isolated context"))return false;
    ECSThread source(context),restored(context);EmptyObjectThread(source);
    source.SetExceptionFunction(L"例外\U0001f680");
    source.GetContext().m_stack.PushObject(new ECSInteger(0x123456789LL));
    source.GetContext().m_arg.m_varArray.Add(new ECSString(L"引数\U0001f680"));
    source.GetContext().m_pRetObj=new ECSInteger(97);
    EMemoryFile valid;valid.Create(4096);
    if(!Check(!source.Save(valid,context)&&Rewind(valid)&&!restored.Load(valid,context)&&!restored.CommitAllReference(context)&&
        EWideString(restored.GetExceptionFunction())==L"例外\U0001f680"&&
        Integer(restored.GetContext().m_stack.GetVariableAt(0),0x123456789LL)&&
        String(restored.GetContext().m_arg.GetVariableAt(0),L"引数\U0001f680")&&Integer(restored.GetContext().m_pRetObj,97)&&
        restored.GetVariableAt(1)==&restored.GetContext().m_arg,"thread processor/stack/args/return/exception-name round trip"))return false;
    ShortWriter shortWrite(valid.GetLength()-1);
    if(!Check(source.Save(shortWrite,context)!=0,"ignored nested final array bounds short write propagates"))return false;
    if(!ThreadFailures(context,valid)||!StackWire(context)||!ContainerCommit(context)||!HashWire(context))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy thread state probe PASS: nested errors, truncated loads, reference commit, live-load rejection, UTF16 stack directories");
    return true;
}
