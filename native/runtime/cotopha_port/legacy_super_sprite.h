#pragma once
#include "runtime/cotopha_port/legacy_sprite.h"

class LegacySuperRaster;
class LegacySuperShadingState;

class ECSSuperSprite : public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSSuperSprite, ECSSprite)
    ECSSuperSprite();
    ~ECSSuperSprite() override;
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
    ESLError Release() override;
    ECSObject* GetVariableAt(int) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
private:
    class SuperNative;
    class ColorEffect;
    std::shared_ptr<ColorEffect> colorEffect_;
    std::shared_ptr<LegacySuperRaster> rasterEffect_;
    std::shared_ptr<LegacySuperShadingState> shadingEffect_;
    void ConnectEffectDegree(const std::array<uint32_t,33>&,uint32_t);
    void DrawColor(SakuraGL::S3DRenderContextInterface&);
    void AdvanceEffect(uint32_t milliseconds);
    void DrawEffect();
    void ClearEffect();
    ESLError ApplyEffect(const std::array<uint32_t,33>&, ECSResource*, ECSContext*);
    // Original Windows EFFECT_PARAM is 132 bytes. Store fixed-width words,
    // including unused fields, never an Android struct or a live pointer.
    std::array<uint32_t,33> effectWords_{};
    ECSReference effectImage_;
    bool restoreEffect_ = false;
    bool tile_ = false, dirty_ = false;
    uint32_t width_ = 0, height_ = 0, interval_ = 0;
    uint64_t elapsed_ = 0;
    int speedX_ = 0, speedY_ = 0, scrollX_ = 0, scrollY_ = 0;
    std::unique_ptr<SakuraGL::SGLImageObject> tileSource_;
    std::unique_ptr<SakuraGL::SGLImage> tileImage_;
    friend bool CheckLegacySuperSprite(ECSEnvironment&);
};
bool CheckLegacySuperSprite(ECSEnvironment&);
