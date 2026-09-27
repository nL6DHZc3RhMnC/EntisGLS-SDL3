#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_resource_manager.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include "platform/log.h"
#include <chrono>
#include <cwchar>
#include <utility>
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include "platform/sdl/game_file_metrics.h"
#if defined(__ANDROID__)
#include "platform/android/android_game_files.h"
#endif
#endif

namespace {
void FileDiagnostics(const char* phase, const wchar_t* path = L"") {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    using namespace study::platform::sdl;
    if (!GameFileMetricsEnabled()) return;
    const auto stats = SnapshotGameFileMetrics();
    for (const auto& item : {std::pair<const char*, GameFileOperationMetrics>{"duplicate", stats.duplicate},
            {"read", stats.read}, {"seek", stats.seek}, {"length", stats.getLength}})
        study::platform::LogPrint(study::platform::LogPriority::Info, "StartupIO",
            "phase=%s path=%ls operation=%s calls=%llu bytes=%llu total_ms=%.3f max_all_ms=%.3f",
            phase, path, item.first, static_cast<unsigned long long>(item.second.calls),
            static_cast<unsigned long long>(item.second.bytes), item.second.totalNs / 1e6, item.second.maxNs / 1e6);
#if defined(__ANDROID__)
    study::platform::LogPrint(study::platform::LogPriority::Info, "StartupIO",
        "phase=%s provider=%s", phase, AndroidGameFileDiagnostics().c_str());
#endif
#endif
}
}

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
    FileDiagnostics("skin_begin");
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
    FileDiagnostics("skin_end");
    return eslErrSuccess;
}
ESLError ECSResourceManager::LoadSkinFile(const wchar_t* path, ECSContext& context) {
    FileDiagnostics("skin_open_begin", path);
    std::unique_ptr<ESLFileObject> file(context.OpenFileOnScript(path));
    FileDiagnostics("skin_open_end", path);
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
