#include "compatibility/sdk/legacy/gls.h"
#include "legacy_window_probe.h"
#include "platform/log.h"
#include <memory>
#include <chrono>

namespace {
bool Invoke(ECSContext& context, ECSObject& object, const wchar_t* method,
    ECSObjArray<ECSObject>& args, INT64& status) {
    int index;
    ESLError error = object.GetFunction(context, index, method);
    if (!error) error = object.CallFunction(context, index, args);
    if (error) {
        study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Window probe %ls: %s", method, GetESLErrorMsg(error));
        return false;
    }
    std::unique_ptr<ECSObject> result(context.PopObject());
    return result && !result->OperateInteger(status);
}
bool Simple(ECSContext& context, ECSObject& object, const wchar_t* method) {
    ECSObjArray<ECSObject> args; args.Add(new ECSReference(&object));
    INT64 status;
    return Invoke(context, object, method, args, status) && status == 0;
}
}

bool CheckLegacyWindow(ECSContext& context) {
    ECSWindow window;
    if (!Simple(context, window, L"CreateDisplay")) return false;
    ECSInputFilter input;
    if (input.LoadInputFilter(L"input.xml", &context) || input.OpenFilter(&window, 0, &context)) return false;
    if (!Simple(context, window, L"EnableCommandQueue") || !Simple(context, window, L"FlushCommandQueue")) return false;
    ECSResourceManager manager;
    if (manager.LoadSkinFile(L"wm_langpicker.noa", context)) return false;
    ECSSprite page;
    if (page.BuildFormPage(manager.GetSkin(), L"ID_LANGPICKER_FRAME")) return false;
    window.NativeSprite().AddChild(&page.NativeSprite());
    std::unique_ptr<ECSStructureInterface> command(context.CreateUserStructure(L"WndSpriteCmd"));
    if (!command) return false;
    auto* commandObject = ESLTypeCast<ECSStructure>(command->GetInstanceObject());
    if (!commandObject) return false;
    context.SetStatus(ECSContext::xsExecution);
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Window probe READY: actual language picker rendered; waiting for UI button command");
    bool clicked = false;
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(45);
    while (std::chrono::steady_clock::now() < end && context.GetStatus() == ECSContext::xsExecution) {
        ECSObjArray<ECSObject> args;
        args.Add(new ECSReference(&window));
        args.Add(new ECSReference(command->GetInstanceObject()));
        args.Add(new ECSInteger(1000));
        INT64 status;
        if (!Invoke(context, window, L"GetCommand", args, status)) break;
        if (status == 0) {
            EWideString id = commandObject->GetMemberAsStr(L"strID", L"");
            study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Window probe command: %ls notification=%lld",
                id.CharPtr(), static_cast<long long>(command->GetMemberAsInt(L"nNotification", 0)));
            if (id == L"ID_LANGPICKER_JA" || id == L"ID_LANGPICKER_EN" || id == L"ID_LANGPICKER_ZHTW") { clicked = true; break; }
        }
    }
    window.NativeSprite().DetachChild(&page.NativeSprite());
    input.CloseFilter();
    window.CloseDisplay();
    context.SetStatus(ECSContext::xsHalt);
    study::platform::LogWrite(clicked ? study::platform::LogPriority::Info : study::platform::LogPriority::Error, "StudySteady",
        clicked ? "Window probe PASS: native display, real skin, input -> button -> legacy command" : "Window probe ended without a language button command");
    return clicked;
}
