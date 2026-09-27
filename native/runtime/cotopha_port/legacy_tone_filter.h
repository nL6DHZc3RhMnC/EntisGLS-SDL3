#pragma once
#include <sakuraglx/sprite/sglx_sprite.h>
#include <array>
#include <memory>

struct LegacyToneState {
    uint32_t flags=0;
    std::array<std::array<uint8_t,256>,4> bgra{};
    uint64_t revision=0;
    LegacyToneState();
};
bool HasLegacyToneEffect(const LegacyToneState&);
ESLError ApplyLegacyToneImage(SakuraGL::SGLImageObject&,const LegacyToneState&,bool sourceHasAlpha=true);

class ECSToneFilter : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSToneFilter,ECSObject)
    ECSToneFilter();
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError Move(ECSContext&,ECSObject*) override;
    ESLError UnaryOperate(ECSContext&,CSUnaryOperatorType) override;
    ESLError Operate(ECSContext&,CSOperatorType,ECSObject*) override;
    ESLError Compare(ECSContext&,int&,CSCompareType,ECSObject&) override;
    ESLError GetFunction(ECSContext&,int&,const wchar_t*) override;
    ESLError CallFunction(ECSContext&,int,ECSObjArray<ECSObject>&) override;
    ESLError Save(ESLFileObject&,ECSContext&) override;
    ESLError Load(ESLFileObject&,ECSContext&) override;
    ESLError LoadFilterFile(const wchar_t*,ECSContext&);
    ESLError ReadFilterFile(ESLFileObject&);
    ESLError SetGeneralTone(const std::array<int,8>& values,uint32_t flags=0);
    ESLError MorphingFilter(const ECSToneFilter&,const ECSToneFilter&,uint32_t degree);
    std::shared_ptr<LegacyToneState> ToneState() const {return state_;}
private:
    std::shared_ptr<LegacyToneState> state_;
    void Commit(const LegacyToneState&);
};
bool CheckLegacyTone();
