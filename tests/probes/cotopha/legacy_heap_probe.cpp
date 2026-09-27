#include "compatibility/sdk/legacy/gls.h"
#include "legacy_heap_probe.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include "platform/log.h"
#include <cstring>
#include <vector>

bool CheckLegacyHeapState(ECSEnvironment& environment) {
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
    auto check=[](bool valid,const char* step){
        study::platform::LogPrint(valid?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady","Legacy heap probe %s: %s",valid?"OK":"FAIL",step);return valid;
    };
    image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSContext* previous;
        ~Scope(){context.m_bufNakedStack=nullptr;context.m_vaNakedStack=0;context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } scope{context,previous};
    if(!check(!context.InitializeContext(&image),"real InitializeSakuraProcessor allocates stack"))return false;
    auto* stack=context.m_bufNakedStack;const auto stackAddress=context.m_vaNakedStack;
    if(!check(stack&&stackAddress&&image.m_heapGlobal.GetLength()>0,"object-mode heap is not empty"))return false;
    const auto stackLength=stack->GetLength();
    for(int i=0;i<stackLength;++i)stack->GetBuffer()[i]=uint8_t(i*13+7);
    auto* extra=new ECSBuffer;if(extra->CreateBuffer(31,128)){delete extra;return false;}
    for(int i=0;i<31;++i)extra->GetBuffer()[i]=uint8_t(i*5+3);
    const auto extraAddress=image.AllocateHeapObjectAddress(extra);
    const auto id=image.AddClassIdentity(L"Buffer");
    {
        std::unique_ptr<ECSSakura2::Object> created(image.NewObjectByIdentity(&context,id));
        if(!check(ESLTypeCast<ECSBuffer>(created.get())!=nullptr,"saved Buffer identity creates actual legacy ECSBuffer"))return false;
    }
    EMemoryFile classvec,heap;if(classvec.Create(2048)||heap.Create(8192))return false;
    if(!check(!image.m_heapGlobal.PrepareSave(&image,&context)&&!image.SaveClassVector(classvec)&&!image.SaveHeapMemory(context,heap),"save real class vector and nonempty stack heap"))return false;
    const auto bytes=std::vector<uint8_t>(static_cast<const uint8_t*>(heap.GetBuffer()),static_cast<const uint8_t*>(heap.GetBuffer())+heap.GetLength());
    if(!check(bytes.size()>4096&&StudySteadyLegacyWire::Read32(bytes.data())==12&&StudySteadyLegacyWire::Read32(bytes.data()+4)==2,
        "original heap header counts two actual objects"))return false;
    context.m_bufNakedStack=nullptr;
    image.m_heapGlobal.RemoveAll(&image,&context);
    classvec.SeekLarge(0,ESLFileObject::FromBegin);heap.SeekLarge(0,ESLFileObject::FromBegin);
    if(!check(!image.LoadClassVector(classvec)&&!image.LoadHeapMemory(context,heap)&&!context.CommitLoadedProcessorContext(),"load actual heap and reconnect processor stack"))return false;
    stack=context.m_bufNakedStack;
    extra=ESLTypeCast<ECSBuffer>(image.ObjectFromAddress(uint32_t(extraAddress>>32)));
    bool content=stack&&extra&&context.m_vaNakedStack==stackAddress&&stack->GetLength()==stackLength&&extra->GetLength()==31&&extra->GetBufferBase()==128;
    if(content){for(int i=0;i<stackLength;++i)content&=stack->GetBuffer()[i]==uint8_t(i*13+7);for(int i=0;i<31;++i)content&=extra->GetBuffer()[i]==uint8_t(i*5+3);}
    if(!check(content,"preserve both virtual addresses, stack bytes and buffer base"))return false;
    EMemoryFile second;if(second.Create(8192)||image.SaveHeapMemory(context,second))return false;
    if(!check(second.GetLength()==bytes.size()&&!std::memcmp(second.GetBuffer(),bytes.data(),bytes.size()),"byte-exact second heap save"))return false;
    auto failed=[&](std::vector<uint8_t> corrupt,const char* stage){
        context.m_bufNakedStack=nullptr;EMemoryFile file;file.Open(corrupt.data(),corrupt.size());
        return check(image.LoadHeapMemory(context,file)!=eslErrSuccess,stage);
    };
    auto invalid=bytes;StudySteadyLegacyWire::Write32(invalid.data()+16,0x7fffffff);
    if(!failed(std::move(invalid),"unknown class identity returns error without null dereference"))return false;
    auto truncated=bytes;truncated.resize(24);
    if(!failed(std::move(truncated),"truncated Buffer payload fails"))return false;
    auto overrun=bytes;StudySteadyLegacyWire::Write32(overrun.data()+12,0xffffffff);
    if(!failed(std::move(overrun),"unbounded heap allocation rejected before resize"))return false;
    // A heap can legitimately have zero live objects and zero capacity. Consume
    // its original header; do not skip a record or substitute a fake success.
    std::vector<uint8_t> empty(16);StudySteadyLegacyWire::Write32(empty.data(),12);
    EMemoryFile emptyFile;emptyFile.Open(empty.data(),empty.size());
    if(!check(!image.LoadHeapMemory(context,emptyFile)&&image.m_heapGlobal.GetLength()==0&&emptyFile.GetPosition()==16,"actual empty heap consumes validated header"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy heap probe PASS: %zu-byte real stack+Buffer heap, stable virtual addresses, byte-exact roundtrip, class/truncation/limit errors, empty heap",bytes.size());
    return true;
}
