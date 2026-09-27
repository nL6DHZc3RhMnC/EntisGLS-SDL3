#!/usr/bin/env python3
"""Preserve the Win32 UTF-16 naked-memory ABI on native wchar_t=32 hosts.

Object strings remain native wide strings. Naked String.GetBuffer/LockBuffer
expose an owned UTF-16 buffer; UnlockBuffer or FlushBuffer commits writes.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re
from patch_legacy_serialization import _inject, _replace


STRING_METHODS = r'''
bool ECSString::PrepareLegacyUtf16()
{
    if (m_legacyUtf16Locked) return true;
    if (!m_legacyUtf16.empty() && m_legacyUtf16Source == m_varStr) return true;
    if (!StudySteadyLegacyWire::EncodeUtf16(m_varStr, m_legacyUtf16)) return false;
    m_legacyUtf16Units = m_legacyUtf16.size() / 2;
    m_legacyUtf16.push_back(0); m_legacyUtf16.push_back(0);
    m_legacyUtf16Source = m_varStr;
    return true;
}
size_t ECSString::LegacyUtf16Length()
{
    return PrepareLegacyUtf16() ? m_legacyUtf16Units : 0;
}
bool ECSString::LockLegacyUtf16(int units)
{
    if (units < 0 || m_legacyUtf16Locked || !PrepareLegacyUtf16()) return false;
    const size_t capacity = std::max<size_t>(units, m_legacyUtf16Units);
    m_legacyUtf16.resize((capacity + 1) * 2, 0);
    m_legacyUtf16Locked = true;
    return true;
}
bool ECSString::UnlockLegacyUtf16(int units)
{
    if (!m_legacyUtf16Locked) return false;
    const size_t capacity = m_legacyUtf16.size() / 2 - 1;
    size_t length = 0;
    if (units < 0) {
        while (length < capacity &&
               (m_legacyUtf16[length * 2] || m_legacyUtf16[length * 2 + 1])) ++length;
    } else {
        if (size_t(units) > capacity) return false;
        length = size_t(units);
    }
    if (!StudySteadyLegacyWire::DecodeUtf16(m_legacyUtf16.data(), length, m_varStr)) return false;
    m_legacyUtf16Units = length;
    m_legacyUtf16Locked = false;
    m_legacyUtf16Source = m_varStr;
    m_legacyUtf16[length * 2] = m_legacyUtf16[length * 2 + 1] = 0;
    return true;
}
void ECSString::FlushBuffer(int offset, int bytes, void* ptr, bool modified)
{
    if (!modified || m_legacyUtf16Locked || offset < 0 || bytes < 0 ||
        m_legacyUtf16.empty() || size_t(offset) > m_legacyUtf16Units * 2 ||
        size_t(bytes) > m_legacyUtf16Units * 2 - size_t(offset) ||
        ptr != m_legacyUtf16.data() + offset) return;
    StudySteadyLegacyWire::DecodeUtf16(m_legacyUtf16.data(), m_legacyUtf16Units, m_varStr);
    m_legacyUtf16Source = m_varStr;
}
'''


def _function(text, name, body):
    # Exported naked-call function bodies have their closing brace in column 0.
    return _replace(text,
        r'(?:ECS_EXPORT const wchar_t \*\s*)?' + name + r'\s*\([^)]*\)\s*\{.*?\n\}', body)


def patch_source(relative_path: str, text: str) -> str:
    name = Path(relative_path).name
    if name == 'glscsobj_string.h':
        declarations = '''class ECSString : public ECSObject
{
private:
    std::vector<uint8_t> m_legacyUtf16;
    EWideString m_legacyUtf16Source;
    size_t m_legacyUtf16Units = 0;
    bool m_legacyUtf16Locked = false;
public:
    bool PrepareLegacyUtf16();
    size_t LegacyUtf16Length();
    bool LockLegacyUtf16(int units);
    bool UnlockLegacyUtf16(int units);
    void FlushBuffer(int offset, int bytes, void* ptr, bool modified) override;
'''
        text = '#include <vector>\n#include <cstdint>\n' + text
        text = _replace(text, r'class\s+ECSString\s*:\s*public\s+ECSObject\s*\{', declarations)
    elif name == 'glscsobj_string.cpp':
        if 'namespace StudySteadyLegacyWire' not in text: text = _inject(text)
        text = _replace(text,
            r'void \* ECSString::GetBuffer\( int iOffset, int nSize, bool fWritable \)\s*\{.*?\n\}',
            '''void* ECSString::GetBuffer(int offset, int bytes, bool writable)
{
    if (offset < 0 || bytes < 0 || !PrepareLegacyUtf16()) return NULL;
    const size_t available = m_legacyUtf16Locked ? m_legacyUtf16.size() - 2 : m_legacyUtf16Units * 2;
    if (size_t(offset) > available || size_t(bytes) > available - size_t(offset)) return NULL;
    return m_legacyUtf16.data() + offset;
}''')
        text = _replace(text,
            r'ECSSakura2Processor::LinearAddressCache \*\s*ECSString::GetSegmentBuffer\([^)]*\)\s*\{.*?\n\}',
            '''ECSSakura2Processor::LinearAddressCache*
ECSString::GetSegmentBuffer(ECSSakura2Processor::LinearAddressCache& seg)
{
    if (!PrepareLegacyUtf16()) return NULL;
    seg.baseOffset = 0;
    seg.limitSegment = m_legacyUtf16.size();
    seg.pbytBuffer = m_legacyUtf16.data();
    return &seg;
}
''' + STRING_METHODS)
        text = text.replace('pThis->m_varStr.GetLength() ;', 'pThis->LegacyUtf16Length() ;')
        for method in ('SetString', 'AppendString'):
            operation = '=' if method == 'SetString' else '+='
            text = _function(text, 'ecs_nakedcall_String_' + method,
                '''ECS_EXPORT const wchar_t* ecs_nakedcall_String_''' + method + '''
(ECSSakura2Processor::Context* raw, const ECSSakura2Processor::Register* args)
{
    ECSContext* context = static_cast<ECSContext*>(raw);
    int offset;
    auto* value = ESLTypeCast<ECSString>(context->GetObjectFromLinearAddress(args[0].i, offset));
    if (!value) return L"Invalid String object";
    EWideString source;
    const wchar_t* decoded = args[2].i < 0
        ? context->AtomicLoadString(source, args[1].i)
        : context->AtomicLoadString(source, args[1].i, args[2].l32);
    if (!decoded) return L"Invalid UTF-16 memory range";
    value->m_varStr ''' + operation + ''' source;
    context->ResetAddressTranslationCache();
    return NULL;
}''')
        text = _function(text, 'ecs_nakedcall_String_GetBuffer', '''ECS_EXPORT const wchar_t*
ecs_nakedcall_String_GetBuffer(ECSSakura2Processor::Context* raw, const ECSSakura2Processor::Register* args)
{
    auto* context = static_cast<ECSContext*>(raw);
    int offset;
    auto* value = ESLTypeCast<ECSString>(context->GetObjectFromLinearAddress(args[0].i, offset));
    if (!value || !value->PrepareLegacyUtf16()) return L"Invalid String buffer";
    context->ResetAddressTranslationCache();
    context->m_regset[ECSSakura2Processor::regAcc] = args[0];
    return NULL;
}''')
        text = text.replace('pThis->LockBuffer( pArg[1].l32 ) ;',
            'if (!pThis->LockLegacyUtf16(pArg[1].l32)) return L"Invalid String.LockBuffer";\n'
            '\t\tcontext->ResetAddressTranslationCache();')
        text = text.replace('pThis->UnlockBuffer( pArg[1].l32 ) ;',
            'if (!pThis->UnlockLegacyUtf16(pArg[1].l32)) return L"Invalid String.UnlockBuffer";\n'
            '\t\tcontext->ResetAddressTranslationCache();')
        text = _replace(text,
            r'void ECSString::OnDestruction\( ECSContext & context \)\s*\{.*?\n\}',
            '''void ECSString::OnDestruction(ECSContext& context)
{
    m_varStr.FreeString();
    m_legacyUtf16.clear();
    m_legacyUtf16Source.FreeString();
    m_legacyUtf16Units = 0;
    m_legacyUtf16Locked = false;
}''')
    elif name == 'glscs_context_naked.cpp':
        text = _inject(text)
        prefix = r'const wchar_t \* ECSContext::AtomicLoadString\s*\( EWideString & wstrBuf, ECSSakura2::Object \* pObj, int iOffset'
        text = _replace(text, prefix + r' \)\s*\{.*?\n\}', '''const wchar_t* ECSContext::AtomicLoadString
(EWideString& out, ECSSakura2::Object* object, int offset)
{
    if (!object || offset < 0) return NULL;
    ECSSakura2Processor::LinearAddressCache seg;
    if (object->GetSegmentBuffer(seg)) {
        const int64_t begin = int64_t(offset) - seg.baseOffset;
        if (begin < 0 || uint64_t(begin) > seg.limitSegment) return NULL;
        const uint8_t* bytes = seg.pbytBuffer + begin;
        const size_t limit = (seg.limitSegment - size_t(begin)) / 2;
        size_t length = 0;
        while (length < limit && (bytes[length*2] || bytes[length*2+1])) ++length;
        if (length == limit) return NULL;
        return StudySteadyLegacyWire::DecodeUtf16(bytes, length, out) ? out.CharPtr() : NULL;
    }
    auto* legacy = ESLTypeCast<ECSObject>(object);
    if (!legacy) return NULL;
    std::vector<uint8_t> bytes;
    for (;;) {
        auto* unit = static_cast<uint8_t*>(legacy->GetBuffer(offset, 2, false));
        if (!unit) return NULL;
        const uint8_t low = unit[0], high = unit[1];
        legacy->FlushBuffer(offset, 2, unit, false);
        if (!low && !high) break;
        bytes.push_back(low); bytes.push_back(high);
        if (offset > INT_MAX - 2) return NULL;
        offset += 2;
    }
    return StudySteadyLegacyWire::DecodeUtf16(bytes.data(), bytes.size()/2, out) ? out.CharPtr() : NULL;
}''')
        text = _replace(text, prefix + r', int nLength \)\s*\{.*?\n\}', '''const wchar_t* ECSContext::AtomicLoadString
(EWideString& out, ECSSakura2::Object* object, int offset, int units)
{
    if (!object || offset < 0 || units < 0 || units > INT_MAX / 2) return NULL;
    ECSSakura2Processor::LinearAddressCache seg;
    if (object->GetSegmentBuffer(seg)) {
        const int64_t begin = int64_t(offset) - seg.baseOffset;
        if (begin < 0 || uint64_t(begin) > seg.limitSegment ||
            size_t(units) * 2 > seg.limitSegment - size_t(begin)) return NULL;
        return StudySteadyLegacyWire::DecodeUtf16(seg.pbytBuffer + begin, units, out) ? out.CharPtr() : NULL;
    }
    auto* legacy = ESLTypeCast<ECSObject>(object);
    if (!legacy) return NULL;
    void* bytes = legacy->GetBuffer(offset, units * 2, false);
    if (!bytes && units) return NULL;
    bool success = StudySteadyLegacyWire::DecodeUtf16(static_cast<uint8_t*>(bytes), units, out);
    legacy->FlushBuffer(offset, units * 2, bytes, false);
    return success ? out.CharPtr() : NULL;
}''')
    return text
