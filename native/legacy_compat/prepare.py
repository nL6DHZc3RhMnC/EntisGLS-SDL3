#!/usr/bin/env python3
"""Make reproducible UTF-8/Clang copies of the locally supplied GLS3 sources.

The SDK and extracted originals are never modified.  This is a source port,
not a binary compatibility layer for Windows plug-ins.
"""
from pathlib import Path
import argparse
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from patch_legacy_serialization import patch_source
from patch_legacy_string_memory import patch_source as patch_string_memory
from patch_legacy_thread_context import patch_source as patch_thread_context
from patch_legacy_native_binding import patch_source as patch_native_binding
from patch_legacy_compiler import patch_source as patch_compiler
from patch_legacy_array_commit import patch_source as patch_array_commit
from patch_legacy_stack_state import patch_source as patch_stack_state
from patch_legacy_hash_state import patch_source as patch_hash_state
from patch_legacy_heap_state import patch_source as patch_heap_state
from patch_legacy_primary_context import patch_source as patch_primary_context
from patch_legacy_reference_commit import patch_source as patch_reference_commit
from patch_legacy_context_failure import patch_source as patch_context_failure

p = argparse.ArgumentParser()
p.add_argument("--legacy", type=Path, required=True)
p.add_argument("--output", type=Path, required=True)
a = p.parse_args()

def write_if_changed(path, text):
    content = text.encode("utf-8")
    if not path.exists() or path.read_bytes() != content:
        path.write_bytes(content)

for component in ("ESL", "GLS3", "erisalib", "EGL"):
    for source in (a.legacy / component).rglob("*"):
        if source.suffix not in (".h", ".cpp"):
            continue
        target = a.output / source.relative_to(a.legacy)
        target.parent.mkdir(parents=True, exist_ok=True)
        text = source.read_bytes().decode("cp932", errors="replace").replace("\r\n", "\n")
        # Calling a non-static method through nullptr is undefined even if its
        # body checks `this`. Inlining from app TUs can discard that check and
        # override a weak definition compiled with defensive optimizer flags.
        if component == "GLS3":
            text = re.sub(r'(\(\((?:const\s+)?ECSReference\s*\*\)\s*\w+\)->m_pRef)->IsValidObject\(\s*\)',
                          r'ECSObject::IsValidObject(\1)', text)
            text = re.sub(r'\b([A-Za-z_]\w*(?:(?:->|\.)[A-Za-z_]\w*)*)->IsValidObject\(\s*\)',
                          r'ECSObject::IsValidObject(\1)', text)
            if "->IsValidObject(" in text:
                raise ValueError(f"Unconverted unsafe validity call in {source}")
        if source.name == "glscsobj_object.h":
            text = text.replace("bool IsValidObject( void ) const", '''static bool IsValidObject(const ECSObject *object)
    {
        if (object == nullptr) return false;
        #if defined(_DEBUG)
        return object->IsValidObjectType();
        #else
        return true;
        #endif
    }
    bool IsValidObject( void ) const''')
        # The original guarded plugin calls with Windows SEH. Native Android
        # uses C++ exception guards; access violations remain process faults.
        text = text.replace("__try", "try")
        text = re.sub(r'__except\s*\(\s*EXCEPTION_EXECUTE_HANDLER\s*\)', "catch (...) ", text)
        # The common SDK already owns these memory APIs, with libc-compatible
        # signatures; the GLS3 declarations would conflict after macro expansion.
        if source.name == "eslheap.h":
            text = re.sub(r'extern\s+"C"\s*\{\s*GLSEXPORT void eslFillMemory.*?\}\s*;', "", text, flags=re.S)
        if source.name == "esl.h":
            # Do not truncate pointer-valued legacy error messages on AArch64.
            text = text.replace("enum\tESLError\n", "enum ESLError : intptr_t\n")
            # Thread/window glue is a separate port; containers do not need it.
            text = text.replace("#include\t<eslthread.h>", "")
        if source.name == "eslstring.h":
            # ReplaceWords uses an undeclared _Obj as its map key type.
            text = text.replace("ETaggedElement<_Obj,_CObj>", "ETaggedElement<EString,_CObj>", 1)
            text = text.replace("ETaggedElement<_Obj,_CObj>", "ETaggedElement<EWideString,_CObj>", 1)
        if source.name == "glsscript.h":
            text = text.replace("#include <ctscriptplugin.h>", "#include <ctscriptplugin.h>\n#include <legacy_file_pi.h>")
            # Keep the VM + object declarations, without pulling in the Windows
            # environment, compiler/editor, UI, multimedia and registry layer.
            for header in ("glscs_environment.h", "glscs_execution_image_linker.h",
                           "glscs_execution_image_compiler.h", "glscs_execution_reverse_assembler.h",
                           "glscs_execution_optimizer.h", "glscs_assembler.h",
                           "glscs_compiler.h", "glscs_compiler_c_style.h"):
                text = text.replace(f"#include <{header}>", "")
        if source.name == "glscs_environment.h":
            start = text.index("class\tECSFilePIInterface")
            end = text.index("class\tECSEnvironment", start)
            write_if_changed(a.output / "GLS3/Include/legacy_file_pi.h", "#pragma once\n" + text[start:end])
        if source.name == "erisafile.h":
            start = text.index("class\tEMCFile")
            end = text.index(" #if\t!defined(COMPACT_NOA_DECODER)", start)
            write_if_changed(a.output / "GLS3/Include/legacy_emc.h", "#pragma once\n" + text[start:end])
        if source.name == "erisafile.cpp":
            start = text.index("char\tEMCFile::")
            end = text.index("#if\t!defined(COMPACT_NOA_DECODER)", start)
            write_if_changed(a.output / "GLS3/Source/legacy_emc.cpp", "#include <gls.h>\n" + text[start:end])
        if source.name == "glscs_script_core.cpp":
            start = text.index("IMPLEMENT_CLASS_INFO( ECSStrTagArray")
            extracted = "#include <gls.h>\nstatic HESLHEAP g_hHeapCotopha = nullptr;\n" + text[start:]
            write_if_changed(a.output / "GLS3/Source/legacy_script_types.cpp", patch_source("legacy_script_types.cpp", extracted))
        if source.name == "glscs_compiler.cpp":
            start = text.index("void ECSCompiler::SYMBOL_NAMESPACE::ParseSymbol")
            end = text.index("bool ECSCompiler::IsCStyleBasicIntegerType", start)
            write_if_changed(a.output / "GLS3/Source/legacy_symbol_namespace.cpp", "#include <gls.h>\n" + text[start:end])
        if source.name == "ctscriptplugin.h":
            # Core objects retain opaque pointers to optional graphics plugin
            # interfaces. Their concrete Win32 declarations are not needed.
            start = text.index("struct\tECS_RESOURCE_INTERFACE\n{")
            end = text.index("struct\tECS_FILE_INTERFACE\n{", start)
            text = text[:start] + text[end:]
        if source.name == "glsscriptobj.h":
            start = text.index("class\tECSObjArray")
            position = text.index("{", start) + 1
            declarations = "\npublic:\n" + "".join(f" using SSystem::SObjectArray<T>::{name};\n" for name in ("m_nLength", "SetLength", "Remove", "Detach", "GetLength", "GetAt", "Merge"))
            text = text[:position] + declarations + text[position:]
        if source.name == "glscs_execution_image.h":
            # Environment is intentionally opaque here, so move the conversion
            # into the environment adapter's translation unit.
            text = re.sub(r'void AttachCSEnvironment\( ECSEnvironment \* pEnv \)\s*\{.*?\n\s*\}',
                          "void AttachCSEnvironment( ECSEnvironment * pEnv );", text, count=1, flags=re.S)
        if source.name == "glscs_context.h":
            start = text.index("class ETemporaryStack")
            position = text.index("{", start) + 1
            text = text[:position] + "\npublic:\n using EObjArray<T>::Pop;\n using EObjArray<T>::Push;\n using EObjArray<T>::GetSize;\n" + text[position:]
        if source.name == "glscs_context.cpp":
            text = text.replace("#include <gls.h>", '#include <gls.h>\n#include "runtime_support.h"')
            text = re.sub(r'ERawFile \*\s*pfile = new ERawFile ;\s*if \( pfile->Open\( strFileName, nOpenFlags \) \)\s*\{\s*delete\s+pfile ;\s*return\s+NULL ;\s*\}\s*return\s+pfile ;',
                          'auto *file = SSystem::SFileOpener::DefaultNewOpenFile(pwszFileName, nOpenFlags);\n return file ? new LegacyFileAdapter(file, nOpenFlags) : nullptr;', text, count=1)
            start = text.index("const char * __stdcall ECSFilePIInterface::PIC_GetFilePath")
            text = text[:start] + '''const char * __stdcall ECSFilePIInterface::PIC_GetFilePath(ECS_FILE *) {
    // Android streams may be NOA entries or content URIs, with no native path.
    return nullptr;
}
'''
            for cls in ("InputFilter", "MessageSprite", "ModelJoint", "MovieSprite",
                        "ParticleModel", "ParticleSprite", "PolygonModel", "RenderSprite",
                        "ResourceManager", "Setup", "SuperSprite", "ToneFilter", "Window"):
                text = re.sub(r'return\s+new ECS' + cls + r'\s*;', "return LegacyCreatePlatformObject(*this, pwszType);", text)
            start = text.index("static ESLError call_api")
            end = text.index("ESLError ECSContext::CallGlobalFunction", start)
            text = text[:start] + '''static ESLError call_api(long int &, FARPROC, EStreamBuffer &) {
    return ESLErrorMsg("Windows x86 native-call ABI is unavailable in the Android runtime");
}
\n''' + text[end:]
            text = text.replace("nValue = ERealFontImage::IsEnabledFontSmoothing() ? -1 : 0 ;", "return ESLErrorMsg(\"Legacy font smoothing query is not connected to the Android renderer\");")
            text = text.replace("ERealFontImage::EnableFontSmoothing( nFlags != 0 ) ;", "return ESLErrorMsg(\"Legacy font smoothing control is not connected to the Android renderer\");")
            text = text.replace("ESLThread::GetLogicalProcessorCount()", "sysconf(_SC_NPROCESSORS_ONLN)")
            text = text.replace("::glsGetEnabledProcessorType( )", "LegacyProcessorFeatures()")
            text = text.replace("::glsEnableProcessorType( 0 ) ;", "return ESLErrorMsg(\"Windows processor feature control is unavailable on ARM64\");")
            text = re.sub(r'::glsEnableProcessorType\( nFlags \) ;|::glsDisableProcessorType\( -1 \) ;', "", text)
            text = text.replace("wsprintf(", "sprintf(")
            text = text.replace("ESLAssert( sizeof(m_status) == sizeof(long) ) ;", "static_assert(sizeof(m_status) == sizeof(int), \"execution status width\");")
            text = text.replace("::InterlockedExchange( (LPLONG) &m_status, status ) ;", "__atomic_exchange_n(reinterpret_cast<int *>(&m_status), static_cast<int>(status), __ATOMIC_SEQ_CST);")
            # 0xff on disk denotes csotNop (-1), not an out-of-range enum value.
            text = text.replace("CSOperatorType\tcsotType = (CSOperatorType) m_pcsxi->m_pImage[m_ip ++] ;", "const BYTE rawOperator = m_pcsxi->m_pImage[m_ip++];\n CSOperatorType csotType = rawOperator == 0xff ? csotNop : static_cast<CSOperatorType>(rawOperator);")
            text = text.replace("csotType == (BYTE) csotNop", "csotType == csotNop")
            text = text.replace("return\t*(CreateUserClassObject( *pClassInfo )) ;", "auto *object = CreateUserClassObject(*pClassInfo);\n return object ? object->GetInstanceObject() : nullptr;")
            text = text.replace("return\t*(CreateUserClassObject( clsinf )) ;", "auto *object = CreateUserClassObject(clsinf);\n return object ? object->GetInstanceObject() : nullptr;")
            for method in ("PIC_GetWaveOutputDevice", "PIC_GetDrawImageObject"):
                pattern = r'(ESLObject\s*\*\s*__stdcall ECSContext::' + method + r'\s*\([^)]*\))\s*\{.*?\n\}'
                text = re.sub(pattern, r'\1\n{\n OutputDebugString("Legacy multimedia plugin interface is unavailable");\n return nullptr;\n}', text, count=1, flags=re.S)
        if source.name == "glscs_execution_image.cpp":
            text = text.replace("return\tm_pEnv->FindPluginedFunction( pszFuncName ) ;", "return\treinterpret_cast<void *>(m_pEnv->FindPluginedFunction( pszFuncName )) ;")
            start = text.index("void ECSExecutionImage::CompileToNativeCode")
            end = text.index("DWORD * ECSExecutionImage::GetFunctionAddress", start)
            text = text[:start] + '''void ECSExecutionImage::CompileToNativeCode(bool) {
    // This GLS3 compiler emits Windows x86 machine code. The portable
    // interpreter remains active; never run those bytes on AArch64.
    OutputDebugString("GLS3 x86 JIT disabled; using the portable interpreter");
}
\n''' + text[end:]
        if source.name == "glscs_rosetta.cpp":
            text = text.replace("#include <glscs_rosetta.h>", "#include <rosetta/rosetta.h>\n#include <glscs_rosetta.h>")
            text = text.replace("return\t!m_pObject->OperateInteger( number ) ;", "INT64 value = 0;\n const bool success = !m_pObject->OperateInteger(value);\n number = value;\n return success;")
            text = text.replace("(int) pInstance", "static_cast<int>(reinterpret_cast<intptr_t>(pInstance))")
        if source.name == "glscsobj_integer.cpp":
            text = re.sub(r'case\s+(0x[8-9A-Fa-f][0-9A-Fa-f]{15})\s*:', r'case static_cast<INT64>(\1ULL):', text)
        if source.name == "glscsobj_string.cpp":
            text = text.replace("swsUsage = wstrOld ;", "swsUsage( wstrOld.CharPtr() ) ;")
            start = text.index("ESLError ECSString::Call_Calculate")
            end = text.index("ESLError ECSString::Call_Execute", start)
            text = text[:start] + '''ESLError ECSString::Call_Calculate(ECSContext &, ECSObjArray<ECSObject> &) {
    return ESLErrorMsg("String.Calculate requires the legacy compiler, which is not ported yet");
}
\n''' + text[end:]
        if source.name == "glscsobj_hash.cpp":
            text = text.replace("return\t((ECSInteger*)pObj)->GetInt() ;", "return\t(int) ((ECSInteger*)pObj)->GetInt() ;")
        if source.name == "glscs_classinf.cpp":
            # The nested local hid the parent cast before reading its offsets.
            text = text.replace("ECS_CAST_INTERFACE\tciParent\n\t\t\t\t( NULL, ciParent.iVarOffset", "ECS_CAST_INTERFACE\tciChild\n\t\t\t\t( NULL, ciParent.iVarOffset")
            text = text.replace("ciParent, __max( pClass->dwFlags, dwScopeFlags )", "ciChild, __max( pClass->dwFlags, dwScopeFlags )")
        # MSVC permits lookup in dependent bases.  Clang needs explicit lookup.
        if source.name == "eslarray.h":
            text = re.sub(r'(class\s+EObjArray\s*:\s*public\s+EPtrObjArray<([^>]+)>\s*\{)',
                          r'\1\npublic:\n using EPtrObjArray<\2>::m_nLength;\n using EPtrObjArray<\2>::m_ptrArray;\n using EPtrObjArray<\2>::GetSize;\n using EPtrObjArray<\2>::GetAt;\n using EPtrObjArray<\2>::SetAt;\n using EPtrObjArray<\2>::SetSize;\n using EPtrObjArray<\2>::InsertAt;\n using EPtrObjArray<\2>::Add;\n', text)
            for element in ("ETaggedElement", "ETaggedPtrElement"):
                base = f"EObjArray< {element}<TagType,ObjType> >"
                match = re.search(r'(:\s*public\s+EObjArray<\s*' + element + r'<TagType,ObjType>\s*>\s*\{)', text)
                if match:
                    declarations = "\npublic:\n" + "".join(f" using {base}::{name};\n" for name in ("GetSize", "GetAt", "InsertAt", "RemoveAt", "RemoveAll"))
                    text = text[:match.end()] + declarations + text[match.end():]
        text = patch_source(str(source.relative_to(a.legacy)), text)
        text = patch_string_memory(str(source.relative_to(a.legacy)), text)
        text = patch_thread_context(str(source.relative_to(a.legacy)), text)
        text = patch_native_binding(str(source.relative_to(a.legacy)), text)
        text = patch_compiler(str(source.relative_to(a.legacy)), text)
        text = patch_array_commit(str(source.relative_to(a.legacy)), text)
        text = patch_stack_state(str(source.relative_to(a.legacy)), text)
        text = patch_hash_state(str(source.relative_to(a.legacy)), text)
        text = patch_heap_state(str(source.relative_to(a.legacy)), text)
        text = patch_primary_context(str(source.relative_to(a.legacy)), text)
        text = patch_reference_commit(str(source.relative_to(a.legacy)), text)
        text = patch_context_failure(str(source.relative_to(a.legacy)), text)
        write_if_changed(target, text)
