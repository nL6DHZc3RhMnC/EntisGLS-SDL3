#pragma once
#include "legacy_sprite.h"
#include "legacy_resource_manager.h"
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <string>

class ECSMessageSprite : public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSMessageSprite, ECSSprite)
    ECSMessageSprite();
    ~ECSMessageSprite() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError GetFunction(ECSContext &, int &, const wchar_t *) override;
    ESLError CallFunction(ECSContext &, int, ECSObjArray<ECSObject> &) override;
    ESLError Release() override;
    ESLError Save(ESLFileObject &, ECSContext &) override;
    ESLError Load(ESLFileObject &, ECSContext &) override;
    ECSObject* GetVariableAt(int) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    SakuraGL::SGLSpriteMessage &NativeMessage();
    ESLError CreateMessage(int width, int height, const SakuraGL::SGLRect *view = nullptr, bool inSize = false);
    ESLError OutputMessage(const wchar_t *, size_t &consumed);
    ESLError AttachMessageStyle(std::shared_ptr<SakuraGL::SGLSkinManager>, const wchar_t *);
    ESLError SetDefaultMsgSpeed(int charSpeed, int fadeSpeed, int ratio = 256);
private:
    class MessageBridge;
    MessageBridge &Bridge();
    std::shared_ptr<SakuraGL::SGLSkinManager> skin_;
    std::wstring defaultStyle_;
    SakuraGL::SGLRect messageRect_;
    SakuraGL::SGLSpriteMessage::MessageStyle defaultNativeStyle_;
    SSystem::SString defaultFont_, defaultRuby_;
    int defaultCharSpeed_ = 0, defaultFade_ = 0, speedRatio_ = 256;
    bool inSize_ = false;
    ECSReference styleManager_;
    std::wstring formattedLog_;
    SSystem::SXMLDocument restoredStyle_;
    SakuraGL::SGLSpriteMessage::ViewActionStyle restoredAction_;
    bool messageRestorePending_=false,restoredBorder_=false;
    ESLError ApplyStyle(const wchar_t *);
};

// No GPU substitute: exercises real font rasterization, markup, and native
// AdvanceTime/Flush/Clear. Called by the Android diagnostic runner.
bool CheckLegacyMessage();
