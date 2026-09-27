"""Keep the GLS3 heap's real legacy object identities and Buffer wire."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re


def replace_function(text, signature, body):
    pattern = signature + r'\s*\{.*?\n\}'
    text, count = re.subn(pattern, lambda _: body, text, flags=re.S)
    if count != 1:
        raise ValueError(f'heap patch expected one function, got {count}: {signature}')
    return text


def patch_source(relative_path, text):
    name = Path(relative_path).name
    if name == 'glscs_execution_image.cpp':
        text = replace_function(text,
            r'ECSSakura2::Object\s*\*\s*ECSExecutionImage::NewObjectByIdentity\s*\([^)]*\)',
            '''ECSSakura2::Object * ECSExecutionImage::NewObjectByIdentity(
    ECSSakura2Processor::Context * context, int cls_id)
{
    if (!context || cls_id < 0) return nullptr;
    const auto* name = m_vectorNewObject.GetEntryIndex().GetAt(cls_id);
    if (!name) return nullptr;
    // "Buffer" in a GLS3 heap is ECSBuffer, not modern SSystem::Buffer.
    // Resolve the saved vector name through the real object interpreter first.
    auto* object = static_cast<ECSContext*>(context)->CreateObject(csvtObject, *name);
    if (object) return object;
    return StandardVM::NewObjectByIdentity(context, cls_id);
}''')
    elif name == 'glscs_context.cpp':
        needle='m_pcsxi->LoadClassVector( emcfile ) ;'
        if text.count(needle)!=1:raise ValueError('heap classvec error propagation anchor')
        text=text.replace(needle,'err = m_pcsxi->LoadClassVector(emcfile); if (err) return err;')
        needle='err = CommitLoadedProcessorContext() ;'
        if text.count(needle)!=1:raise ValueError('heap return object commit anchor')
        text=text.replace(needle,'''if (m_pRetObj) { err = m_pRetObj->CommitAllReference(*this); if (err) return err; }
    err = CommitLoadedProcessorContext() ;''')
        text=replace_function(text,r'ESLError ECSContext::CommitLoadedProcessorContext\s*\([^)]*\)','''ESLError ECSContext::CommitLoadedProcessorContext()
{
    if (m_vaNakedStack && m_pcsxi) {
        m_bufNakedStack=ESLTypeCast<ECSBuffer>(m_pcsxi->ObjectFromAddress(uint32_t(m_vaNakedStack>>32)));
        if (!m_bufNakedStack) return ESLErrorMsg("Saved naked stack does not resolve to a legacy Buffer");
    } else if (m_vaNakedStack) return eslErrInvalidParam;
    return eslErrSuccess;
}''')
    elif name == 'glscsobj_buffer.cpp':
        helper = (ROOT / 'native/runtime/cotopha_port/legacy_serialization.h').read_text()
        text = text.replace('#include <gls.h>', '#include <gls.h>\n' + helper + '\n#include <vector>')
        text = replace_function(text, r'ESLError ECSBuffer::Save\s*\([^)]*\)',
            '''ESLError ECSBuffer::Save(ESLFileObject& file, ECSContext&)
{
    if (m_nBufSize < 0 || m_nBufBase < 0 || uint64_t(m_nBufSize) > 64*1024*1024)
        return eslErrInvalidParam;
    uint8_t header[8];
    StudySteadyLegacyWire::Write32(header,uint32_t(m_nBufBase));
    StudySteadyLegacyWire::Write32(header+4,uint32_t(m_nBufSize));
    if (file.Write(header,8)!=8 || (m_nBufSize && file.Write(m_pbytBuf,m_nBufSize)!=size_t(m_nBufSize)))
        return eslErrGeneral;
    return eslErrSuccess;
}''')
        text = replace_function(text, r'ESLError ECSBuffer::Load\s*\([^)]*\)',
            '''ESLError ECSBuffer::Load(ESLFileObject& file, ECSContext&)
{
    uint8_t header[8];if(file.Read(header,8)!=8)return eslErrGeneral;
    const uint32_t base=StudySteadyLegacyWire::Read32(header),size=StudySteadyLegacyWire::Read32(header+4);
    if(base>INT32_MAX || size>64*1024*1024 || uint64_t(base)+size>INT32_MAX)
        return eslErrInvalidParam;
    const auto position=file.GetPosition(),length=file.GetLength();
    if(position>length || size>length-position)return eslErrGeneral;
    std::vector<uint8_t> bytes(size);
    if(size && file.Read(bytes.data(),size)!=size)return eslErrGeneral;
    if(size) {
        if(const auto error=CreateBuffer(int(size),int(base)))return error;
        std::memcpy(m_pbytBuf,bytes.data(),size);
    } else {FreeBuffer();m_nBufBase=int(base);}
    return eslErrSuccess;
}''')
    return text
