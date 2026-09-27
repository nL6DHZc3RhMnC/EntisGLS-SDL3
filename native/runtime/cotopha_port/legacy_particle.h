#pragma once
#include "runtime/cotopha_port/legacy_sprite.h"
#include <memory>

class ECSParticleSprite : public ECSSprite {
public:
    DECLARE_CLASS_INFO(ECSParticleSprite, ECSSprite)
    ECSParticleSprite();
    ~ECSParticleSprite() override;
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
    ECSObject* GetVariableAt(int) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext&) override;
    ESLError CommitAllReference(ECSContext&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
private:
    class ParticleNative;
    struct Impl;
    std::unique_ptr<Impl> impl_;
    void AdvanceParticles(uint32_t);
    void DrawParticles();
    friend bool CheckLegacyParticleState(ECSEnvironment&);
};
bool CheckLegacyParticleState(ECSEnvironment&);
