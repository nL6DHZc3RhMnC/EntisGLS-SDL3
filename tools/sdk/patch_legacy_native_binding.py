"""Resolve native metadata using real receivers, never temporary dummy objects."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re


def replace(text, pattern, replacement, count=1):
    text, found = re.subn(pattern, lambda _: replacement, text, flags=re.S)
    if found != count:
        raise ValueError(f'native binding patch expected {count}, found {found}: {pattern}')
    return text


def patch_source(relative_path, text):
    name = Path(relative_path).name
    if name == 'glscsobj_object.h':
        text = replace(text, r'funcNakedCall,',
                       'funcNakedCall,\n        funcDeferredNativeCall, // runtime metadata only; never a callable pointer')
    elif name == 'glscs_execution_image.cpp':
        text = replace(text,
            r'ESLError ECSExecutionImage::InitializeNativeClass\( ECSContext & context \)\s*\{.*?\n\}',
            '''ESLError ECSExecutionImage::InitializeNativeClass(ECSContext& context)
{
    for (int i = 0; i < GetClassInfoCount(); ++i) {
        ECSClassInfo* cls = GetClassInfoAt(i);
        if (!cls || cls->IsNakedMemoryClass()) continue;
        for (unsigned j = 0; j < cls->GetFunctionCount(); ++j) {
            ECSClassInfo::MemberFunction* method = cls->GetFunctionAt(j);
            if (!method) return ESLErrorMsg("Missing member metadata");
            if (!(method->GetAttribute() & ECSTypeInfo::flagNativeObject)) continue;
            if ((cls->GetAttribute() & ECSTypeInfo::flagNativeObject) &&
                (method->GetAttribute() & ECSTypeInfo::flagNakedCall)) continue;
            method->m_fpFuncPointer.m_ftType = ECS_FUNCTION_POINTER::funcDeferredNativeCall;
            method->m_fpFuncPointer.m_varFunc.addrScript = 0;
            // Preserve the native-parent slot and inherited offsets for script
            // structures; standalone native receivers have no parent slot.
            if (cls->GetAttribute() & ECSTypeInfo::flagNativeObject)
                method->m_fpFuncPointer.m_castThis = ECS_CAST_INTERFACE(nullptr);
        }
    }
    return eslErrSuccess;
}''')
        # A saved CSX must retain the original portable declaration, not our
        # transient enum value or a process-specific method index.
        text = replace(text,
            r'if \( !StudySteadyLegacyWire::WriteFunction\(file, pPrototype->m_fpFuncPointer\) \)',
            '''ECS_FUNCTION_POINTER portablePointer = pPrototype->m_fpFuncPointer;
        if (portablePointer.m_ftType == ECS_FUNCTION_POINTER::funcDeferredNativeCall) {
            portablePointer.m_ftType = ECS_FUNCTION_POINTER::funcScriptCall;
            portablePointer.m_varFunc.addrScript = 0;
        }
        if ( !StudySteadyLegacyWire::WriteFunction(file, portablePointer) )''')
    elif name == 'glscs_context.cpp':
        text = replace(text,
            r'ESLTrace\( "cotopha exception : %s #\%08X\\n", GetESLErrorMsg\(err\), m_ip \) ;',
            'LegacyLogScriptError(GetESLErrorMsg(err), m_ip);')
        text = replace(text,
            r'else if \( pClassInfo->GetAttribute\(\) & ECSTypeInfo::flagNativeObject \)\s*\{\s*//\s*return\s+CreateObject\( csvtObject, clsinf.GetGlobalName\(\), NULL \) ;\s*\}',
            '''else if (pClassInfo->GetAttribute() & ECSTypeInfo::flagNativeObject) {
            return LegacyCreatePlatformObject(*this, pwszType);
        }''')
        text = replace(text,
            r'if \( pFunc->m_fpFuncPointer.m_ftType\s*== ECS_FUNCTION_POINTER::funcIndexCall \)',
            '''ECS_FUNCTION_POINTER resolved;
    if (pFunc->m_fpFuncPointer.m_ftType == ECS_FUNCTION_POINTER::funcDeferredNativeCall) {
        ESLError error = LegacyResolveNativeMethod(*this, pEntity, *pFunc, resolved);
        if (error) return error;
        return resolved.m_castThis.pCastObject->CallFunction(
            *this, resolved.m_varFunc.nIndex, m_arg.m_varArray);
    }
    if ( pFunc->m_fpFuncPointer.m_ftType == ECS_FUNCTION_POINTER::funcIndexCall )''')
    elif name == 'glscsobj_structure.cpp':
        text = replace(text,
            r'fptr = pFunc->m_fpFuncPointer ;\s*fptr.m_castThis =\s*ECS_CAST_INTERFACE\s*\( m_varArray.GetAt\( fptr.m_castThis.iNativeParent \) \) ;',
            '''ECSObject* nativeReceiver = m_varArray.GetAt(pFunc->m_fpFuncPointer.m_castThis.iNativeParent);
            ESLError error = LegacyResolveNativeMethod(context, nativeReceiver, *pFunc, fptr);
            if (error) return error;''', count=2)
    return text
