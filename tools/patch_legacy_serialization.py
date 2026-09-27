#!/usr/bin/env python3
"""Mechanical, checked Win32 CSX wire-format fixes for generated GLS3 sources.

Call patch_source(relative_path, text) after decoding the original SDK to UTF-8.
The original sources and native in-memory ECS structures are not modified.
"""
from pathlib import Path
import re


def _replace(text, pattern, replacement, count=1):
    text, found = re.subn(pattern, lambda _: replacement, text, flags=re.S)
    if found != count:
        raise ValueError(f'legacy wire patch expected {count} matches, found {found}: {pattern}')
    return text


def _inject(text):
    helper = Path(__file__).with_name('legacy_serialization.h').read_text()
    return _replace(text, r'#include\s*<gls.h>', '#include <gls.h>\n' + helper)


def patch_source(relative_path: str, text: str) -> str:
    name = Path(relative_path).name
    if name == 'glscs_execution_image.cpp':
        text = _inject(text)
        for verb in ('Read', 'Write'):
            const = '' if verb == 'Read' else 'const '
            signature = (f'ESLError ECSExecutionImage::{verb}WideString\n'
                         f'\t( ESLFileObject & file, {const}ECSWideString & wstr )')
            body = ('\n{\n\treturn StudySteadyLegacyWire::' + verb +
                    'WideString(file, wstr) ? eslErrSuccess : eslErrGeneral;\n}')
            text = _replace(text,
                r'ESLError ECSExecutionImage::' + verb + r'WideString\s*\([^)]*\)\s*\{.*?\n\}',
                signature + body)
            text = _replace(text,
                r'file\.' + verb + r'\( pci, sizeof\(ECS_CAST_INTERFACE\) \)\s*< sizeof\(ECS_CAST_INTERFACE\)',
                '!StudySteadyLegacyWire::' + verb + 'Cast(file, *pci)')
            text = _replace(text,
                r'file\.' + verb + r'\s*\( &\(pPrototype->m_fpFuncPointer\),\s*sizeof\(ECS_FUNCTION_POINTER\) \)\s*< sizeof\(ECS_FUNCTION_POINTER\)',
                '!StudySteadyLegacyWire::' + verb + 'Function(file, pPrototype->m_fpFuncPointer)')
    elif name == 'glscs_context.cpp':
        text = _inject(text)
        text = _replace(text,
            r'ESLError ECSContext::SaveProcessorContext\( ESLFileObject & file \)\s*\{.*?\n\}',
            '''ESLError ECSContext::SaveProcessorContext(ESLFileObject& file)
{
    // Windows record: 3 u32, time u64, 2 virtual-address u64, 256 raw u64
    // registers. No host pointer, alignment padding or native struct is stored.
    uint8_t wire[36 + 256 * 8]{};
    using namespace StudySteadyLegacyWire;
    Write32(wire, m_ip); Write32(wire + 4, m_ipSegment);
    Write32(wire + 8, static_cast<uint32_t>(GetStatus()));
    Write64(wire + 12, m_nLastTickTime);
    Write64(wire + 20, static_cast<uint64_t>(m_vaStack));
    Write64(wire + 28, static_cast<uint64_t>(m_vaNakedStack));
    for (unsigned i = 0; i < 256; ++i) Write64(wire + 36 + i * 8, m_regset[i].ui);
    if (file.Write(wire, sizeof(wire)) != sizeof(wire)) return ESLErrorMsg("Processor state write failed");
    const auto error = SaveObject(file, m_pRetObj);
    if (error) return error;
    const uint8_t extension[4]{};
    return file.Write(extension, sizeof(extension)) == sizeof(extension)
        ? eslErrSuccess : ESLErrorMsg("Processor extension write failed");
}''')
        text = _replace(text,
            r'ESLError ECSContext::LoadProcessorContext\( ESLFileObject & file \)\s*\{.*?\n\}',
            '''ESLError ECSContext::LoadProcessorContext(ESLFileObject& file)
{
    uint8_t wire[36 + 256 * 8];
    using namespace StudySteadyLegacyWire;
    if (file.Read(wire, sizeof(wire)) != sizeof(wire)) return ESLErrorMsg("Truncated processor state");
    const auto status = Read32(wire + 8);
    if (status > static_cast<uint32_t>(xsInterrupt)) return ESLErrorMsg("Invalid saved execution status");
    ECSObject* restored = nullptr;
    const auto error = LoadObject(file, restored);
    auto release = [this](ECSObject* object) { delete_CSObject(object); };
    std::unique_ptr<ECSObject, decltype(release)> restoredOwner(restored, release);
    if (error) return error;
    uint8_t extension[4];
    if (file.Read(extension, sizeof(extension)) != sizeof(extension) || Read32(extension) != 0)
        return ESLErrorMsg("Invalid or truncated processor extension");
    // Commit only once the entire processor record is validated.
    if (m_vaStack && m_pcsxi)
        m_pcsxi->FreeVirtualAddressDirectory(m_vaStack, &m_stack.m_varArray);
    m_ip = Read32(wire); m_ipSegment = Read32(wire + 4);
    SetStatus(static_cast<ExecutionStatus>(status));
    m_nLastTickTime = Read64(wire + 12); m_dwLastBaseTime = ::timeGetTime();
    m_vaStack = static_cast<INT64>(Read64(wire + 20));
    m_vaNakedStack = static_cast<INT64>(Read64(wire + 28)); m_bufNakedStack = nullptr;
    for (unsigned i = 0; i < 256; ++i) m_regset[i].ui = Read64(wire + 36 + i * 8);
    delete_CSObject(m_pRetObj); m_pRetObj = restoredOwner.release();
    if (m_vaStack && m_pcsxi)
        m_vaStack = m_pcsxi->AllocateVirtualAddressDirectory(m_vaStack, &m_stack.m_varArray);
    return eslErrSuccess;
}''')
        # A failed resource serializer must still resume every script thread.
        save_start = text.index('ESLError ECSContext::Save( ESLFileObject & file )')
        save_end = text.index('ESLError ECSContext::SaveProcessorContext', save_start)
        save = text[save_start:save_end]
        finished = '''ECotophaScript::Lock( ) ;
\tpNextThread = m_pThreadList ;
\twhile ( pNextThread != NULL )
\t{
\t\tpNextThread->OnFinishedSave( *this ) ;
\t\tpNextThread = pNextThread->m_pNextThread ;
\t}
\tECotophaScript::Unlock( ) ;'''
        if save.count(finished) != 1:
            raise ValueError('Save thread completion block changed')
        save = save.replace(finished, '')
        save = save.replace('ECotophaScript::Unlock( ) ;', '''ECotophaScript::Unlock( ) ;
    std::shared_ptr<void> finishThreads(nullptr, [this](void*) {
        ECotophaScript::Lock();
        for (ECSThread* thread = m_pThreadList; thread; thread = thread->m_pNextThread)
            thread->OnFinishedSave(*this);
        ECotophaScript::Unlock();
    });
    // Input callbacks use an independent context on the graphics thread.
    // Freeze that object graph only after the script threads have suspended;
    // release this lock before the completion guard resumes those threads.
    SSystem::Lock();
    std::shared_ptr<void> snapshotLock(nullptr, [](void*) { SSystem::Unlock(); });''', 1)
        save = save.replace('m_pcsxi->SaveClassVector( emcfile ) ;',
                            'err = m_pcsxi->SaveClassVector(emcfile); if (err) return err;')
        save = save.replace('SaveProcessorContext( emcfile ) ;',
                            'err = SaveProcessorContext(emcfile); if (err) return err;')
        text = text[:save_start] + save + text[save_end:]
        text = text.replace('LoadProcessorContext( emcfile ) ;',
                            'err = LoadProcessorContext(emcfile); if (err) return err;')
        text = _replace(text, r'\*\(\(long int\*\)\(pImage \+ m_ip\)\)',
                        '*((SDWORD*)(pImage + m_ip))', count=3)
        text = _replace(text, r'sizeof\(long int\)', 'sizeof(SDWORD)', count=3)
        text = _replace(text,
            r'::eslMoveMemory\( wstrBuf.GetBuffer\(dwLength\),\s*pImage \+ m_ip, dwLength \* sizeof\(wchar_t\) \) ;\s*wstrBuf.ReleaseBuffer\( dwLength \) ;\s*m_ip \+= dwLength \* sizeof\(wchar_t\) ;',
            'StudySteadyLegacyWire::DecodeUtf16(pImage + m_ip, dwLength, wstrBuf);\n'
            '\t\t\tm_ip += dwLength * 2;', count=2)
        text = _replace(text,
            r'DWORD\s+dwLength = wstrType.GetLength\( \) ;.*?#endif',
            'if (!StudySteadyLegacyWire::WriteWideString(file, wstrType))\n'
            '\t\t\t\treturn ESLErrorMsg("Object type name write failed");')
        text = _replace(text,
            r'DWORD\s+dwLength ;\s*if \( file.Read\( &dwLength,.*?wstrType.ReleaseBuffer\( dwLength \) ;\s*\}',
            'if (!StudySteadyLegacyWire::ReadWideString(file, wstrType))\n'
            '\t\t\t\t\treturn ESLErrorMsg("Object type name read failed");')
    elif name == 'glscsobj_string.cpp':
        text = _inject(text)
        text = _replace(text,
            r'ESLError ECSString::Save\( ESLFileObject & file, ECSContext & context \)\s*\{.*?\n\}',
            '''ESLError ECSString::Save(ESLFileObject& file, ECSContext& context)
{
    const DWORD index = m_varStr.GetIndex();
    if (!StudySteadyLegacyWire::WriteWideString(file, m_varStr) ||
        file.Write(&index, sizeof(index)) != sizeof(index) ||
        file.Write(&m_nLockBufSize, sizeof(m_nLockBufSize)) != sizeof(m_nLockBufSize))
        return ESLErrorMsg("String save failed");
    return eslErrSuccess;
}''')
        text = _replace(text,
            r'ESLError ECSString::Load\( ESLFileObject & file, ECSContext & context \)\s*\{.*?\n\}',
            '''ESLError ECSString::Load(ESLFileObject& file, ECSContext& context)
{
    DWORD index;
    if (!StudySteadyLegacyWire::ReadWideString(file, m_varStr) ||
        file.Read(&index, sizeof(index)) != sizeof(index) ||
        file.Read(&m_nLockBufSize, sizeof(m_nLockBufSize)) != sizeof(m_nLockBufSize))
        return ESLErrorMsg("String load failed");
    m_varStr.MoveIndex(index);
    if (m_nLockBufSize > 0) m_varStr.GetBuffer(m_nLockBufSize);
    return eslErrSuccess;
}''')
    elif name in ('glscs_script_core.cpp', 'legacy_script_types.cpp'):
        text = _inject(text)
        text = _replace(text,
            r'ESLError ECSStrTagArray::SaveArray\( ESLFileObject & file \)\s*\{.*?\n\}',
            '''ESLError ECSStrTagArray::SaveArray(ESLFileObject& file)
{
    const DWORD count = GetSize();
    if (file.Write(&count, sizeof(count)) != sizeof(count)) return eslErrGeneral;
    for (DWORD i = 0; i < count; ++i) {
        const wchar_t* value = GetAt(i);
        ECSWideString text(value ? value : L"");
        if (!StudySteadyLegacyWire::WriteWideString(file, text)) return eslErrGeneral;
    }
    return eslErrSuccess;
}''')
        text = _replace(text,
            r'ESLError ECSStrTagArray::LoadArray\s*\( ESLFileObject & file, ECSContext & context \)\s*\{.*?\n\}',
            '''ESLError ECSStrTagArray::LoadArray(ESLFileObject& file, ECSContext& context)
{
    DWORD count;
    if (file.Read(&count, sizeof(count)) != sizeof(count)) return eslErrGeneral;
    for (DWORD i = 0; i < count; ++i) {
        ECSWideString text;
        if (!StudySteadyLegacyWire::ReadWideString(file, text)) return eslErrGeneral;
        Add(text.IsEmpty() ? NULL : context.GetConstantString(text)->CharPtr());
    }
    return eslErrSuccess;
}''')
    return text


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    source = args.source.read_bytes().decode('cp932').replace('\r\n', '\n')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(patch_source(str(args.source), source))
