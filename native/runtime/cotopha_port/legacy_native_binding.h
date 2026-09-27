#pragma once

// Metadata is immutable after initialization. Resolve against the actual
// receiver into a local pointer, so derived native types and script threads
// cannot overwrite one another's dispatch index.
ESLError LegacyResolveNativeMethod(ECSContext& context, ECSObject* receiver,
    const ECSClassInfo::MemberFunction& method, ECS_FUNCTION_POINTER& result);
