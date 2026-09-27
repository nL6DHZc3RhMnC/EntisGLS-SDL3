"""Checked GLS3 stack records using the original Windows UTF-16 wire."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re


SAVE = r'''ESLError ECSStack::Save(ESLFileObject& file, ECSContext& context) try
{
    StudySteadyLegacy::CheckedStateFile checked(file);
    auto error=checked.Result(ECSArray::Save(checked,context));
    if(error)return error;
    auto write32=[&](uint32_t value){uint8_t bytes[4];StudySteadyLegacyWire::Write32(bytes,value);checked.Write(bytes,4);};
    const auto count=m_block.GetSize();
    if(count>65536)return eslErrInvalidParam;
    write32(uint32_t(count));
    for(size_t i=0;i<count;++i) {
        const auto* block=m_block.GetAt(i);
        if(!block||!block->m_pwstrName)return eslErrInvalidParam;
        write32(block->m_dwFlags);
        if(!StudySteadyLegacyWire::WriteWideString(checked,*block->m_pwstrName))return eslErrGeneral;
        write32(block->m_iBound);write32(block->m_nVarCount);write32(block->m_dwCatchAddr);
        error=checked.Result(const_cast<EStackBlock*>(block)->SaveArray(checked));
        if(error)return error;
    }
    return checked.Result();
} catch(const std::bad_alloc&) {return eslErrGeneral;}
'''

LOAD = r'''ESLError ECSStack::Load(ESLFileObject& file, ECSContext& context)
{
    auto clear=[&] {
        CleanupAllReference(context);RemoveAll();
        if(m_pDefObj)m_pDefObj->CleanupAllReference(context);
        SetDefaultElement(nullptr);SetBounds();
    };
    struct Incomplete {decltype(clear)& cleanup;bool complete=false;~Incomplete(){if(!complete)cleanup();}} incomplete{clear};
    try {
        clear();
        StudySteadyLegacy::CheckedStateFile checked(file);
        auto error=checked.Result(ECSArray::Load(checked,context));
        if(error)return error;
        auto read32=[&](){uint8_t bytes[4];checked.Read(bytes,4);return StudySteadyLegacyWire::Read32(bytes);};
        const auto count=read32();
        const auto position=checked.GetLargePosition(),length=checked.GetLargeLength();
        if(checked.failed||count>65536||position>length||count>(length-position)/24)return eslErrGeneral;
        for(uint32_t i=0;i<count;++i) {
            std::unique_ptr<EStackBlock> block(new EStackBlock);
            block->m_dwFlags=read32();
            ECSWideString name;
            if(!StudySteadyLegacyWire::ReadWideString(checked,name))return eslErrGeneral;
            block->m_pwstrName=context.GetConstantString(name);
            if(!block->m_pwstrName)return eslErrGeneral;
            block->m_iBound=read32();block->m_nVarCount=read32();block->m_dwCatchAddr=read32();
            if(checked.failed)return eslErrGeneral;
            error=checked.Result(block->LoadArray(checked,context));
            if(error)return error;
            m_block.Add(block.release());
        }
        UpdateCurrentFrame();
        incomplete.complete=true;return eslErrSuccess;
    } catch(const std::bad_alloc&) {return eslErrGeneral;}
}
'''


def patch_source(relative_path: str, text: str) -> str:
    if Path(relative_path).name != 'glscsobj_stack.cpp':
        return text
    root = ROOT
    helpers = (ROOT / 'native/runtime/cotopha_port/legacy_serialization.h').read_text()
    helpers += '\n' + (root/'native/runtime/cotopha_port/legacy_checked_state_file.h').read_text()
    if text.count('#include <gls.h>') != 1:
        raise ValueError('Expected original stack include')
    text = text.replace('#include <gls.h>', '#include <gls.h>\n#include <new>\n' + helpers)
    for name, replacement in [('Save', SAVE), ('Load', LOAD)]:
        pattern = r'ESLError ECSStack::' + name + r'\( ESLFileObject & file, ECSContext & context \)\s*\{.*?\n\}'
        text, count = re.subn(pattern, lambda _: replacement, text, flags=re.S)
        if count != 1:
            raise ValueError(f'Expected one original ECSStack::{name}, found {count}')
    return text
