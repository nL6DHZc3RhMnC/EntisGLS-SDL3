#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_message.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include <algorithm>
#include "platform/log.h"
#include <cmath>
#include <cwchar>
#include <limits>

IMPLEMENT_CLASS_INFO(ECSMessageSprite, ECSSprite)
namespace {
using namespace SakuraGL;
constexpr int messageMethodBase = 3072;
const wchar_t *const messageMethods[] = {
    L"CreateMessage", L"OutputMessage", L"FlushMessage", L"ClearMessage", L"IsMessagePending",
    L"AttachMessageStyle", L"SetDefaultMsgSpeed", L"SetMessageEffect", L"SetShadowTransparency",
    L"SetFontBordering", L"SetFontStyle", L"SetFontFace", L"SetFontColor", L"GetCursorPos",
    L"MoveCursorPos", L"GetCharacterCount", L"GetMessageRect"
};
struct MessageLock {
    MessageLock() { SSystem::Lock(); }
    ~MessageLock() { SSystem::Unlock(); }
};
void CopyStyleXML(const SSystem::SXMLDocument& from,EDescription& to) {
    to.SetTag(from.GetTag());
    for(size_t i=0;i<from.GetAttributeCount();++i)
        to.SetAttrString(*from.GetAttributeNameAt(i),*from.GetAttributeValueAt(i));
    for(size_t i=0;i<from.GetElementsCount();++i)if(auto* child=from.GetElementAt(i)) {
        auto* target=to.CreateContentTagAs(to.GetContentTagCount(),child->GetTag());CopyStyleXML(*child,*target);
    }
}
void CopyStyleXML(const EDescription& from,SSystem::SXMLDocument& to) {
    if(from.Tag().IsEmpty())to.SetType(SSystem::SXMLDocument::typeRoot);else to.SetTag(from.Tag());
    for(int i=0;i<from.GetAttributeCount();++i)to.SetAttributeAs(from.GetAttributeNameAt(i),from.GetAttributeValueAt(i));
    for(int i=0;i<from.GetContentTagCount();++i)if(auto* child=from.GetContentTagAt(i)) {
        auto* target=new SSystem::SXMLDocument;CopyStyleXML(*child,*target);to.AddElement(target);
    }
}
}

class ECSMessageSprite::MessageBridge final : public LegacyCallbackSprite<SGLSpriteMessage> {
public:
    // Unlike the SDK's normally unbuffered message object, the legacy ABI
    // creates a private canvas. NotifyUpdate only invalidates ancestors, so
    // changes to the glyph list or fade clock must also redraw this canvas.
    void ClearMessage() { SGLSpriteMessage::ClearMessage();PostUpdate(); }
    void FlushMessage() { SGLSpriteMessage::FlushMessage();PostUpdate(); }
    void AdvanceTime(uint32_t milliseconds) override {
        const auto previous=m_msecFading;
        LegacyCallbackSprite<SGLSpriteMessage>::AdvanceTime(milliseconds);
        if(previous!=m_msecFading)PostUpdate();
    }
    void BeforeDraw(Stereo3DView view=s3dMonoview) override {
        // Native clients may call nonvirtual ClearMessage/FlushMessage through
        // SGLSpriteMessage&, including restored/native UI objects.
        if(lastDrawCount_!=m_characters.GetLength()||lastDrawClock_!=m_msecFading)PostUpdate();
        LegacyCallbackSprite<SGLSpriteMessage>::BeforeDraw(view);
        lastDrawCount_=m_characters.GetLength();lastDrawClock_=m_msecFading;
    }
    size_t lastDrawCount_=size_t(-1);
    uint32_t lastDrawClock_=UINT32_MAX;
    void Trace(const char* stage,size_t input=0,size_t consumed=0) const {
        const auto size=GetImageSize();const auto& p=GetParameter();
        const auto& r=m_lcLettering.rectWritable;
        char face[80]={};if(m_fsFont.pszFace)for(size_t i=0;i<79&&m_fsFont.pszFace[i];++i)
            face[i]=m_fsFont.pszFace[i]<128?char(m_fsFont.pszFace[i]):'?';
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
            "Legacy Message %s native=%p input=%zu consumed=%zu glyphs=%zu canvas=%dx%d rect=%d,%d,%d,%d cursor=%d,%d font=%s/%d flags=%u ink=%08x visible=%d transparency=%u pos=%.1f,%.1f parent=%p time=%u/%u speed=%u fade=%u",
            stage,this,input,consumed,m_characters.GetLength(),size.w,size.h,r.left,r.top,r.right,r.bottom,
            m_lcLettering.ptStartWriting.x,m_lcLettering.ptStartWriting.y,face,m_fsFont.nSize,m_fsFont.nStyles,
            m_ltDecoration.rgbaBody.ui32,IsVisible(),GetTransparency(),p.vDst.x,p.vDst.y,GetParent(),
            m_msecFading,m_msecDuration,m_styleMsg.viewAction.msecPerChar,m_styleMsg.viewAction.msecFade);
        if(m_characters.GetLength())if(auto* first=m_characters.GetAt(0)) {
            const auto glyph=first->m_pImage?first->m_pImage->GetImageSize():SGLSize(0,0);
            study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy Message glyph native=%p first=%dx%d at=%d,%d offset=%d,%d start=%u",
                this,glyph.w,glyph.h,first->m_ptWriting.x,first->m_ptWriting.y,first->m_ptOffset.x,first->m_ptOffset.y,first->m_msecStart);
        }
    }
    struct Checkpoint { size_t count; SGLLetteringContext context; SGLRect rect;
        bool hasRect; uint32_t lastTiming,duration; };
    Checkpoint SaveCheckpoint() const { return {m_characters.GetLength(),m_lcLettering,m_rectChars,
        m_flagRectChars,m_msecLastTiming,m_msecDuration}; }
    void Rollback(const Checkpoint &point) {
        m_characters.SetLength(point.count);m_lcLettering=point.context;m_rectChars=point.rect;
        m_flagRectChars=point.hasRect;m_msecLastTiming=point.lastTiming;m_msecDuration=point.duration;
        PostUpdate();
    }
    void SetCursor(int x, int y) { m_lcLettering.ptStartWriting=SGLPoint(x,y); }
    void SetWritable(const SGLRect &rect) {
        m_styleMsg.context.rectWritable=rect;
        m_styleMsg.context.ptStartWriting=SGLPoint(rect.left,rect.top);
        m_lcLettering.rectWritable=rect;
    }
    void SetAlignment(SGLLetteringContext::AlignmentType type) { m_lcLettering.typeAlignment=type; }
    void SetLineHeight(int height) { m_lcLettering.pitchLine=height; }
    int LineHeight() const { return m_lcLettering.pitchLine; }
    const SGLFontStyle &CurrentFont() const { return m_fsFont; }
    const SGLLetterer::Decoration &Decoration() const { return m_ltDecoration; }
    void SetDecoration(const SGLLetterer::Decoration &deco) { m_ltDecoration=deco; }
    ESLError FontFace(const wchar_t *face) {
        m_strFontCur=face;
        m_fsFont.pszFace=m_strFontCur;
        return static_cast<ESLError>(m_font.SetStyle(m_fsFont));
    }
    ESLError FontSize(int size) {
        if(size <= 0 || size > 4096) return eslErrInvalidParam;
        m_fsFont.nSize=size;
        return static_cast<ESLError>(m_font.SetStyle(m_fsFont));
    }
    ESLError FontFlag(uint32_t flag,bool enabled) {
        if(enabled) m_fsFont.nStyles|=flag; else m_fsFont.nStyles&=~flag;
        return static_cast<ESLError>(m_font.SetStyle(m_fsFont));
    }
    void BeginOutput() {
        FlushMessage();
        // Keep old characters fully shown while scheduling new text from now.
        m_msecLastTiming=m_msecFading;
    }
    ESLError AppendText(const std::wstring &text,size_t &consumed) {
        // Same native lettering/decoration pipeline as SGLSpriteMessage, with
        // its consumed count retained for the legacy OutputMessage ABI.
        SGLLetterer letterer;
        consumed=letterer.WriteLetter(m_font,m_lcLettering,text.c_str());
        const auto error=letterer.DecorateLetter(m_ltDecoration);
        if(error) return static_cast<ESLError>(error);
        SGLImageRect bounds;
        if(letterer.GetLetterLength()) {
            letterer.GetLetterRect(bounds);
            if(m_flagRectChars) m_rectChars|=SGLRect(bounds);
            else { m_rectChars=bounds; m_flagRectChars=true; }
        }
        for(size_t i=0;i<letterer.GetLetterLength();++i) {
            auto *letter=letterer.GetCharacterAt(i);
            if(!letter || !letter->pImage) continue;
            auto character=std::make_unique<Character>();
            character->m_pImage=new SGLImage;
            const auto status=character->m_pImage->CreateCloneBuffer(*letter->pImage);
            if(status) return static_cast<ESLError>(status);
            character->m_ptWriting=letter->ptWriting;
            character->m_ptOffset=letter->ptOffset;
            character->m_sizeChar=letter->sizeChar;
            character->m_wchCode=letter->wchCode;
            character->m_msecStart=m_msecLastTiming;
            m_characters.Add(character.release());
            m_msecLastTiming+=m_styleMsg.viewAction.msecPerChar;
        }
        m_msecDuration=m_msecLastTiming+m_styleMsg.viewAction.msecFade;
        PostUpdate();
        return eslErrSuccess;
    }
    ESLError AppendElement(const wchar_t *tag,const std::wstring &text,const wchar_t *attr=nullptr,const std::wstring &value=L"") {
        SSystem::SXMLDocument xml;
        xml.SetTag(tag);
        if(attr) xml.SetAttributeAs(attr,value.c_str());
        if(!text.empty()) { auto *child=new SSystem::SXMLDocument;child->SetText(text.c_str());xml.AddElement(child); }
        const auto result=AddLettering(xml);
        PostUpdate();
        return static_cast<ESLError>(result);
    }
    void Border(bool enabled,bool preserveWidth=false) {
        if(enabled) { m_ltDecoration.nFlags|=SGLLetterer::flagBorder;if(!preserveWidth)m_ltDecoration.widthBorder=2; }
        else m_ltDecoration.nFlags&=~SGLLetterer::flagBorder;
    }
    void ShadowTransparency(int transparency) {
        const unsigned alpha=(256-std::clamp(transparency,0,256))*255/256;
        m_ltDecoration.rgbaShadow=SGLPalette((m_ltDecoration.rgbaShadow.ui32 & 0xffffffu)|(alpha<<24));
    }
    ESLError CaptureStyle(SSystem::SXMLDocument& xml) const {
        if(m_ltDecoration.nGradationCount)return ESLErrorMsg("MessageSprite gradient text needs a saved gradient palette");
        auto tag=[&](const wchar_t* name){auto* node=new SSystem::SXMLDocument;node->SetTag(name);xml.AddElement(node);return node;};
        auto font=[&](const wchar_t* name,const SGLFontStyle& style){
            auto* node=tag(name);node->SetAttributeAs(L"face",style.pszFace?style.pszFace:L"");node->SetAttrIntegerAs(L"size",style.nSize);
            node->SetAttributeAs(L"bold",style.nStyles&SGLFontStyle::styleBold?L"true":L"false");
            node->SetAttributeAs(L"italic",style.nStyles&SGLFontStyle::styleItalic?L"true":L"false");
        };
        font(L"font",m_fsFont);font(L"ruby",m_styleMsg.fontRuby);
        auto* arrange=tag(L"arrange");
        const wchar_t* aligns[]={L"left",L"right",L"center",L"accordance"};
        arrange->SetAttributeAs(L"align",m_lcLettering.flagVertical?L"top":aligns[std::min<unsigned>(m_lcLettering.typeAlignment,3)]);
        arrange->SetAttrIntegerAs(L"line_height",m_lcLettering.pitchLine);arrange->SetAttrIntegerAs(L"indent",m_lcLettering.widthIndent);
        arrange->SetAttrIntegerAs(L"pitch",m_lcLettering.pitchChar);arrange->SetAttrIntegerAs(L"tab_pitch",m_lcLettering.pitchTab);
        const wchar_t* names[]={L"text",L"shadow",L"border"};
        const SGLPalette colors[]={m_ltDecoration.rgbaBody,m_ltDecoration.rgbaShadow,m_ltDecoration.rgbaBorder};
        for(int i=0;i<3;++i) {
            auto* node=tag(names[i]);node->SetAttrIntegerAs(L"color",colors[i].ui32&0xffffff);
            node->SetAttrIntegerAs(L"transparency",256-(uint32_t(colors[i].argb.Alpha)*256+254)/255);
            if(i==1){node->SetAttrIntegerAs(L"x",m_ltDecoration.ptShadow.x);node->SetAttrIntegerAs(L"y",m_ltDecoration.ptShadow.y);}
            if(i==2)node->SetAttrIntegerAs(L"width",m_ltDecoration.widthBorder);
        }
        // XML style records are extensible. Keep the original font/text tags,
        // with explicit native values to avoid premultiplication/alpha rounding
        // when a saved Android style is parsed again. No pointer is serialized.
        auto* exact=tag(L"android_style");exact->SetAttrIntegerAs(L"font_flags",m_fsFont.nStyles);
        exact->SetAttrIntegerAs(L"flags",m_ltDecoration.nFlags);
        exact->SetAttrIntegerAs(L"body",m_ltDecoration.rgbaBody.ui32);exact->SetAttrIntegerAs(L"shadow",m_ltDecoration.rgbaShadow.ui32);
        exact->SetAttrIntegerAs(L"border",m_ltDecoration.rgbaBorder.ui32);exact->SetAttrIntegerAs(L"border2",m_ltDecoration.rgbaBorder2.ui32);
        exact->SetAttrIntegerAs(L"width",m_ltDecoration.widthBorder);exact->SetAttrIntegerAs(L"width2",m_ltDecoration.widthBorder2);
        return eslErrSuccess;
    }
    ESLError RestoreStyle(const SSystem::SXMLDocument& xml) {
        auto style=GetMessageStyle();SSystem::SString font,ruby;
        SGLSpriteMessage::ParseMessageStyle(style,font,ruby,xml);
        SetMessageStyle(style);
        if(auto* exact=xml.GetElementTagAs(L"android_style")) {
            m_fsFont.nStyles=exact->GetAttrIntegerAs(L"font_flags",m_fsFont.nStyles);
            if(const auto error=m_font.SetStyle(m_fsFont))return ESLError(error);
            m_ltDecoration.nFlags=exact->GetAttrIntegerAs(L"flags",m_ltDecoration.nFlags);
            m_ltDecoration.rgbaBody=SGLPalette(uint32_t(exact->GetAttrIntegerAs(L"body",m_ltDecoration.rgbaBody.ui32)));
            m_ltDecoration.rgbaShadow=SGLPalette(uint32_t(exact->GetAttrIntegerAs(L"shadow",m_ltDecoration.rgbaShadow.ui32)));
            m_ltDecoration.rgbaBorder=SGLPalette(uint32_t(exact->GetAttrIntegerAs(L"border",m_ltDecoration.rgbaBorder.ui32)));
            m_ltDecoration.rgbaBorder2=SGLPalette(uint32_t(exact->GetAttrIntegerAs(L"border2",m_ltDecoration.rgbaBorder2.ui32)));
            m_ltDecoration.widthBorder=exact->GetAttrIntegerAs(L"width",m_ltDecoration.widthBorder);
            m_ltDecoration.widthBorder2=exact->GetAttrIntegerAs(L"width2",m_ltDecoration.widthBorder2);
        }
        return eslErrSuccess;
    }
};

ECSMessageSprite::ECSMessageSprite() : ECSSprite(new MessageBridge) {
    messageRect_=SGLRect(0,0,639,32767);
    auto initial=Bridge().GetMessageStyle();
    initial.context.rectWritable=messageRect_;
    initial.context.ptStartWriting=SGLPoint(messageRect_.left,messageRect_.top);
    Bridge().SetMessageStyle(initial);
    defaultNativeStyle_=Bridge().GetMessageStyle();
    defaultFont_=defaultNativeStyle_.font.pszFace;
    defaultRuby_=defaultNativeStyle_.fontRuby.pszFace;
    defaultNativeStyle_.font.pszFace=defaultFont_;
    defaultNativeStyle_.fontRuby.pszFace=defaultRuby_;
}
ECSMessageSprite::~ECSMessageSprite() {styleManager_.SetReference(nullptr,nullptr);}
ECSMessageSprite::MessageBridge &ECSMessageSprite::Bridge() { return *static_cast<MessageBridge *>(sprite_.get()); }
SGLSpriteMessage &ECSMessageSprite::NativeMessage() { return Bridge(); }
const wchar_t *ECSMessageSprite::GetTypeName() const { return L"MessageSprite"; }
ECSObject *ECSMessageSprite::GetTypeOf(const wchar_t *name) {
    return name&&!std::wcscmp(name,L"MessageSprite") ? this : ECSSprite::GetTypeOf(name);
}
ECSObject *ECSMessageSprite::Duplicate() {
    auto *copy=new ECSMessageSprite;
    if(copy->CopySprite(*this)) { delete copy;return nullptr; }
    return copy;
}
ESLError ECSMessageSprite::Release() {
    MessageLock lock;
    Bridge().ClearMessage();
    Bridge().AttachSkin(nullptr);
    skin_.reset();
    formattedLog_.clear();styleManager_.SetReference(nullptr,nullptr);
    return ECSSprite::Release();
}
ESLError ECSMessageSprite::Save(ESLFileObject& file,ECSContext& context) {
    MessageLock lock;
    if(messageRestorePending_)return ESLErrorMsg("MessageSprite restoration has not been committed");
    if(skin_&&!styleManager_.m_pRef)return ESLErrorMsg("MessageSprite style has no script ResourceManager provenance");
    SSystem::SXMLDocument style;EDescription description;EMemoryFile blob;
    if(const auto error=Bridge().CaptureStyle(style))return error;
    CopyStyleXML(style,description);
    if(const auto error=blob.Create(256))return error;
    if(const auto error=description.WriteDescription(blob,0,EDescription::dftXML,EDescription::ceUTF8))return error;
    if(const auto error=ECSSprite::Save(file,context))return error;
    LegacySave::Writer out{file};out.Reference(styleManager_,context);out.String(defaultStyle_.c_str());
    out.U32(blob.GetLength());out.Bytes(blob.GetBuffer(),blob.GetLength());
    const auto& action=Bridge().GetViewActionStyle();
    out.I32(defaultCharSpeed_);out.U32(action.msecPerChar);out.I32(defaultFade_);out.U32(action.msecFade);
    out.I32((Bridge().Decoration().nFlags&SGLLetterer::flagBorder)!=0);out.I32(inSize_);out.Rect(messageRect_);
    out.F64(action.vMove.x);out.F64(action.vMove.y);out.F64(action.vZoom.x);out.F64(action.vZoom.y);out.F64(action.zRotation);
    out.String(formattedLog_.c_str());return out.error;
}
ESLError ECSMessageSprite::Load(ESLFileObject& file,ECSContext& context) {
    MessageLock lock;
    if(const auto error=ECSSprite::Load(file,context))return error;
    LegacySave::Reader in{file};in.Reference(styleManager_,context);
    const auto styleName=in.String();if(in.error)return in.error;
    defaultStyle_=styleName.CharPtr()?styleName.CharPtr():L"";
    const auto length=in.Count(16*1024*1024);EStreamBuffer blob;
    if(in.error)return in.error;
    in.Bytes(blob.PutBuffer(length),length);blob.Flush(length);if(in.error)return in.error;
    EDescription description;
    if(length)if(const auto error=description.ReadDescription(blob,EDescription::dftAuto,EDescription::ceUTF8))return error;
    restoredStyle_=SSystem::SXMLDocument();CopyStyleXML(description,restoredStyle_);
    defaultCharSpeed_=in.I32();restoredAction_.msecPerChar=in.U32();defaultFade_=in.I32();restoredAction_.msecFade=in.U32();
    restoredBorder_=in.I32()!=0;inSize_=in.I32()!=0;messageRect_=in.Rect();
    restoredAction_.vMove.x=in.F64();restoredAction_.vMove.y=in.F64();
    restoredAction_.vZoom.x=in.F64();restoredAction_.vZoom.y=in.F64();restoredAction_.zRotation=in.F64();
    const auto textLog=in.String();if(in.error)return in.error;
    formattedLog_=textLog.CharPtr()?textLog.CharPtr():L"";
    if(defaultCharSpeed_<0||defaultFade_<0||restoredAction_.msecPerChar>INT32_MAX||restoredAction_.msecFade>INT32_MAX)return eslErrInvalidParam;
    messageRestorePending_=!in.error;return in.error;
}
ECSObject* ECSMessageSprite::GetVariableAt(int index) {return index==-11?&styleManager_:ECSSprite::GetVariableAt(index);}
void ECSMessageSprite::IndexAllMember() {
    ECSSprite::IndexAllMember();styleManager_.IndexAllMember();styleManager_.m_pParent=this;styleManager_.m_nIndex=-11;
}
void ECSMessageSprite::CleanupAllReference(ECSContext& context) {
    MessageLock lock;styleManager_.CleanupAllReference(context);ECSSprite::CleanupAllReference(context);
}
ESLError ECSMessageSprite::CommitAllReference(ECSContext& context) {
    MessageLock lock;
    if(!messageRestorePending_)return ECSSprite::CommitAllReference(context);
    if(const auto error=ECSSprite::CommitAllReference(context))return error;
    if(const auto error=styleManager_.CommitAllReference(context))return error;
    if(auto* manager=ESLTypeCast<ECSResourceManager>(styleManager_.m_pRef)) {
        if(const auto error=AttachMessageStyle(manager->GetSkin(),defaultStyle_.c_str()))return error;
    } else if(!defaultStyle_.empty())return ESLErrorMsg("Saved MessageSprite style manager is unavailable");
    if(const auto error=Bridge().RestoreStyle(restoredStyle_))return error;
    Bridge().SetWritable(messageRect_);Bridge().SetViewActionStyle(restoredAction_);Bridge().Border(restoredBorder_,true);
    // The original loader explicitly restores a completed message, not a partly
    // faded frame. Rebuild native glyphs from the exact formatted text log.
    const auto text=formattedLog_;formattedLog_.clear();Bridge().ClearMessage();
    size_t consumed=0;if(const auto error=OutputMessage(text.c_str(),consumed))return error;
    if(consumed!=text.size())return ESLErrorMsg("Saved MessageSprite text does not fit its restored writable rectangle");
    Bridge().FlushMessage();messageRestorePending_=false;return eslErrSuccess;
}
ESLError ECSMessageSprite::CreateMessage(int width,int height,const SGLRect *view,bool inSize) {
    MessageLock lock;
    if(width<1 || height<1 || width>16384 || height>16384) {
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Message CreateMessage invalid canvas %dx%d",width,height);
        return eslErrInvalidParam;
    }
    Bridge().ClearMessage();
    const auto result=Bridge().CreateBuffer(width,height);
    if(result) return static_cast<ESLError>(result);
    formattedLog_.clear();RecordLegacyImageCreation(SakuraGL::formatImageRGB,width,height);
    messageRect_=view?*view:SGLRect(0,0,width-1,32767);
    if(!inSize) messageRect_.bottom=32767;
    inSize_=inSize;
    Bridge().SetWritable(messageRect_);
    Bridge().ClearMessage();
    Bridge().Trace("create");
    return eslErrSuccess;
}
ESLError ECSMessageSprite::ApplyStyle(const wchar_t *id) {
    if(!skin_) return ESLErrorMsg("MessageSprite has no attached style manager");
    const auto *xml=skin_->GetStyleAs(id&&*id?id:defaultStyle_.c_str());
    if(!xml) return ESLErrorMsg("MessageSprite style was not found");
    auto style=Bridge().GetMessageStyle();
    const auto action=style.viewAction;
    const auto cursor=Bridge().GetNextMessagePoint();
    SSystem::SString font,ruby;
    SGLSpriteMessage::ParseMessageStyle(style,font,ruby,*xml);
    style.context.rectWritable=messageRect_;
    style.context.ptStartWriting=SGLPoint(messageRect_.left,messageRect_.top);
    style.viewAction=action;
    Bridge().SetMessageStyle(style);
    Bridge().SetCursor(cursor.x,cursor.y);
    Bridge().Trace("style");
    return eslErrSuccess;
}
ESLError ECSMessageSprite::AttachMessageStyle(std::shared_ptr<SGLSkinManager> skin,const wchar_t *id) {
    MessageLock lock;
    if(!skin) return eslErrInvalidParam;
    skin_=std::move(skin);defaultStyle_=id?id:L"";
    Bridge().AttachSkin(skin_.get());
    const auto status=ApplyStyle(defaultStyle_.c_str());
    if(status) return status;
    defaultNativeStyle_=Bridge().GetMessageStyle();
    defaultFont_=defaultNativeStyle_.font.pszFace;
    defaultRuby_=defaultNativeStyle_.fontRuby.pszFace;
    defaultNativeStyle_.font.pszFace=defaultFont_;
    defaultNativeStyle_.fontRuby.pszFace=defaultRuby_;
    return eslErrSuccess;
}
ESLError ECSMessageSprite::SetDefaultMsgSpeed(int charSpeed,int fadeSpeed,int ratio) {
    MessageLock lock;
    if(charSpeed<0 || fadeSpeed<0 || ratio<0 || uint64_t(charSpeed)*uint64_t(ratio)/256>UINT32_MAX) return eslErrInvalidParam;
    defaultCharSpeed_=charSpeed;defaultFade_=fadeSpeed;speedRatio_=ratio;
    auto action=Bridge().GetViewActionStyle();
    action.msecPerChar=uint64_t(charSpeed)*uint64_t(ratio)/256;
    action.msecFade=fadeSpeed;
    Bridge().SetViewActionStyle(action);
    return eslErrSuccess;
}

ESLError ECSMessageSprite::OutputMessage(const wchar_t *message,size_t &consumed) {
    MessageLock lock;
    const std::wstring input=message?message:L"";
    consumed=0;
    struct OutputTrace { MessageBridge& bridge;size_t length;size_t& consumed;~OutputTrace(){bridge.Trace("output",length,consumed);} } trace{Bridge(),input.size(),consumed};
    Bridge().BeginOutput();
    auto append=[&](const std::wstring &text,size_t &count) { return Bridge().AppendText(text,count); };
    auto enclosed=[&](size_t &at,wchar_t end,std::wstring &value) {
        const auto stop=input.find(end,at);
        if(stop==std::wstring::npos) return false;
        value=input.substr(at,stop-at);at=stop+1;return true;
    };
    auto numeric=[](const std::wstring &text,int &value,int base=10) {
        wchar_t *tail=nullptr;
        const long long n=std::wcstoll(text.c_str(),&tail,base);
        if(tail==text.c_str() || *tail || n<INT32_MIN || n>UINT32_MAX) return false;
        value=static_cast<int32_t>(n);return true;
    };
    size_t at=0;
    while(at<input.size()) {
        if(input[at]!=L'\\') {
            const auto stop=input.find(L'\\',at);
            const auto plain=input.substr(at,stop==std::wstring::npos?stop:stop-at);
            size_t written=0;
            const auto status=append(plain,written);
            if(status) return status;
            at+=written;consumed=at;
            if(written<plain.size()) break;
            Bridge().SetAlignment(SGLLetteringContext::alignLeft);
            continue;
        }
        const size_t commandStart=at++;
        const auto checkpoint=Bridge().SaveCheckpoint();
        if(at==input.size()) return ESLErrorMsg("Truncated MessageSprite escape");
        const wchar_t code=input[at++];
        ESLError status=eslErrSuccess;
        if(code==L'\\') {
            size_t written;
            status=append(L"\\",written);
            if(!written) { consumed=commandStart;return status; }
        } else if(code==L'b' || code==L'i') {
            status=Bridge().FontFlag(code==L'b'?SGLFontStyle::styleBold:SGLFontStyle::styleItalic,true);
        } else if(code==L'n') {
            if(at==input.size()) return ESLErrorMsg("Truncated MessageSprite n escape");
            const wchar_t next=input[at++];
            if(next==L'b' || next==L'i') status=Bridge().FontFlag(next==L'b'?SGLFontStyle::styleBold:SGLFontStyle::styleItalic,false);
            else if(next==L':') { size_t n;status=append(L"\n",n); }
            else if(next==L';') {
                bool done=false;
                while(!done) {
                    const auto stop=input.find_first_of(L";:",at);
                    if(stop==std::wstring::npos) return ESLErrorMsg("Unterminated MessageSprite word group");
                    status=Bridge().AppendElement(L"word",input.substr(at,stop-at));
                    done=input[stop]==L':';at=stop+1;
                    if(status) break;
                }
            } else return ESLErrorMsg("Unknown MessageSprite n escape");
        } else if(input.compare(commandStart+1,6,L"right:")==0 || input.compare(commandStart+1,7,L"center:")==0) {
            const bool center=code==L'c';
            at=commandStart+1+(center?7:6);
            Bridge().SetAlignment(center?SGLLetteringContext::alignCenter:SGLLetteringContext::alignRight);
        } else if(input.compare(commandStart+1,6,L"style;")==0) {
            at=commandStart+7;
            std::wstring style;
            if(!enclosed(at,L':',style)) return ESLErrorMsg("Unterminated MessageSprite style escape");
            status=ApplyStyle(style.c_str());
        } else if(code==L'r') {
            if(at>=input.size() || input[at++]!=L';') return ESLErrorMsg("Invalid MessageSprite ruby escape");
            std::wstring word,ruby;
            if(!enclosed(at,L';',word) || !enclosed(at,L':',ruby)) return ESLErrorMsg("Unterminated MessageSprite ruby escape");
            status=Bridge().AppendElement(L"ruby",word,L"reading",ruby);
        } else if(code==L'f') {
            if(at>=input.size()) return ESLErrorMsg("Truncated MessageSprite font escape");
            if(input[at++]==L';') {
                std::wstring face;
                if(!enclosed(at,L':',face)) return ESLErrorMsg("Unterminated MessageSprite font escape");
                status=Bridge().FontFace(face.c_str());
            } else status=Bridge().FontFace(defaultFont_);
        } else if(code==L's' || code==L'h') {
            if(at>=input.size()) return ESLErrorMsg("Truncated MessageSprite size escape");
            const wchar_t separator=input[at++];
            int value=code==L's'?int(defaultNativeStyle_.font.nSize):defaultNativeStyle_.context.pitchLine;
            if(separator==L';') {
                std::wstring text;
                if(!enclosed(at,L':',text) || !numeric(text,value)) return ESLErrorMsg("Invalid MessageSprite size escape");
                if(!text.empty() && (text.front()==L'+' || text.front()==L'-'))
                    value+=code==L's'?int(Bridge().CurrentFont().nSize):defaultNativeStyle_.context.pitchLine;
            } else if(separator!=L':') return ESLErrorMsg("Invalid MessageSprite size delimiter");
            if(code==L's') status=Bridge().FontSize(value);
            else Bridge().SetLineHeight(value);
        } else if(code==L'c') {
            if(at>=input.size()) return ESLErrorMsg("Truncated MessageSprite color escape");
            const wchar_t separator=input[at++];
            if(separator==L':') Bridge().SetDecoration(defaultNativeStyle_.decoration);
            else if(separator==L';') {
                const auto stop=input.find_first_of(L";:",at);
                if(stop==std::wstring::npos) return ESLErrorMsg("Unterminated MessageSprite color escape");
                int color;
                if(!numeric(input.substr(at,stop-at),color,16)) return ESLErrorMsg("Invalid MessageSprite color");
                Bridge().SetTextColor(SGLPalette((Bridge().Decoration().rgbaBody.ui32&0xff000000u)|(uint32_t(color)&0xffffffu)));
                at=stop+1;
                if(input[stop]==L';') {
                    std::wstring shadow;
                    if(!enclosed(at,L':',shadow) || !numeric(shadow,color,16)) return ESLErrorMsg("Invalid MessageSprite shadow color");
                    Bridge().SetTextShadowColor(SGLPalette((Bridge().Decoration().rgbaShadow.ui32&0xff000000u)|(uint32_t(color)&0xffffffu)));
                }
            } else return ESLErrorMsg("Invalid MessageSprite color delimiter");
        } else if(code==L'v') {
            if(at>=input.size()) return ESLErrorMsg("Truncated MessageSprite speed escape");
            const wchar_t separator=input[at++];
            int speed=int(uint64_t(defaultCharSpeed_)*speedRatio_/256),fade=defaultFade_;
            if(separator==L';') {
                const auto stop=input.find_first_of(L";:",at);
                if(stop==std::wstring::npos || !numeric(input.substr(at,stop-at),speed)) return ESLErrorMsg("Invalid MessageSprite speed");
                at=stop+1;
                if(input[stop]==L';') {
                    std::wstring value;
                    if(!enclosed(at,L':',value) || !numeric(value,fade)) return ESLErrorMsg("Invalid MessageSprite fade speed");
                }
            }
            if(speed<0 || fade<0) return eslErrInvalidParam;
            auto action=Bridge().GetViewActionStyle();action.msecPerChar=speed;action.msecFade=fade;
            Bridge().SetViewActionStyle(action);
        } else if(code==L'x') {
            if(at<input.size() && input[at]==L'c')
                return ESLErrorMsg("Tinted MessageSprite extension glyphs have not been ported");
            if(at>=input.size() || input[at++]!=L';') return ESLErrorMsg("Invalid MessageSprite extension glyph");
            std::wstring id;
            if(!enclosed(at,L':',id)) return ESLErrorMsg("Unterminated MessageSprite extension glyph");
            if(!skin_ || !skin_->GetImageAs(id.c_str())) return ESLErrorMsg("MessageSprite extension glyph was not found");
            status=Bridge().AppendElement(L"xfont",L"",L"id",id);
        } else return ESLErrorMsg("Unknown MessageSprite formatting escape");
        if(status==eslErrAbort) {
            Bridge().Rollback(checkpoint);consumed=commandStart;
            formattedLog_.append(input,0,consumed);return eslErrSuccess;
        }
        if(status) return status;
        consumed=at;
    }
    formattedLog_.append(input,0,consumed);
    return eslErrSuccess;
}

ESLError ECSMessageSprite::GetFunction(ECSContext &context,int &index,const wchar_t *name) {
    if(name) for(size_t i=0;i<sizeof(messageMethods)/sizeof(messageMethods[0]);++i) {
        if(!std::wcscmp(name,messageMethods[i])) { index=messageMethodBase+int(i);return eslErrSuccess; }
    }
    return ECSSprite::GetFunction(context,index,name);
}
ESLError ECSMessageSprite::CallFunction(ECSContext &context,int index,ECSObjArray<ECSObject> &args) {
    if(index<messageMethodBase) return ECSSprite::CallFunction(context,index,args);
    index-=messageMethodBase;
    if(index<0 || size_t(index)>=sizeof(messageMethods)/sizeof(messageMethods[0])) return ESLErrorMsg("Invalid MessageSprite method index");
    const auto *name=messageMethods[index];
    auto named=[name](const wchar_t *value) { return !std::wcscmp(name,value); };
    auto result=[&context](INT64 value=0) { return context.PushObject(new ECSInteger(value)); };
    auto count=[&](int min,int max) { return context.VerifyArgumentCount(args,min,max); };
    auto integer=[&](int &value,int i,int def=0) { return context.GetArgumentAsInt(value,args,i,def); };
    ESLError error;
    MessageLock lock;
    if(named(L"CreateMessage")) {
        if((error=count(3,5))) return error;
        int w,h,inSize;
        if((error=integer(w,1,1)) || (error=integer(h,2,1)) || (error=integer(inSize,4))) return error;
        auto *view=ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args,3,L"Rect"));
        SGLRect rect;
        if(view) rect=SGLRect(view->GetMemberAsInt(L"left",0),view->GetMemberAsInt(L"top",0),
                             view->GetMemberAsInt(L"right",w-1),view->GetMemberAsInt(L"bottom",h-1));
        return result(CreateMessage(w,h,view?&rect:nullptr,inSize!=0));
    }
    if(named(L"OutputMessage")) {
        if((error=count(2,2))) return error;
        ECSWideString text;
        if((error=context.GetArgumentAsStr(text,args,1,L""))) return error;
        size_t consumed;
        if((error=OutputMessage(text,consumed))) return error;
        return result(consumed);
    }
    if(named(L"FlushMessage") || named(L"ClearMessage") || named(L"IsMessagePending") || named(L"GetCharacterCount")) {
        if((error=count(1,1))) return error;
        if(named(L"FlushMessage")) Bridge().FlushMessage();
        else if(named(L"ClearMessage")) {Bridge().ClearMessage();formattedLog_.clear();}
        else if(named(L"IsMessagePending")) return result(Bridge().IsMessagePending()?-1:0);
        else return result(Bridge().GetMessageCharacterCount());
        return result();
    }
    if(named(L"AttachMessageStyle")) {
        if((error=count(3,3))) return error;
        auto *manager=ESLTypeCast<ECSResourceManager>(context.GetArgumentObjectAs(args,1,L"ResourceManager"));
        if(!manager) return ESLErrorMsg("AttachMessageStyle requires ResourceManager");
        ECSWideString style;
        if((error=context.GetArgumentAsStr(style,args,2,L""))) return error;
        error=AttachMessageStyle(manager->GetSkin(),style);
        if(!error)styleManager_.SetReference(manager,&context);
        return result(error);
    }
    if(named(L"SetDefaultMsgSpeed")) {
        if((error=count(3,4))) return error;
        int speed,fade,ratio;
        if((error=integer(speed,1)) || (error=integer(fade,2)) || (error=integer(ratio,3,256))) return error;
        return result(SetDefaultMsgSpeed(speed,fade,ratio));
    }
    if(named(L"SetMessageEffect")) {
        if((error=count(1,6))) return error;
        double x,y,mx,my,r;
        if((error=context.GetArgumentAsReal(x,args,1,0)) || (error=context.GetArgumentAsReal(y,args,2,0)) ||
           (error=context.GetArgumentAsReal(mx,args,3,1)) || (error=context.GetArgumentAsReal(my,args,4,1)) ||
           (error=context.GetArgumentAsReal(r,args,5,0))) return error;
        auto action=Bridge().GetViewActionStyle();action.vMove=S2DVector(x,y);action.vZoom=S2DVector(mx,my);action.zRotation=r;
        Bridge().SetViewActionStyle(action);return result();
    }
    if(named(L"SetShadowTransparency") || named(L"SetFontBordering")) {
        if((error=count(1,2))) return error;
        int value;
        if((error=integer(value,1,named(L"SetShadowTransparency")?256:0))) return error;
        if(named(L"SetShadowTransparency")) Bridge().ShadowTransparency(value);
        else Bridge().Border(value!=0);
        return result();
    }
    if(named(L"SetFontStyle") || named(L"SetFontFace")) {
        if((error=count(named(L"SetFontStyle")?1:2,2))) return error;
        ECSWideString value;
        if((error=context.GetArgumentAsStr(value,args,1,L""))) return error;
        return result(named(L"SetFontStyle")?ApplyStyle(value):Bridge().FontFace(value));
    }
    if(named(L"SetFontColor")) {
        if((error=count(2,3))) return error;
        int color,shadow;
        if((error=integer(color,1,-1)) || (error=integer(shadow,2,0))) return error;
        Bridge().SetTextColor(SGLPalette((Bridge().Decoration().rgbaBody.ui32&0xff000000u)|(uint32_t(color)&0xffffffu)));Bridge().SetTextShadowColor(SGLPalette((Bridge().Decoration().rgbaShadow.ui32&0xff000000u)|(uint32_t(shadow)&0xffffffu)));
        return result();
    }
    if(named(L"GetCursorPos")) {
        if((error=count(1,1))) return error;
        auto *point=context.CreateUserStructure(L"Point");
        if(!point) return ESLErrorMsg("Missing Point class");
        const auto cursor=Bridge().GetNextMessagePoint();
        point->SetMemberAsInt(L"x",cursor.x);point->SetMemberAsInt(L"y",cursor.y);
        return context.PushObject(*point);
    }
    if(named(L"MoveCursorPos")) {
        if((error=count(3,3))) return error;
        int x,y;
        if((error=integer(x,1)) || (error=integer(y,2))) return error;
        Bridge().SetCursor(x,y);return result();
    }
    if(named(L"GetMessageRect")) {
        if((error=count(1,1))) return error;
        SGLRect bounds(0,0,-1,-1);
        Bridge().GetCircumscribedRect(bounds);
        auto *rect=context.CreateUserStructure(L"Rect");
        if(!rect) return ESLErrorMsg("Missing Rect class");
        rect->SetMemberAsInt(L"left",bounds.left);rect->SetMemberAsInt(L"top",bounds.top);
        rect->SetMemberAsInt(L"right",bounds.right);rect->SetMemberAsInt(L"bottom",bounds.bottom);
        return context.PushObject(*rect);
    }
    return ESLErrorMsg("Unimplemented MessageSprite method");
}

bool CheckLegacyMessage() {
    auto check=[](bool ok,const char *stage) {
        if(!ok) study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy MessageSprite probe FAIL: %s",stage);
        return ok;
    };
    ECSMessageSprite sprite;
    if(!check(!sprite.CreateMessage(512,160),"real message buffer")) return false;
    if(!check(!sprite.SetDefaultMsgSpeed(30,60),"typewriter timing")) return false;
    auto& native=sprite.NativeMessage();
    auto draw=[&]{native.PrepareDrawFrame();native.BeforeDraw();native.FinishDrawFrame();};
    auto hasInk=[&] {
        auto* image=native.GetAttachedImage();if(!image)return false;
        const auto size=image->GetImageSize();SGLPalette pixel;
        for(int y=0;y<size.h;++y)for(int x=0;x<size.w;++x)
            if(!image->GetPixelRGBA(pixel,x,y)&&pixel.argb.Alpha)return true;
        return false;
    };
    draw();
    if(!check(!native.HasUpdate()&&!hasInk(),"initial empty framebuffer is clean"))return false;
    size_t consumed=0;
    const wchar_t text[]=L"日本語\\n:試験\\r;漢字;かんじ:";
    if(!check(!sprite.OutputMessage(text,consumed) && consumed==std::wcslen(text),"Japanese text, newline and ruby formatting")) return false;
    SGLRect bounds;
    if(!check(native.GetMessageCharacterCount()>=7 && native.GetCircumscribedRect(bounds) &&
              !bounds.IsEmpty() && native.IsMessagePending(),"rasterized glyphs and pending display")) return false;
    bool hasRaster=false;
    for(size_t i=0;i<native.GetMessageCharacterCount();++i) {
        auto *character=native.GetMessageCharacterAt(i);
        if(character && character->m_pImage) {
            const auto size=character->m_pImage->GetImageSize();
            if(size.w>0 && size.h>0) hasRaster=true;
        }
    }
    if(!check(hasRaster,"actual native font images")) return false;
    native.AdvanceTime(10000);
    if(!check(!native.IsMessagePending(),"AdvanceTime finishes typewriter")) return false;
    draw();if(!check(hasInk(),"glyph pixels refresh a previously clean local framebuffer"))return false;
    if(!check(!sprite.OutputMessage(L"終",consumed) && native.IsMessagePending(),"append starts new timing")) return false;
    native.FlushMessage();
    if(!check(!native.IsMessagePending(),"FlushMessage")) return false;
    native.ClearMessage();
    if(!check(native.GetMessageCharacterCount()==0 && !native.IsMessagePending(),"ClearMessage lifecycle")) return false;
    draw();if(!check(!hasInk(),"native ClearMessage removes previously rendered framebuffer ink"))return false;
    if(!check(!sprite.OutputMessage(L"再",consumed),"output after clean clear"))return false;
    native.FlushMessage();draw();if(!check(hasInk(),"native FlushMessage repaints timed glyphs without AdvanceTime"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy MessageSprite probe PASS: font rasterization, Japanese/ruby/newline, timed display, flush/clear");
    return true;
}
