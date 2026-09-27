// Read-only, bounded diagnostics for a saved local UISave receiver. This never
// calls CommitGlobalReference/CommitAllReference or changes a back-link chain.
#include "platform/log.h"
#include <string>
#include <cstring>
#include <cwchar>
namespace {
ECSObject* PrimaryTraceEntity(ECSObject* object) {
    for(int depth=0;object&&object->m_vtType==csvtReference&&depth<32;++depth)
        object=static_cast<ECSReference*>(object)->m_pRef;
    return object;
}
void PrimaryTraceObject(ECSContext& context,const char* stage,size_t slot,ECSObject* object) {
    auto* entity=PrimaryTraceEntity(object);
    auto* ref=ESLTypeCast<ECSReference>(object);
    std::string owner;
    auto* at=entity;
    for(int i=0;at&&at->m_pParent&&i<24;++i){owner+="/"+std::to_string(at->m_nIndex);at=at->m_pParent;}
    const char* root=at==&context.m_stack?"stack":at==&context.m_arg?"arg":
        context.m_pcsxi&&at==&context.m_pcsxi->m_csgGlobal?"global":
        context.m_pcsxi&&at==&context.m_pcsxi->m_csgData?"static":"unindexed";
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
        "Primary restore %s slot=%zu obj=%p ref=%p own=%p entity=%p type=%ls parentRef=%p parentIndex=%d owner=%s%s cast=%d/%d/%d",
        stage,slot,object,ref?ref->m_pRef:nullptr,ref?ref->m_pOwnObj:nullptr,entity,
        entity?entity->GetTypeName():L"<null>",ref?ref->m_pRefParent:nullptr,ref?ref->m_iParentRef:-1,root,owner.c_str(),
        ref?ref->m_iVarOffset:0,ref?ref->m_nVarBounds:0,ref?ref->m_iFuncOffset:0);
}
void LegacyTracePrimary(ECSContext& context,const char* stage) {
    if(!std::strcmp(stage,"load-complete"))
        study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy context load complete");
    bool interesting=false;
    for(size_t i=0;i<context.m_stack.m_block.GetSize();++i){auto* block=context.m_stack.m_block.GetAt(i);
        if(block&&block->m_pwstrName&&block->m_pwstrName->CharPtr()&&std::wcsstr(block->m_pwstrName->CharPtr(),L"UISave")){interesting=true;break;}}
    if(!interesting)return;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Primary restore %s ip=%08x stack=%u blocks=%u arg=%u current=%p bound=%u",
        stage,context.m_ip,context.m_stack.m_varArray.GetSize(),context.m_stack.m_block.GetSize(),context.m_arg.m_varArray.GetSize(),
        context.m_stack.m_pCurrent,context.m_stack.m_pCurrent?context.m_stack.m_pCurrent->m_iBound:0);
    PrimaryTraceObject(context,stage,size_t(-1),context.m_pRetObj);
    if (context.m_arg.m_varArray.GetSize()) PrimaryTraceObject(context,stage,size_t(-2),context.m_arg.m_varArray.GetAt(0));
    for(size_t i=0;i<context.m_stack.m_block.GetSize()&&i<64;++i){
        auto* block=context.m_stack.m_block.GetAt(i);if(!block||!block->m_pwstrName)continue;
        if((block->m_pwstrName->CharPtr()&&std::wcsstr(block->m_pwstrName->CharPtr(),L"UISave"))||block==context.m_stack.m_pCurrent){
            study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Primary restore frame=%zu name=%ls bound=%u vars=%u flags=%u first=%ls",
                i,block->m_pwstrName->CharPtr()?block->m_pwstrName->CharPtr():L"",block->m_iBound,block->m_nVarCount,block->m_dwFlags,
                block->GetAt(0)?block->GetAt(0):L"<none>");
            PrimaryTraceObject(context,stage,block->m_iBound,context.m_stack.m_varArray.GetAt(block->m_iBound));
        }
    }
    for(size_t i=0;i<context.m_stack.m_varArray.GetSize()&&i<1024;++i){
        auto* object=context.m_stack.m_varArray.GetAt(i);auto* entity=PrimaryTraceEntity(object);
        if(entity&&!std::wcscmp(entity->GetTypeName(),L"UISave"))PrimaryTraceObject(context,stage,i,object);
    }
}
}
