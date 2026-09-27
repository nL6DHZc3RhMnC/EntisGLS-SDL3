#pragma once

#include "runtime/cotopha_port/legacy_resource.h"

// The historical public API spells speech "Speach". These objects expose the
// unavailable-backend state: they never claim to initialize or synthesize
// speech successfully. Recorded game audio continues through Resource.
class ECSSpeachVoiceGenerator final : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSSpeachVoiceGenerator, ECSObject)
    ECSSpeachVoiceGenerator();
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError Move(ECSContext&, ECSObject*) override;
    ESLError UnaryOperate(ECSContext&, CSUnaryOperatorType) override;
    ESLError Operate(ECSContext&, CSOperatorType, ECSObject*) override;
    ESLError Compare(ECSContext&, int&, CSCompareType, ECSObject&) override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
};

class ECSSpeachVoicePlayer final : public ECSResource {
public:
    DECLARE_CLASS_INFO(ECSSpeachVoicePlayer, ECSResource)
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
};
