#include "compatibility/sdk/legacy/gls.h"
#include "legacy_native_binding_probe.h"
#include "platform/log.h"
#include <memory>
#include <cstring>

namespace {
class BindingImage : public ECSExecutionImage {
public:
    void SetNativeCall() {
        BYTE code[13] = {static_cast<BYTE>(csicCallNativeMember)};
        const DWORD operands[] = {2, 0, 0};
        std::memcpy(code + 1, operands, sizeof(operands));
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)), code, sizeof(code));
        m_bufImage.Flush(sizeof(code));
        m_pImage = static_cast<BYTE*>(m_bufImage.ModifyBuffer(0, sizeof(code)));
        m_dwImageSize = sizeof(code);
    }
};
ECSClassInfo* AddClass(ECSExecutionImage& image, const wchar_t* name, bool native) {
    auto* cls = new ECSClassInfo;
    cls->SetName(name);
    cls->SetGlobalName(name);
    cls->SetAttribute(native ? ECSTypeInfo::flagNativeObject : 0);
    image.AddClassInfo(cls);
    cls->AddCastClassInfo(name, new ECSClassInfo::CastInfo(ECS_CAST_INTERFACE(nullptr), 0, cls));
    return cls;
}
ECSClassInfo::MemberFunction* AddMethod(ECSClassInfo& cls, ECSClassInfo& declared,
    const wchar_t* name, bool native, unsigned parent = 0) {
    auto* method = new ECSClassInfo::MemberFunction;
    method->SetName(name);
    method->SetGlobalName(cls.GetGlobalName() + L"::" + name);
    method->SetAttribute(native ? ECSTypeInfo::flagNativeObject : 0);
    method->m_wstrClass = declared.GetGlobalName();
    method->m_pClassCast = declared.GetCastClassInfoAs(declared.GetGlobalName());
    method->m_fpFuncPointer.m_ftType = ECS_FUNCTION_POINTER::funcScriptCall;
    method->m_fpFuncPointer.m_varFunc.addrScript = native ? 0 : 123;
    method->m_fpFuncPointer.m_castThis = ECS_CAST_INTERFACE(nullptr);
    method->m_fpFuncPointer.m_castThis.iNativeParent = parent;
    cls.AddFunction(method);
    return method;
}
bool Check(bool ok, const char* detail) {
    if (!ok) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Native binding FAIL: %s", detail);
    return ok;
}
}

bool CheckLegacyNativeBinding(ECSContext& context) {
    BindingImage image;
    auto* native = AddClass(image, L"Resource", true);
    auto* nativeMethod = AddMethod(*native, *native, L"GetTotalVolume", true);
    auto* unsupported = AddClass(image, L"ProbeUnavailableNative", true);
    AddMethod(*unsupported, *unsupported, L"NeverCalled", true);
    auto* derived = AddClass(image, L"ProbeDerived", false);
    auto* inherited = AddMethod(*derived, *native, L"GetTotalVolume", true, 1);
    AddMethod(*derived, *native, L"ProbeMissingMethod", true, 1);
    AddMethod(*derived, *derived, L"ScriptMethod", false);
    if (!Check(!image.InitializeNativeClass(context), "unused unavailable declaration initializes")) return false;
    if (!Check(nativeMethod->m_fpFuncPointer.m_ftType == ECS_FUNCTION_POINTER::funcDeferredNativeCall &&
        inherited->m_fpFuncPointer.m_castThis.iNativeParent == 1, "explicit deferred state preserves parent slot")) return false;
    struct Restore {
        ECSContext& context; ECSExecutionImage* image; DWORD ip;
        ~Restore() { context.m_pcsxi = image; context.m_ip = ip; }
    } restore{context, context.m_pcsxi, context.m_ip};
    context.m_pcsxi = &image;
    if (!Check(context.CreateClassObject(*unsupported) == nullptr, "actual missing construction fails")) return false;
    ECSStructure object(derived);
    object.m_varArray.Add(new ECSInteger(999));
    auto* resource = new ECSResource;
    object.m_varArray.Add(resource);
    ECS_FUNCTION_POINTER byName, byIndex, script, missing;
    if (!Check(!object.GetFunctionPointer(context, byName, L"GetTotalVolume") &&
        !object.GetFunctionPointer(context, byIndex, 0) && byName.m_varFunc.nIndex == byIndex.m_varFunc.nIndex &&
        byName.m_castThis.pCastObject == resource && byName.m_ftType == ECS_FUNCTION_POINTER::funcIndexCall,
        "derived native function pointer resolves actual parent")) return false;
    if (!Check(object.GetFunctionPointer(context, missing, L"ProbeMissingMethod") != eslErrSuccess,
        "missing method fails explicitly")) return false;
    if (!Check(!object.GetFunctionPointer(context, script, L"ScriptMethod") &&
        script.m_ftType == ECS_FUNCTION_POINTER::funcScriptCall && script.m_varFunc.addrScript == 123 &&
        script.m_castThis.pCastObject == &object, "ordinary script pointer preserved")) return false;
    ECSObjArray<ECSObject> args;
    args.Add(context.new_CSReference(resource)); args.Add(context.new_CSInteger(0));
    if (!Check(!byName.m_castThis.pCastObject->CallFunction(context, byName.m_varFunc.nIndex, args),
        "stored resolved native pointer executes")) return false;
    ECSObject* result = context.PopObject();
    double volume = 0;
    bool resultOk = result && !result->OperateReal(volume) && volume == 1;
    context.delete_CSObject(result);
    if (!Check(resultOk, "stored pointer returns real volume")) return false;
    image.SetNativeCall();
    context.m_ip = 0;
    context.PushObject(context.new_CSReference(resource));
    context.PushObject(context.new_CSInteger(0));
    if (!Check(!context.ExecuteInstruction(), "native opcode resolves by method name")) return false;
    result = context.PopObject(); volume = 0;
    resultOk = result && !result->OperateReal(volume) && volume == 1;
    context.delete_CSObject(result);
    if (!Check(resultOk && nativeMethod->m_fpFuncPointer.m_ftType == ECS_FUNCTION_POINTER::funcDeferredNativeCall,
        "native opcode result without shared metadata mutation")) return false;
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Native binding PASS: unused declaration, strict construction, inherited/stored pointers, script pointer, native opcode");
    return true;
}
