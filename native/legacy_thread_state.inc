// Included by legacy_thread.cpp. Preserve the existing GLS3 record order while
// making every nested error and short I/O visible to File's atomic save layer.
#include <limits>
#include <new>
#include "legacy_checked_state_file.h"

namespace {
using CheckedThreadFile=StudySteadyLegacy::CheckedStateFile;

void ClearThreadStorage(ECSContext& inner,ECSReference& procedure,ECSContext& context) {
    procedure.CleanupAllReference(context);procedure.SetReference(nullptr,&context);
    inner.m_stack.CleanupAllReference(context);inner.m_arg.CleanupAllReference(context);
    for(ECSArray* array:{static_cast<ECSArray*>(&inner.m_stack),&inner.m_arg}) {
        if(array->m_pDefObj)array->m_pDefObj->CleanupAllReference(context);
        array->SetDefaultElement(nullptr);array->SetBounds();
    }
    if(inner.m_pRetObj)inner.m_pRetObj->CleanupAllReference(context);
    inner.delete_CSObject(inner.m_pRetObj);inner.m_pRetObj=nullptr;
    inner.ReleaseContext(false);
    inner.SetStatus(ECSContext::xsHalt);
}
}

ESLError ECSThread::CommitAllReference(ECSContext& context) {
    auto error=m_refThreadProc.CommitAllReference(context);
    if(!error)error=m_context.m_stack.CommitAllReference(context);
    if(!error)error=m_context.m_arg.CommitAllReference(context);
    if(!error&&m_context.m_pRetObj)error=m_context.m_pRetObj->CommitAllReference(context);
    if(!error&&m_fLoadedContext)error=m_context.CommitLoadedProcessorContext();
    if(!error&&m_fLoadedContext&&m_context.m_vaNakedStack&&!m_context.m_bufNakedStack)
        error=ESLErrorMsg("Loaded thread naked stack reference cannot be resolved");
    if(error) {
        // A failed object graph must never be launched by OnFinishedLoad.
        m_fLoadedContext=false;m_sdRestore={ECSContext::xsHalt,0,0};
        if(!IsThreadRunning()) {
            __atomic_store_n(&m_dwSuspendCount,0u,__ATOMIC_SEQ_CST);
            ClearThreadStorage(m_context,m_refThreadProc,context);
        }
        return error;
    }
    if(m_fLoadedContext) {
        if(const auto reset=m_context.ResetAbortWatingEvent()) {
            m_fLoadedContext=false;m_context.SetStatus(ECSContext::xsHalt);return reset;
        }
        m_context.SetStatus(m_sdRestore.nStatus);
    }
    return eslErrSuccess;
}

ESLError ECSThread::Save(ESLFileObject& file,ECSContext& context) try {
    CheckedThreadFile checked(file);
    std::vector<uint8_t> name;
    if(!StudySteadyLegacyWire::EncodeUtf16(m_wstrExceptionFunc,name)||
        name.size()/2>std::numeric_limits<uint32_t>::max())return eslErrGeneral;
    uint8_t header[12];
    StudySteadyLegacyWire::Write32(header,uint32_t(m_context.GetStatus()));
    StudySteadyLegacyWire::Write32(header+4,GetSuspendCount());
    StudySteadyLegacyWire::Write32(header+8,uint32_t(name.size()/2));
    checked.Write(header,sizeof(header));
    if(!name.empty())checked.Write(name.data(),name.size());
    if(checked.failed)return eslErrGeneral;
    auto error=checked.Result(m_refThreadProc.Save(checked,context));
    if(!error)error=checked.Result(m_context.SaveProcessorContext(checked));
    if(!error)error=checked.Result(m_context.m_stack.Save(checked,context));
    if(!error)error=checked.Result(m_context.m_arg.Save(checked,context));
    return error;
} catch(const std::bad_alloc&) {return eslErrGeneral;}

ESLError ECSThread::Load(ESLFileObject& file,ECSContext& context) {
    // Context.Load constructs a new ECSThread for each serialized native object.
    // In-place mutation of a currently executing/suspended OS thread is unsafe.
    if(IsThreadRunning())return ESLErrorMsg("Cannot load state into a running thread");
    m_fLoadedContext=false;
    auto reset=[&] {
        m_fLoadedContext=false;m_sdRestore={ECSContext::xsHalt,0,0};
        {
            std::lock_guard<std::recursive_mutex> guard(m_controlMutex);
            m_abortRequested=false;__atomic_store_n(&m_dwSuspendCount,0u,__ATOMIC_SEQ_CST);
        }
        ClearThreadStorage(m_context,m_refThreadProc,context);
    };
    struct Incomplete {decltype(reset)& reset;bool complete=false;~Incomplete(){if(!complete)reset();}} incomplete{reset};
    try {
        CheckedThreadFile checked(file);uint8_t header[12];
        checked.Read(header,sizeof(header));if(checked.failed)return eslErrGeneral;
        SAVE_DATA state{};
        const auto status=StudySteadyLegacyWire::Read32(header);
        if(status>uint32_t(ECSContext::xsInterrupt))return eslErrInvalidParam;
        state.nStatus=static_cast<ECSContext::ExecutionStatus>(status);
        state.dwSuspendCount=StudySteadyLegacyWire::Read32(header+4);
        state.dwExceptionFuncLen=StudySteadyLegacyWire::Read32(header+8);
        const auto position=checked.GetLargePosition(),length=checked.GetLargeLength();
        if(position>length||state.dwExceptionFuncLen>8*1024*1024||
            state.dwExceptionFuncLen>(length-position)/2)return eslErrGeneral;
        std::vector<uint8_t> bytes(size_t(state.dwExceptionFuncLen)*2);EWideString name;
        if(!bytes.empty())checked.Read(bytes.data(),bytes.size());
        if(checked.failed||!StudySteadyLegacyWire::DecodeUtf16(bytes.data(),state.dwExceptionFuncLen,name))return eslErrGeneral;
        // Release prior owned VA pages before loading the saved processor VA.
        // A failed direct Load leaves an empty halted target, not the old value.
        reset();
        auto error=checked.Result(m_refThreadProc.Load(checked,context));
        if(!error)error=checked.Result(m_context.LoadProcessorContext(checked));
        m_context.SetStatus(ECSContext::xsHalt);
        if(!error)error=checked.Result(m_context.m_stack.Load(checked,context));
        if(!error)error=checked.Result(m_context.m_arg.Load(checked,context));
        if(error)return error;
        m_wstrExceptionFunc=name;m_sdRestore=state;
        __atomic_store_n(&m_dwSuspendCount,state.dwSuspendCount,__ATOMIC_SEQ_CST);
        m_fLoadedContext=true;incomplete.complete=true;
        return eslErrSuccess;
    } catch(const std::bad_alloc&) {return eslErrGeneral;}
}
