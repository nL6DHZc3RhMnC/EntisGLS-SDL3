#include "legacy_compat/gls.h"
#include "legacy_resource_manager.h"
#include "../tools/legacy_serialization.h"
#include "platform/log.h"
#include <chrono>
#include <cwchar>

IMPLEMENT_CLASS_INFO(ECSResourceManager, ECSGlobal)
ECSResourceManager::ECSResourceManager() { m_vtType = csvtObject; }
const wchar_t* ECSResourceManager::GetTypeName() const { return L"ResourceManager"; }
ECSObject* ECSResourceManager::GetTypeOf(const wchar_t* name) {
    return name && !std::wcscmp(name, L"ResourceManager") ? this : ECSGlobal::GetTypeOf(name);
}
ECSObject* ECSResourceManager::Duplicate() { return new ECSResourceManager; }
void ECSResourceManager::Release() {
    RemoveAllVariable();
    skin_.reset();
    filename_ = L"";
}
ESLError ECSResourceManager::ReadSkinFile(ESLFileObject& file) {
    const auto started = std::chrono::steady_clock::now();
    Release();
    auto* copy = file.Duplicate();
    if (!copy) return eslErrGeneral;
    SESLFileInterface stream(copy, true);
    auto skin = std::make_shared<SakuraGL::SGLSkinManager>();
    // The legacy manager eagerly exposed every loaded resource as a variable.
    if (skin->ReadSkinFile(stream, true)) return eslErrGeneral;
    const auto& resources = skin->GetResourceArray();
    for (size_t i = 0; i < resources.GetLength(); ++i) {
        std::unique_ptr<ECSResource> resource(new ECSResource);
        if (resource->AttachNativeResource(resources.GetAt(i), skin)) {
            study::platform::LogPrint(study::platform::LogPriority::Error, "LegacyCotopha", "Unsupported skin resource: %ls", static_cast<const wchar_t*>(*resources.GetTagAt(i)));
            Release();
            return eslErrNotSupported;
        }
        AddVariable(*resources.GetTagAt(i), resource.release());
    }
    skin_ = std::move(skin);
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady",
        "Legacy skin loaded: resources=%zu forms=%zu decode_attach_ms=%.1f",
        resources.GetLength(), skin_->GetFormDefinitions().GetLength(),
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count());
    return eslErrSuccess;
}
ESLError ECSResourceManager::LoadSkinFile(const wchar_t* path, ECSContext& context) {
    std::unique_ptr<ESLFileObject> file(context.OpenFileOnScript(path));
    if (!file) return eslErrGeneral;
    ESLError error = ReadSkinFile(*file);
    if (!error) filename_ = path;
    return error;
}
ESLError ECSResourceManager::GetFunction(ECSContext&, int& index, const wchar_t* name) {
    static const wchar_t* names[] = {L"LoadResource", L"DeleteContents", L"CreateFormPage"};
    if (name) for (int i = 0; i < 3; ++i) if (!std::wcscmp(name, names[i])) {
        index = i; return eslErrSuccess;
    }
    return ESLErrorMsg("Unknown ResourceManager method");
}
ESLError ECSResourceManager::CallFunction(ECSContext& context, int index, ECSObjArray<ECSObject>& args) {
    ESLError error = context.VerifyArgumentCount(args, index == 0 ? 2 : index == 1 ? 1 : 3);
    if (error) return error;
    if (index == 0) {
        auto* file = ESLTypeCast<ECSFile>(context.GetArgumentObjectAs(args, 1, L"File"));
        if (file && file->GetFileInterface()) error = ReadSkinFile(*file->GetFileInterface());
        else {
            ECSWideString path;
            if ((error = context.GetArgumentAsStr(path, args, 1, nullptr))) return error;
            error = LoadSkinFile(path, context);
        }
    } else if (index == 1) Release();
    else if (index == 2) {
        auto* sprite = ESLTypeCast<ECSSprite>(context.GetArgumentObjectAs(args, 1, L"Sprite"));
        ECSWideString page;
        if (!sprite) return ESLErrorMsg("CreateFormPage requires a Sprite");
        if ((error = context.GetArgumentAsStr(page, args, 2, nullptr))) return error;
        error = sprite->BuildFormPage(skin_, page);
        if (!error) sprite->RecordLegacyFormSource(this, page, context);
    } else return ESLErrorMsg("Invalid ResourceManager method index");
    return context.PushObject(context.new_CSInteger(error));
}
ESLError ECSResourceManager::Save(ESLFileObject& file, ECSContext&) {
    if (skin_ && filename_.IsEmpty())
        return ESLErrorMsg("Cannot save a skin loaded from a stream without a reopenable source");
    return StudySteadyLegacyWire::WriteWideString(file, filename_) ? eslErrSuccess : eslErrGeneral;
}
ESLError ECSResourceManager::Load(ESLFileObject& file, ECSContext& context) {
    EWideString path;
    if (!StudySteadyLegacyWire::ReadWideString(file, path)) return eslErrGeneral;
    if (path.IsEmpty()) { Release(); return eslErrSuccess; }
    return LoadSkinFile(path, context);
}
