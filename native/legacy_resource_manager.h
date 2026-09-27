#pragma once
#include <sakuraglx/sprite/sglx_resource_manager.h>
#include <memory>

class ECSResourceManager : public ECSGlobal {
public:
    DECLARE_CLASS_INFO(ECSResourceManager, ECSGlobal)
    ECSResourceManager();
    const wchar_t* GetTypeName() const override;
    ECSObject* GetTypeOf(const wchar_t*) override;
    ECSObject* Duplicate() override;
    ESLError GetFunction(ECSContext&, int&, const wchar_t*) override;
    ESLError CallFunction(ECSContext&, int, ECSObjArray<ECSObject>&) override;
    ESLError Save(ESLFileObject&, ECSContext&) override;
    ESLError Load(ESLFileObject&, ECSContext&) override;
    ESLError ReadSkinFile(ESLFileObject&);
    ESLError LoadSkinFile(const wchar_t*, ECSContext&);
    void Release();
    std::shared_ptr<SakuraGL::SGLSkinManager> GetSkin() const { return skin_; }
private:
    std::shared_ptr<SakuraGL::SGLSkinManager> skin_;
    EWideString filename_;
};
