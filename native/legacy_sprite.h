#pragma once
#include "legacy_resource.h"
#include <sakuraglx/sprite/sglx_sprite.h>
#include <sakuraglx/sprite/sglx_resource_manager.h>
#include <memory>
#include <array>

struct LegacySpriteDrawState;
struct LegacySpriteCallbackState;
class ECSResourceManager;

class ECSSprite : public ECSResource {
public:
    DECLARE_CLASS_INFO(ECSSprite, ECSResource)
    ECSSprite();
    ~ECSSprite() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *name) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &context, ECSObject *source) override;
    ESLError GetFunction(ECSContext &context, int &index, const wchar_t *name) override;
    ESLError CallFunction(ECSContext &context, int index, ECSObjArray<ECSObject> &args) override;
    ESLError ReadImageFile(ESLFileObject &file) override;
    ESLError Release() override;
    ECSObject* GetVariableAt(int index) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
    SakuraGL::SGLImageObject* GetImage() const override;
    ESLError BuildFormPage(std::shared_ptr<SakuraGL::SGLSkinManager> skin, const wchar_t* page);
    SakuraGL::SGLSprite &NativeSprite() { return *sprite_; }
    std::shared_ptr<LegacySpriteDrawState>& LegacyDrawState() { return legacyDrawState_; }
    std::shared_ptr<LegacySpriteCallbackState>& LegacyCallbackState() { return legacyCallbackState_; }
    void RecordLegacyFormSource(ECSResourceManager*, const wchar_t*, ECSContext&);
    void RecordLegacyImageSource(ECSResource*, int frame, const SakuraGL::SGLRect*, ECSContext&);
    void RecordLegacyImageCreation(uint32_t format, uint32_t width, uint32_t height);
    void RecordLegacyItemString(const wchar_t* id, const wchar_t* member, const wchar_t* value);
    void RecordLegacyItemInteger(const wchar_t* id, const wchar_t* member, int value);
    bool IsLegacyRestorePending() const {return legacyRestorePending_;}
    bool EnableLegacyDynamicMode(bool enabled);
    bool IsLegacyDynamicModeEnabled() const {return legacyDynamicMode_;}
protected:
    explicit ECSSprite(SakuraGL::SGLSprite* nativeSprite);
    std::unique_ptr<SakuraGL::SGLSprite> sprite_;
    std::shared_ptr<SakuraGL::SGLSkinManager> formSkin_;
    std::shared_ptr<LegacySpriteDrawState> legacyDrawState_;
    std::shared_ptr<LegacySpriteCallbackState> legacyCallbackState_;
    // Original hidden reference order -2..-6: image, alpha, parent, manager, tone.
    std::array<ECSReference,5> legacyReferences_;
    ECSHash legacyItemState_;
    ECSWideString legacyPage_;
    uint32_t legacyImageFormat_=UINT32_MAX, legacyImageWidth_=0, legacyImageHeight_=0;
    int32_t legacyImageFrame_=-1;
    SakuraGL::SGLRect legacyImageView_{0,0,-1,-1};
    bool legacyRestoring_=false,legacyRestorePending_=false,legacyRestoreCommitting_=false;
    // GLS3 keeps this switch across Release; it is not an image ownership flag.
    bool legacyDynamicMode_=false;
    ESLError RestoreLegacyItems();
    ECSHash& LegacyItemState(const wchar_t* id);
    SakuraGL::SGLSprite *Item(const wchar_t *name);
    ESLError CopySprite(const ECSSprite &source);
};
