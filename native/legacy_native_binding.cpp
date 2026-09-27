#include "legacy_compat/gls.h"
#include "legacy_native_binding.h"
#include "platform/log.h"

ESLError LegacyResolveNativeMethod(ECSContext& context, ECSObject* receiver,
    const ECSClassInfo::MemberFunction& method, ECS_FUNCTION_POINTER& result) {
    receiver = ECSObject::GetEntity(receiver);
    if (!receiver) return ESLErrorMsg("Native member has no receiver");
    int index = -1;
    ESLError error = receiver->GetFunction(context, index, method.GetName());
    if (error || index < 0) {
        study::platform::LogPrint(study::platform::LogPriority::Error, "LegacyCotopha",
            "Unimplemented native member: %ls::%ls (receiver %ls)",
            method.m_wstrClass.CharPtr(), method.GetName().CharPtr(), receiver->GetTypeName());
        return error ? error : ESLErrorMsg("Native member resolution returned an invalid index");
    }
    result = method.m_fpFuncPointer;
    result.m_ftType = ECS_FUNCTION_POINTER::funcIndexCall;
    result.m_castThis = ECS_CAST_INTERFACE(receiver);
    result.m_varFunc.nIndex = index;
    return eslErrSuccess;
}
