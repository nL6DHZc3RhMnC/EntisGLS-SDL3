#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>

class ECSSetup : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSSetup,ECSObject)
    ECSSetup();
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &,ECSObject *) override;
    ESLError UnaryOperate(ECSContext &,CSUnaryOperatorType) override;
    ESLError Operate(ECSContext &,CSOperatorType,ECSObject *) override;
    ESLError Compare(ECSContext &,int &,CSCompareType,ECSObject &) override;
    ESLError GetFunction(ECSContext &,int &,const wchar_t *) override;
    ESLError CallFunction(ECSContext &,int,ECSObjArray<ECSObject> &) override;
    // The original Setup object serializes no payload; implemented operations
    // here are likewise stateless (installer jobs are explicitly unsupported).
    ESLError Save(ESLFileObject &,ECSContext &) override { return eslErrSuccess; }
    ESLError Load(ESLFileObject &,ECSContext &) override { return eslErrSuccess; }
    static const wchar_t *m_pwszFuncName[61];
    static EWideString MakeMD5Digest(const void *,size_t);
    static uint32_t CalcCRC32(const void *,size_t);
    static uint32_t CheckSum32(const void *,size_t);
    static int MessageBoxStyleToAndroid(int windowsStyle);
    static int MessageBoxResultToWindows(int androidResult);
private:
    static ESLError StreamArgument(ECSContext &,ECSObjArray<ECSObject> &,
        const std::function<void(const uint8_t *,size_t)> &);
};
