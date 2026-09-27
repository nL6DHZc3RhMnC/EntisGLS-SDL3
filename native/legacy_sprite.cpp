#include "legacy_compat/gls.h"
#include "legacy_sprite.h"
#include "legacy_sprite_draw.h"
#include "legacy_sprite_callbacks.h"
#include "legacy_save_io.h"
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>
#include <algorithm>
#include "platform/log.h"
#include <cwchar>
#include <map>

IMPLEMENT_CLASS_INFO(ECSSprite, ECSResource)
namespace {
constexpr int spriteMethodBase = 1024;
const wchar_t *const methods[] = {
#include "legacy_sprite_methods.inc"
};
// Accessed under the engine's recursive global lock, including destruction.
std::map<SakuraGL::SGLSprite *, ECSSprite *> owners;
struct GraphicsLock {
    GraphicsLock() { SSystem::Lock(); }
    ~GraphicsLock() { SSystem::Unlock(); }
};
}

bool LegacySpriteDynamicModeEnabled(const SakuraGL::SGLSprite* native) {
    GraphicsLock lock;
    const auto found=owners.find(const_cast<SakuraGL::SGLSprite*>(native));
    return found!=owners.end()&&found->second->IsLegacyDynamicModeEnabled();
}
bool ECSSprite::EnableLegacyDynamicMode(bool enabled) {
    GraphicsLock lock;
    // Notify both old and new bounds: direct children can extend beyond a
    // buffered image, and switching back must erase those previous pixels.
    const bool old=legacyDynamicMode_;
    sprite_->NotifyUpdate();legacyDynamicMode_=enabled;
    sprite_->PostUpdate();sprite_->NotifyUpdate();
    return old;
}

ECSSprite::ECSSprite() : ECSSprite(new LegacyCallbackSprite<SakuraGL::SGLSpriteFormed>) {}
ECSSprite::ECSSprite(SakuraGL::SGLSprite* nativeSprite) : sprite_(nativeSprite) {
    GraphicsLock lock;
    owners[sprite_.get()] = this;
}
ECSSprite::~ECSSprite() {
    ResetLegacySpriteCallbacks(*this);
    ResetLegacySpriteDrawState(*this);
    GraphicsLock lock;
    if (auto *parent = sprite_->GetParent()) parent->DetachChild(sprite_.get());
    sprite_->DetachAllChildren();
    owners.erase(sprite_.get());
    sprite_.reset();
    for(auto& ref:legacyReferences_) ref.SetReference(nullptr,nullptr);
}
const wchar_t *ECSSprite::GetTypeName() const { return L"Sprite"; }
ECSObject *ECSSprite::GetTypeOf(const wchar_t *name) {
    return name && (!std::wcscmp(name, L"Sprite") || !std::wcscmp(name, L"Resource")) ? this : nullptr;
}
SakuraGL::SGLSprite *ECSSprite::Item(const wchar_t *name) {
    return name && *name ? sprite_->GetItemAs(name) : sprite_.get();
}
SakuraGL::SGLImageObject* ECSSprite::GetImage() const {
    if(auto* buffer=sprite_->GetFrameBuffer())return buffer->GetImage();
    return m_image ? m_image.get() : sprite_->GetAttachedImage();
}
ESLError ECSSprite::ReadImageFile(ESLFileObject &file) {
    GraphicsLock lock;
    const auto error = ECSResource::ReadImageFile(file);
    if (!error) sprite_->AttachImage(GetImage());
    return error;
}
ESLError ECSSprite::Release() {
    ResetLegacySpriteDrawState(*this,true);
    GraphicsLock lock;
    auto* formed = ESLTypeCast<SakuraGL::SGLSpriteFormed>(sprite_.get());
    if(formed)formed->AttachSkin(nullptr);
    // ReleaseForm also detaches script children, losing their order and focus.
    // GLS3 removes only skin items; retain the existing script child links.
    for(size_t i=sprite_->GetChildCount();i>0;--i) {
        auto* child=sprite_->GetChildAt(i-1);
        if(owners.find(child)==owners.end())sprite_->DetachChild(child);
    }
    if(formed){formed->AttachFormParser(nullptr);formed->SetBasicForm(nullptr);}
    formSkin_.reset();
    sprite_->AttachImage(nullptr);
    sprite_->ReleaseBuffer();
    if(!legacyRestoring_) {
        legacyReferences_[0].SetReference(nullptr,nullptr);
        legacyReferences_[1].SetReference(nullptr,nullptr);
        legacyReferences_[3].SetReference(nullptr,nullptr);
        legacyItemState_.m_varArray.RemoveAll();legacyPage_=L"";
        legacyImageFormat_=UINT32_MAX;legacyImageWidth_=legacyImageHeight_=0;
        legacyImageFrame_=-1;legacyImageView_=SakuraGL::SGLRect(0,0,-1,-1);
    }
    return ECSResource::Release();
}
ECSObject* ECSSprite::GetVariableAt(int index) {
    if(index<=-2 && index>=-6)return &legacyReferences_[-2-index];
    if (index <= -7 && index >= -10) return GetLegacySpriteCallbackAt(*this, index);
    return ECSResource::GetVariableAt(index);
}
void ECSSprite::IndexAllMember() {
    GraphicsLock lock;
    if(!legacyRestorePending_) {
        const auto parent=owners.find(sprite_->GetParent());
        legacyReferences_[2].SetReference(parent==owners.end()?nullptr:parent->second);
    }
    ECSResource::IndexAllMember();
    for(int i=0;i<5;++i) {
        auto& ref=legacyReferences_[i];ref.IndexAllMember();ref.m_pParent=this;ref.m_nIndex=-2-i;
    }
    legacyItemState_.IndexAllMember();
    IndexLegacySpriteCallbacks(*this);
}
void ECSSprite::CleanupAllReference(ECSContext& context) {
    GraphicsLock lock;
    CleanupLegacySpriteCallbacks(*this, context);
    for(auto& ref:legacyReferences_) ref.CleanupAllReference(context);
    legacyItemState_.CleanupAllReference(context);
    ECSResource::CleanupAllReference(context);
}
#include "legacy_sprite_state.inc"
void ECSSprite::RecordLegacyFormSource(ECSResourceManager* manager,const wchar_t* page,ECSContext& context) {
    GraphicsLock lock;legacyReferences_[3].SetReference(manager,&context);legacyPage_=page?page:L"";
}
void ECSSprite::RecordLegacyImageSource(ECSResource* source,int frame,const SakuraGL::SGLRect* view,ECSContext& context) {
    GraphicsLock lock;legacyReferences_[0].SetReference(source,&context);legacyImageFrame_=frame;
    const auto size=sprite_->GetImageSize();
    legacyImageView_=view?*view:SakuraGL::SGLRect(0,0,size.w-1,size.h-1);
}
void ECSSprite::RecordLegacyImageCreation(uint32_t format,uint32_t width,uint32_t height) {
    GraphicsLock lock;legacyImageFormat_=format;legacyImageWidth_=width;legacyImageHeight_=height;
}
ECSHash& ECSSprite::LegacyItemState(const wchar_t* id) {
    auto* state=ESLTypeCast<ECSHash>(legacyItemState_.m_varArray.GetAs(id?id:L""));
    if(!state){state=new ECSHash;legacyItemState_.m_varArray.SetAs(id?id:L"",state);}
    return *state;
}
void ECSSprite::RecordLegacyItemString(const wchar_t* id,const wchar_t* member,const wchar_t* value) {
    GraphicsLock lock;LegacyItemState(id).SetMemberAsStr(member,value?value:L"");
}
void ECSSprite::RecordLegacyItemInteger(const wchar_t* id,const wchar_t* member,int value) {
    GraphicsLock lock;LegacyItemState(id).SetMemberAsInt(member,value);
}
ESLError ECSSprite::BuildFormPage(std::shared_ptr<SakuraGL::SGLSkinManager> skin, const wchar_t* page) {
    GraphicsLock lock;
    auto* formed = ESLTypeCast<SakuraGL::SGLSpriteFormed>(sprite_.get());
    if (!skin || !formed || !skin->GetFormAs(page)) return eslErrGeneral;
    Release();
    if (skin->BuildFormedPage(*formed, page)) { Release(); return eslErrGeneral; }
    formSkin_ = std::move(skin);
    legacyPage_=page?page:L"";
    return eslErrSuccess;
}
ESLError ECSSprite::CopySprite(const ECSSprite &source) {
    GraphicsLock lock;
    if (&source == this) return eslErrSuccess;
    if (source.m_image) {
        auto copy = std::make_shared<SakuraGL::SGLImage>();
        const auto error = copy->CreateCloneImage(*source.m_image);
        if (error) return static_cast<ESLError>(error);
        m_image = std::move(copy);
    } else m_image.reset();
    // CreateCloneImage owns decoded pixels, but a file-backed clone still has
    // the same reopenable origin. Do not retain a destination's stale path.
    m_imageStream.reset();
    m_wstrFileName = source.m_image ? source.m_wstrFileName : EWideString();
    legacyReferences_[0].SetReference(nullptr,nullptr);
    legacyImageFrame_ = -1;legacyImageView_ = SakuraGL::SGLRect(0,0,-1,-1);
    sprite_->AttachImage(m_image.get());
    sprite_->SetParameter(source.sprite_->GetParameter());
    return eslErrSuccess;
}
ECSObject *ECSSprite::Duplicate() {
    auto *copy = new ECSSprite;
    if (copy->CopySprite(*this)) { delete copy; return nullptr; }
    return copy;
}
ESLError ECSSprite::Move(ECSContext &context, ECSObject *source) {
    auto *sprite = ESLTypeCast<ECSSprite>(ECSObject::GetEntity(source));
    if (!sprite) return ESLErrorMsg("Sprite assignment requires a Sprite");
    const auto error = CopySprite(*sprite);
    if (!error) context.delete_CSObject(source);
    return error;
}
ESLError ECSSprite::GetFunction(ECSContext &context, int &index, const wchar_t *name) {
    if (name) for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); ++i) {
        if (!std::wcscmp(name, methods[i])) {
            index = spriteMethodBase + static_cast<int>(i);
            return eslErrSuccess;
        }
    }
    return ECSResource::GetFunction(context, index, name);
}

ESLError ECSSprite::CallFunction(ECSContext &context, int index, ECSObjArray<ECSObject> &args) {
    if (index < spriteMethodBase) return ECSResource::CallFunction(context, index, args);
    index -= spriteMethodBase;
    if (index < 0 || static_cast<size_t>(index) >= sizeof(methods) / sizeof(methods[0]))
        return ESLErrorMsg("Invalid Sprite method index");
    const wchar_t *name = methods[index];
    auto named = [name](const wchar_t *value) { return std::wcscmp(name, value) == 0; };
    auto result = [&context](INT64 value = 0) { return context.PushObject(new ECSInteger(value)); };
    ESLError error = eslErrSuccess;
    bool handled = false;
    error = HandleLegacySpriteCallbacks(context, *this, name, args, handled);
    if (handled) return error;
    error = HandleLegacySpriteDraw(context, *this, name, args, handled);
    if (handled) return error;
    GraphicsLock lock;
    if (named(L"EnableDynamicMode")) {
        if ((error=context.VerifyArgumentCount(args,2))) return error;
        int enabled;
        if ((error=context.GetArgumentAsInt(enabled,args,1,0))) return error;
        return result(EnableLegacyDynamicMode(enabled!=0)?-1:0);
    }
    if (named(L"GetInfo") || named(L"GetImageInfo")) {
        if ((error = context.VerifyArgumentCount(args, 1))) return error;
        if (m_image) return PushMediaInfo(context, false);
        SakuraGL::SGLImageInfo native;
        auto* info = context.CreateUserStructure(L"ImageInfo");
        if (!info) return ESLErrorMsg("Missing ImageInfo class");
        if (!sprite_->GetImageInfo(native)) {
            info->SetMemberAsInt(L"nFormatType", native.format);
            info->SetMemberAsInt(L"nImageWidth", native.width);
            info->SetMemberAsInt(L"nImageHeight", native.height);
            info->SetMemberAsInt(L"nBitsPerPixel", native.depth);
            info->SetMemberAsInt(L"nFrameCount", 1);
            info->SetMemberAsInt(L"nResourceBytes", std::abs(native.pitchLine) * native.height);
        }
        return context.PushObject(*info);
    }
    if (named(L"SetBackColor")) {
        if ((error = context.VerifyArgumentCount(args, 3))) return error;
        int color, fill;
        if ((error = context.GetArgumentAsInt(color, args, 1, 0)) ||
            (error = context.GetArgumentAsInt(fill, args, 2, 0))) return error;
        return result(sprite_->SetFillBackColor(static_cast<uint32_t>(color), fill != 0));
    }
    if (named(L"Refresh")) {
        if ((error = context.VerifyArgumentCount(args, 1))) return error;
        sprite_->Refresh();
        return result();
    }
    if (named(L"UpdateRect")) {
        if ((error = context.VerifyArgumentCount(args, 1, 2))) return error;
        auto* rect = ESLTypeCast<ECSStructureInterface>(context.GetArgumentObjectAs(args, 1, L"Rect"));
        SakuraGL::SGLRect native;
        if (rect) {
            native.left = rect->GetMemberAsInt(L"left", 0);
            native.top = rect->GetMemberAsInt(L"top", 0);
            native.right = rect->GetMemberAsInt(L"right", 0);
            native.bottom = rect->GetMemberAsInt(L"bottom", 0);
        }
        sprite_->PostUpdate(rect ? &native : nullptr);
        return result();
    }
    if (named(L"SetZPosition")) {
        if ((error = context.VerifyArgumentCount(args, 2))) return error;
        double z;
        if ((error = context.GetArgumentAsReal(z, args, 1, 0))) return error;
        const auto old = sprite_->GetPosition();
        sprite_->SetPosition3D(old.x, old.y, z);
        return result();
    }
    if (named(L"GetSpriteID") || named(L"SetSpriteID")) {
        if ((error = context.VerifyArgumentCount(args, named(L"SetSpriteID") ? 2 : 1))) return error;
        if (named(L"GetSpriteID")) return context.PushObject(new ECSString(sprite_->GetID()));
        ECSWideString id;
        if ((error = context.GetArgumentAsStr(id, args, 1, nullptr))) return error;
        sprite_->SetID(id);
        return result();
    }
    if (named(L"GetSpriteText") || named(L"SetSpriteText") || named(L"SetSpriteFontFace") ||
        named(L"SetSpriteImage") || named(L"IsButtonChecked") || named(L"CheckButton")) {
        const bool query = named(L"GetSpriteText") || named(L"IsButtonChecked");
        if ((error = context.VerifyArgumentCount(args, query ? 1 : 3, query ? 2 : 3))) return error;
        ECSWideString id, value;
        if ((error = context.GetArgumentAsStr(id, args, 1, nullptr))) return error;
        auto* formed = ESLTypeCast<SakuraGL::SGLSpriteFormed>(sprite_.get());
        if (!formed) return ESLErrorMsg("Sprite does not support form items");
        if (named(L"GetSpriteText")) return context.PushObject(new ECSString(formed->GetSpriteText(id)));
        if (named(L"IsButtonChecked")) return result(formed->IsSpriteButtonChecked(id) ? -1 : 0);
        if (named(L"CheckButton")) {
            int checked;
            if ((error = context.GetArgumentAsInt(checked, args, 2, 1))) return error;
            // GLS3 checks the actual Button type and leaves absent/other
            // controls untouched. Do not turn that no-op into saved state.
            if(ESLTypeCast<SakuraGL::SGLSpriteButton>(Item(id))) {
                formed->CheckSpriteButton(id, checked != 0);
                RecordLegacyItemInteger(id,L"checked",checked);
            }
        } else {
            if ((error = context.GetArgumentAsStr(value, args, 2, nullptr))) return error;
            if (named(L"SetSpriteText")) formed->SetSpriteText(id, value);
            else if (named(L"SetSpriteImage")) formed->SetSpriteImage(id, value);
            else formed->SetSpriteTextFont(id, value);
            if(Item(id))RecordLegacyItemString(id,named(L"SetSpriteText")?L"text":named(L"SetSpriteImage")?L"image":L"font",value);
        }
        return result();
    }
    if (named(L"CreateSprite")) {
        if ((error = context.VerifyArgumentCount(args, 4))) return error;
        int format, width, height;
        if ((error = context.GetArgumentAsInt(format, args, 1, 1)) ||
            (error = context.GetArgumentAsInt(width, args, 2, 16)) ||
            (error = context.GetArgumentAsInt(height, args, 3, 16))) return error;
        if (width < 1 || height < 1 || width > 16384 || height > 16384)
            return ESLErrorMsg("Sprite dimensions are outside the supported range");
        Release();
        const auto status=sprite_->CreateBuffer(width,height,format,
            (format & 0xff) == SakuraGL::formatImageGray ? 8 : 32);
        if(!status)RecordLegacyImageCreation(format,width,height);
        return result(status);
    }
    if (named(L"Release")) {
        if ((error = context.VerifyArgumentCount(args, 1))) return error;
        return result(Release());
    }
    if (named(L"IsVisible") || named(L"GetParent") || named(L"GetZPosition") || named(L"GetPosition")) {
        if ((error = context.VerifyArgumentCount(args, 1))) return error;
        if (named(L"IsVisible")) return result(sprite_->IsVisible() ? -1 : 0);
        if (named(L"GetZPosition")) return context.PushObject(new ECSReal(sprite_->GetPosition().z));
        if (named(L"GetParent")) {
            auto found = owners.find(sprite_->GetParent());
            return context.PushObject(new ECSReference(found == owners.end() ? nullptr : found->second));
        }
        auto *point = context.CreateUserStructure(L"Point");
        if (!point) return ESLErrorMsg("Missing Point class");
        point->SetMemberAsInt(L"x", static_cast<INT64>(sprite_->GetPosition().x));
        point->SetMemberAsInt(L"y", static_cast<INT64>(sprite_->GetPosition().y));
        return context.PushObject(*point);
    }
    if (named(L"SetVisible")) {
        if ((error = context.VerifyArgumentCount(args, 2))) return error;
        int visible;
        if ((error = context.GetArgumentAsInt(visible, args, 1, 1))) return error;
        sprite_->SetVisible(visible != 0);
        return result();
    }
    if (named(L"MovePosition")) {
        if ((error = context.VerifyArgumentCount(args, 3))) return error;
        int x, y;
        if ((error = context.GetArgumentAsInt(x, args, 1, 0)) ||
            (error = context.GetArgumentAsInt(y, args, 2, 0))) return error;
        sprite_->SetPosition(x, y);
        return result();
    }
    if (named(L"GetTransparency") || named(L"GetPriority") || named(L"IsEnabled")) {
        if ((error = context.VerifyArgumentCount(args, 1, 2))) return error;
        ECSWideString id;
        if ((error = context.GetArgumentAsStr(id, args, 1, nullptr))) return error;
        auto *item = Item(id);
        if (!item) return result();
        if (named(L"GetTransparency")) return result(item->GetTransparency());
        if (named(L"GetPriority")) return result(item->GetPriority());
        return result(item->IsEnabled() ? -1 : 0);
    }
    if (named(L"SetTransparency") || named(L"ChangePriority") || named(L"Enable")) {
        if ((error = context.VerifyArgumentCount(args, 2, 3))) return error;
        int value;
        ECSWideString id;
        if ((error = context.GetArgumentAsInt(value, args, 1, 0)) ||
            (error = context.GetArgumentAsStr(id, args, 2, nullptr))) return error;
        auto *item = Item(id);
        if (item) {
            if (named(L"SetTransparency")) item->SetTransparency(value);
            else if (named(L"ChangePriority")) item->ChangePriority(value);
            else item->SetEnable(value != 0);
            if(!id.IsEmpty())RecordLegacyItemInteger(id,named(L"SetTransparency")?L"transparency":named(L"ChangePriority")?L"priority":L"enabled",value);
        }
        return result();
    }
    if (named(L"AddSprite") || named(L"DetachSprite")) {
        const bool add = named(L"AddSprite");
        if ((error = context.VerifyArgumentCount(args, add ? 3 : 2))) return error;
        int priority = 0;
        if (add && (error = context.GetArgumentAsInt(priority, args, 1, 0))) return error;
        auto *child = ESLTypeCast<ECSSprite>(context.GetArgumentObjectAs(args, add ? 2 : 1, L"Sprite"));
        if (!child) return ESLErrorMsg("Sprite child argument must be a Sprite");
        if (add) {
            for (auto *ancestor = sprite_.get(); ancestor; ancestor = ancestor->GetParent())
                if (ancestor == child->sprite_.get()) return ESLErrorMsg("Sprite parenting would create a cycle");
            if (auto *parent = child->sprite_->GetParent()) parent->DetachChild(child->sprite_.get());
            child->sprite_->ChangePriority(priority);
            sprite_->AddChild(child->sprite_.get());
        } else sprite_->DetachChild(child->sprite_.get());
        return result();
    }
    if (named(L"DetachAllSprite")) {
        if ((error = context.VerifyArgumentCount(args, 1))) return error;
        sprite_->DetachAllChildren();
        return result();
    }
    if (named(L"CopyParameters")) {
        if ((error = context.VerifyArgumentCount(args, 2))) return error;
        auto *source = ESLTypeCast<ECSSprite>(context.GetArgumentObjectAs(args, 1, L"Sprite"));
        if (!source) return ESLErrorMsg("CopyParameters requires a Sprite");
        sprite_->SetParameter(source->sprite_->GetParameter());
        return result();
    }
    context.m_strErrMsg = EString(L"Sprite.") + EString(name) + " is not connected to the Android graphics backend";
    return ESLErrorMsg(context.m_strErrMsg);
}
