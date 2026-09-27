#include "legacy_compat/gls.h"
#include "legacy_sprite_state_probe.h"
#include "legacy_sprite.h"
#include "legacy_sprite_draw.h"
#include "legacy_sprite_callbacks.h"
#include "legacy_message.h"
#include "legacy_tone_filter.h"
#include "legacy_resource_manager.h"
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include "platform/log.h"
#include <cmath>
#include <cstring>
#include <memory>

namespace {
bool Check(bool good,const char* stage) {
    study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,"StudySteady",
        "Legacy Sprite state probe %s: %s",good?"OK":"FAIL",stage);
    return good;
}
bool Status(ESLError error,const char* stage) {
    if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Sprite state probe %s: %s",stage,GetESLErrorMsg(error));
    return Check(!error,stage);
}
class StateImage final:public ECSExecutionImage {
public:
    StateImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
ECSObject* Named(ECSGlobal& root,const wchar_t* name) {
    int index=0;return root.GetVariableIndex(index,name)?nullptr:ECSObject::GetEntity(root.GetVariableAt(index));
}
bool Invoke(ECSContext& context,ECSSprite& sprite,const wchar_t* method,ECSObjArray<ECSObject>& args) {
    int index=-1;ESLError error=sprite.GetFunction(context,index,method);
    if(!error)error=sprite.CallFunction(context,index,args);
    INT64 status=0;ECSObject* result=nullptr;
    if(!error){result=context.PopObject();error=result?result->OperateInteger(status):eslErrGeneral;}
    context.delete_CSObject(result);
    if(error||status)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Sprite state call %ls: %s / %lld",method,GetESLErrorMsg(error),static_cast<long long>(status));
    return !error&&!status;
}
bool NumberCall(ECSContext& context,ECSSprite& sprite,const wchar_t* name,int value) {
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&sprite));args.Add(new ECSInteger(value));
    return Invoke(context,sprite,name,args);
}
}

namespace {
class OptionalButtonSprite:public ECSSprite {
public:
    bool HasSavedItem(const wchar_t* id)const{return legacyItemState_.m_varArray.GetAs(id)!=nullptr;}
};
bool CheckOptionalButtonState(ECSEnvironment& environment) {
    StateImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope{ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} scope{context,previous};
    if(!Status(context.InitializeContext(&image),"optional-button isolated context"))return false;
    auto* skin=new ECSResourceManager;image.m_csgData.AddVariable(L"skin",skin);
    if(!Status(skin->LoadSkinFile(L"wm_window_ZHTW.noa",context),"actual message-window skin for optional controls"))return false;
    auto* page=new OptionalButtonSprite;image.m_csgGlobal.AddVariable(L"page",page);
    if(!Status(page->BuildFormPage(skin->GetSkin(),L"ID_MESSAGE_WINDOW"),"actual original message page"))return false;
    page->RecordLegacyFormSource(skin,L"ID_MESSAGE_WINDOW",context);
    for(const wchar_t* id:{L"ID_SKIP",L"ID_SKIP2"}) {
        if(!Check(!page->NativeSprite().GetItemAs(id),"original message page intentionally lacks generic skip button"))return false;
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(page));args.Add(new ECSString(id));args.Add(new ECSInteger(-1));
        if(!Check(Invoke(context,*page,L"CheckButton",args)&&!page->HasSavedItem(id),"absent CheckButton remains no-op and creates no saved property"))return false;
        // Reproduce the exact harmless records written by the earlier APK.
        page->RecordLegacyItemInteger(id,L"checked",id[7]?0:-1);
    }
    for(const wchar_t* method:{L"SetSpriteText",L"SetSpriteFontFace",L"SetSpriteImage"}) {
        const wchar_t* id=L"ID_ABSENT_TEXT_STATE";
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(page));args.Add(new ECSString(id));args.Add(new ECSString(L"unused"));
        if(!Check(Invoke(context,*page,method,args)&&!page->HasSavedItem(id),
            "absent text/font/image setter remains no-op and creates no saved property"))return false;
    }
    auto* button=page->NativeSprite().GetItemAs(L"ID_MSG_SUB_MENU");
    if(!Check(button!=nullptr,"real page button exists"))return false;
    button->SetEnable(false);page->RecordLegacyItemInteger(L"ID_MSG_SUB_MENU",L"enabled",0);
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();EMemoryFile file;
    if(!Status(file.Create(1024),"optional-button saved graph")||!Status(image.m_csgGlobal.Save(file,context),"serialize earlier optional-button records"))return false;
    image.m_csgGlobal.CleanupAllReference(context);image.m_csgGlobal.RemoveAllVariable();file.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(image.m_csgGlobal.Load(file,context),"load optional-button saved graph")||
       !Status(image.m_csgGlobal.CommitAllReference(context),"restore actual page with optional no-op button records"))return false;
    auto* restored=ESLTypeCast<ECSSprite>(Named(image.m_csgGlobal,L"page"));
    if(!Check(restored&&!restored->NativeSprite().GetItemAs(L"ID_SKIP")&&
        !restored->NativeSprite().GetItemAs(L"ID_MSG_SUB_MENU")->IsEnabled(),"no fake button created and real enabled state restored"))return false;
    // Unlike optional CheckButton, a saved change to an absent actual item must
    // still fail; this is not a general missing-item exception.
    restored->RecordLegacyItemInteger(L"ID_REQUIRED_MISSING",L"enabled",0);
    image.m_csgData.IndexAllMember();image.m_csgGlobal.IndexAllMember();EMemoryFile invalid;
    if(!Status(invalid.Create(1024),"required-item failure record")||!Status(image.m_csgGlobal.Save(invalid,context),"save required-item failure fixture"))return false;
    image.m_csgGlobal.CleanupAllReference(context);image.m_csgGlobal.RemoveAllVariable();invalid.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(image.m_csgGlobal.Load(invalid,context),"load required-item failure fixture"))return false;
    image.m_csgGlobal.IndexAllMember();
    auto* invalidPage=ESLTypeCast<ECSSprite>(Named(image.m_csgGlobal,L"page"));
    // GLS3 containers may continue after a child Commit error. Assert the
    // native restoration contract at its actual boundary, not container policy.
    if(!Check(invalidPage&&invalidPage->CommitAllReference(context)!=eslErrSuccess,
              "missing required enabled item still fails Sprite restoration"))return false;
    return true;
}
}

bool CheckLegacySpriteState(ECSEnvironment& environment) {
    if(!CheckOptionalButtonState(environment))return false;
    StateImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSExecutionImage& image;ECSContext* previous;
        ~Scope(){ShutdownLegacySpriteCallbacksForImage(image);context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } scope{context,image,previous};
    if(!Status(context.InitializeContext(&image),"isolated graphic serialization context"))return false;
    // A static scene root stands in for the game's constant screen Window.
    // Only globals below it are destroyed and replaced by the load operation.
    auto* scene=new ECSSprite;image.m_csgData.AddVariable(L"scene",scene);
    auto* manager=new ECSResourceManager;image.m_csgGlobal.AddVariable(L"manager",manager);
    if(!Status(manager->LoadSkinFile(L"wm_langpicker.noa",context),"reopenable real form skin"))return false;
    auto* source=new ECSResource;image.m_csgGlobal.AddVariable(L"source",source);
    if(!Status(source->LoadImageFile(L"particle_light1.eri",&context),"reopenable real image"))return false;
    auto* fileSprite=new ECSSprite;image.m_csgGlobal.AddVariable(L"fileSprite",fileSprite);
    if(!Status(fileSprite->LoadImageFile(L"particle_light1.eri",&context),"file Sprite copy source"))return false;
    auto* duplicate=ESLTypeCast<ECSSprite>(fileSprite->Duplicate());
    if(!Check(duplicate!=nullptr,"duplicate file Sprite pixels"))return false;
    image.m_csgGlobal.AddVariable(L"duplicate",duplicate);
    auto* assigned=new ECSSprite;image.m_csgGlobal.AddVariable(L"assigned",assigned);
    if(!Status(assigned->Move(context,new ECSReference(fileSprite)),"assign file Sprite pixels"))return false;
    auto* child=new ECSSprite;image.m_csgGlobal.AddVariable(L"child",child);
    {
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(child));args.Add(new ECSReference(source));args.Add(new ECSInteger(0));
        if(!Check(Invoke(context,*child,L"AttachImage",args),"real source attachment"))return false;
    }
    auto* tone=new ECSToneFilter;image.m_csgGlobal.AddVariable(L"tone",tone);
    {ECSObjArray<ECSObject> args;args.Add(new ECSReference(child));args.Add(new ECSReference(tone));
     if(!Check(Invoke(context,*child,L"AttachToneFilter",args),"attach identity tone with serializable graph reference"))return false;}
    scene->NativeSprite().AddChild(&child->NativeSprite());child->NativeSprite().SetID(L"RESTORED_CHILD");
    child->NativeSprite().SetVisible(true);child->NativeSprite().SetPosition(17,29);
    child->NativeSprite().ChangePriority(-3);
    if(!Check(NumberCall(context,*child,L"SetBlendingEnvelope",256)&&NumberCall(context,*child,L"BeginActivation",100),"begin real timed action"))return false;
    child->NativeSprite().AdvanceTime(40);
    const int transparency=child->NativeSprite().GetTransparency();
    auto* form=new ECSSprite;image.m_csgGlobal.AddVariable(L"form",form);
    if(!Status(form->BuildFormPage(manager->GetSkin(),L"ID_LANGPICKER_FRAME"),"real native form page"))return false;
    form->RecordLegacyFormSource(manager,L"ID_LANGPICKER_FRAME",context);
    scene->NativeSprite().AddChild(&form->NativeSprite());
    auto* checkbox=form->NativeSprite().GetItemAs(L"ID_LANGPICKER_CHECKBOX");
    if(!Check(checkbox!=nullptr,"skin checkbox exists"))return false;
    checkbox->CheckButton(true);form->RecordLegacyItemInteger(L"ID_LANGPICKER_CHECKBOX",L"checked",1);
    form->NativeSprite().GetItemAs(L"ID_LANGPICKER_EN")->SetEnable(false);
    form->RecordLegacyItemInteger(L"ID_LANGPICKER_EN",L"enabled",0);
    auto* message=new ECSMessageSprite;image.m_csgGlobal.AddVariable(L"message",message);
    if(!Status(message->CreateMessage(512,160),"message canvas")||!Status(message->SetDefaultMsgSpeed(30,60),"message speed"))return false;
    const wchar_t text[]=L"日本語\\n:試験\\r;漢字;かんじ:";size_t consumed=0;
    if(!Check(!message->OutputMessage(text,consumed)&&consumed==std::wcslen(text),"formatted Japanese ruby text"))return false;
    const auto glyphCount=message->NativeMessage().GetMessageCharacterCount();
    scene->NativeSprite().AddChild(&message->NativeSprite());
    image.m_csgGlobal.IndexAllMember();image.m_csgData.IndexAllMember();
    EMemoryFile file;if(!Status(file.Create(4096),"memory save"))return false;
    if(!Status(image.m_csgGlobal.Save(file,context),"serialize native graph and actual reference indices"))return false;
    const auto bytes=file.GetLength();
    image.m_csgGlobal.CleanupAllReference(context);image.m_csgGlobal.RemoveAllVariable();
    if(!Check(scene->NativeSprite().GetChildCount()==0,"destroyed native graph detaches children from persistent root"))return false;
    file.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(image.m_csgGlobal.Load(file,context),"recreate native object types and media")||
       !Status(image.m_csgGlobal.CommitAllReference(context),"resolve and reconstruct the global graph"))return false;
    auto* restored=ESLTypeCast<ECSSprite>(Named(image.m_csgGlobal,L"child"));
    auto* restoredSource=ESLTypeCast<ECSResource>(Named(image.m_csgGlobal,L"source"));
    auto* restoredForm=ESLTypeCast<ECSSprite>(Named(image.m_csgGlobal,L"form"));
    auto* restoredMessage=ESLTypeCast<ECSMessageSprite>(Named(image.m_csgGlobal,L"message"));
    if(!Check(restored&&restoredSource&&restoredForm&&restoredMessage,"native type identities"))return false;
    if(!Check(scene==Named(image.m_csgData,L"scene")&&scene->NativeSprite().GetChildCount()==3&&
        restored->NativeSprite().GetParent()==&scene->NativeSprite()&&
        ECSObject::GetEntity(restored->GetVariableAt(-2))==restoredSource,
        "static root retained and shared image/parent references resolve"))return false;
    if(!Check(ECSObject::GetEntity(restored->GetVariableAt(-6))==Named(image.m_csgGlobal,L"tone"),
        "restored tone reference resolves to the rebuilt native LUT object"))return false;
    const auto pos=restored->NativeSprite().GetPosition();
    if(!Check(pos.x==17&&pos.y==29&&restored->NativeSprite().GetPriority()==-3&&
        !std::wcscmp(restored->NativeSprite().GetID(),L"RESTORED_CHILD")&&
        std::abs(int(restored->NativeSprite().GetTransparency())-transparency)<=1,
        "visual position, priority, ID and current action state"))return false;
    SakuraGL::SGLPalette actual,expected;
    if(!Check(restored->GetImage()&&!restored->GetImage()->GetPixelRGBA(actual,0,0)&&
        !restoredSource->GetImage()->GetPixelRGBA(expected,0,0)&&actual.ui32==expected.ui32,"decoded image pixels restored"))return false;
    for(const wchar_t* name:{L"duplicate",L"assigned"}) {
        auto* copy=ESLTypeCast<ECSSprite>(Named(image.m_csgGlobal,name));
        if(!Check(copy&&copy->GetImage()&&!copy->GetImage()->GetPixelRGBA(actual,0,0)&&actual.ui32==expected.ui32,
            "Duplicate/Move preserve file provenance across full graph destruction"))return false;
    }
    restored->NativeSprite().AdvanceTime(60);
    if(!Check(restored->NativeSprite().GetTransparency()==256&&!restored->NativeSprite().IsAction(),"restored action finishes after its remaining 60 ms"))return false;
    auto* restoredCheck=restoredForm->NativeSprite().GetItemAs(L"ID_LANGPICKER_CHECKBOX");
    auto* restoredEnglish=restoredForm->NativeSprite().GetItemAs(L"ID_LANGPICKER_EN");
    if(!Check(restoredCheck&&restoredCheck->IsButtonChecked()&&restoredEnglish&&!restoredEnglish->IsEnabled(),
        "native form rebuilt with checkbox and enable state"))return false;
    if(!Check(restoredMessage->NativeMessage().GetMessageCharacterCount()==glyphCount&&
        !restoredMessage->NativeMessage().IsMessagePending(),"ruby glyphs reconstructed and flushed per original load semantics"))return false;
    bool raster=false;
    for(size_t i=0;i<glyphCount;++i){auto* glyph=restoredMessage->NativeMessage().GetMessageCharacterAt(i);raster|=glyph&&glyph->m_pImage&&glyph->m_pImage->GetImageSize().w>0;}
    if(!Check(raster,"restored text has actual rasterized font images"))return false;
    // Match ECSContext::SaveContext: loading resolves references but does not
    // rebuild ownership index chains until the next save starts.
    image.m_csgGlobal.IndexAllMember();image.m_csgData.IndexAllMember();
    EMemoryFile again;if(!Status(again.Create(4096),"second memory save")||
        !Status(image.m_csgGlobal.Save(again,context),"restored graph can save again"))return false;
    // A partial individual object must fail instead of accepting zero-filled state.
    EMemoryFile single;if(!Status(single.Create(256),"truncation fixture")||!Status(restoredMessage->Save(single,context),"message fixture"))return false;
    EMemoryFile shortFile;shortFile.Open(single.GetBuffer(),single.GetLength()-1);ECSMessageSprite incomplete;
    if(!Check(incomplete.Load(shortFile,context)!=eslErrSuccess,"truncated formatted-text record fails"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy Sprite state probe PASS: %llu-byte graph, native reconstruction, static parent/shared source, real form, timed action, Japanese ruby raster and truncated error",static_cast<unsigned long long>(bytes));
    return true;
}
