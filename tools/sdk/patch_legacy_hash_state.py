"""Checked Hash records with original Win32 UTF-16 keys and null-slot markers."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re

SAVE=r'''ESLError ECSHash::Save(ESLFileObject& file, ECSContext& context) try
{
    StudySteadyLegacy::CheckedStateFile checked(file);
    auto write32=[&](uint32_t value){uint8_t bytes[4];StudySteadyLegacyWire::Write32(bytes,value);checked.Write(bytes,4);};
    write32(m_varArray.GetSize());
    for(size_t i=0;i<m_varArray.GetSize();++i) {
        auto* element=m_varArray.GetAt(i);
        if(!element){write32(UINT32_MAX);continue;}
        if(!StudySteadyLegacyWire::WriteWideString(checked,element->Tag()))return eslErrGeneral;
        if(const auto error=checked.Result(context.SaveObject(checked,element->GetObject())))return error;
    }
    return checked.Result(context.SaveObject(checked,m_pDefObj));
} catch(const std::bad_alloc&) {return eslErrGeneral;}
'''

LOAD=r'''ESLError ECSHash::Load(ESLFileObject& file, ECSContext& context)
{
    auto clear=[&] {
        CleanupAllReference(context);m_varArray.RemoveAll();
        if(m_pDefObj)m_pDefObj->CleanupAllReference(context);
        SetDefaultElement(nullptr);
    };
    struct Incomplete {decltype(clear)& cleanup;bool complete=false;~Incomplete(){if(!complete)cleanup();}} incomplete{clear};
    try {
        clear();StudySteadyLegacy::CheckedStateFile checked(file);
        auto read32=[&](){uint8_t bytes[4];checked.Read(bytes,4);return StudySteadyLegacyWire::Read32(bytes);};
        const auto count=read32();
        auto position=checked.GetLargePosition(),length=checked.GetLargeLength();
        if(checked.failed||count>=0x40000000||position>length||count>(length-position)/4)return eslErrGeneral;
        m_varArray.SetLimit(count);
        for(uint32_t i=0;i<count;++i) {
            const auto units=read32();if(checked.failed)return eslErrGeneral;
            if(units==UINT32_MAX){m_varArray.SetAt(i,nullptr);continue;}
            position=checked.GetLargePosition();
            if(position>length||units>8*1024*1024||units>(length-position)/2)return eslErrGeneral;
            std::vector<uint8_t> bytes(size_t(units)*2);
            if(!bytes.empty())checked.Read(bytes.data(),bytes.size());
            std::unique_ptr<ETaggedElement<ECSWideString,ECSObject>> element(new ETaggedElement<ECSWideString,ECSObject>);
            if(checked.failed||!StudySteadyLegacyWire::DecodeUtf16(bytes.data(),units,element->Tag()))return eslErrGeneral;
            ECSObject* object=nullptr;
            if(const auto error=checked.Result(context.LoadObject(checked,object))){context.delete_CSObject(object);return error;}
            element->SetObject(object);m_varArray.SetAt(i,element.release());
        }
        if(const auto error=checked.Result(context.LoadObject(checked,m_pDefObj)))return error;
        incomplete.complete=true;return eslErrSuccess;
    } catch(const std::bad_alloc&) {return eslErrGeneral;}
}
'''

def patch_source(relative_path: str,text: str)->str:
    if Path(relative_path).name!='glscsobj_hash.cpp':
        return text
    root=ROOT
    helpers=(ROOT / 'native/runtime/cotopha_port/legacy_serialization.h').read_text()
    helpers+='\n'+(root/'native/runtime/cotopha_port/legacy_checked_state_file.h').read_text()
    if text.count('#include <gls.h>')!=1:
        raise ValueError('Expected original Hash include')
    text=text.replace('#include <gls.h>','#include <gls.h>\n#include <new>\n'+helpers)
    for name,replacement in [('Save',SAVE),('Load',LOAD)]:
        pattern=r'ESLError ECSHash::'+name+r'\( ESLFileObject & file, ECSContext & context \)\s*\{.*?\n\}'
        text,count=re.subn(pattern,lambda _:replacement,text,flags=re.S)
        if count!=1:raise ValueError(f'Expected one original Hash::{name}, found {count}')
    return text
