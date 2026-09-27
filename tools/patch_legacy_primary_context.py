"""Bounded primary-context diagnostics, without changing interpreter state."""
from pathlib import Path


def once(text,old,new):
    if text.count(old)!=1:raise ValueError(f'primary trace anchor count {text.count(old)}: {old[:100]}')
    return text.replace(old,new)


def patch_source(relative_path,text):
    kind=Path(relative_path).name
    if kind in ('glscsobj_array.cpp','glscsobj_hash.cpp'):
        text=once(text,'#include <gls.h>','#include <gls.h>\n#include "../platform/log.h"')
        cls='ECSArray' if kind=='glscsobj_array.cpp' else 'ECSHash'
        element='pElement' if cls=='ECSArray' else 'pObj'
        start=text.index('ESLError '+cls+'::CommitAllReference');end=text.index('ESLError '+cls+'::Save',start)
        body=text[start:end]
        body=once(body,'//\t\t\t\treturn\terr ;','\t\t\t\tstudy::platform::LogPrint(study::platform::LogPriority::Warn,"StudySteady","Primary commit child unresolved (original container policy continues) owner=%p type=%ls index=%d child=%p childType=%ls error=%s",this,GetTypeName(),i,'+element+','+element+'->GetTypeName(),GetESLErrorMsg(err));')
        return text[:start]+body+text[end:]
    if kind=='glscsobj_reference.cpp':
        text=once(text,'#include <gls.h>','#include <gls.h>\n#include "../platform/log.h"\n#include <string>\n#include <vector>')
        start=text.index('ESLError ECSReference::CommitAllReference');end=text.index('ESLError ECSReference::Save',start)
        body=text[start:end]
        anchor='\n{\n'
        body=once(body,anchor,anchor+'''    if (m_omBaseMode == csomStack && m_dimIndex.GetSize()) {
        std::string path;
        for (size_t i=0;i<m_dimIndex.GetSize()&&i<24;++i) path+=(i?",":"")+std::to_string(m_dimIndex.GetAt(i));
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Primary reference pending ref=%p own=%p path=[%s]",this,m_pOwnObj,path.c_str());
    }
''')
        trace=(Path(__file__).resolve().parents[1]/'native/legacy_reference_trace.inc').read_text()
        body=once(body,'\n{\n','\n{\n'+trace)
        body=once(body,'\t\tm_pRefParent = m_pRef ;','        if (!m_pRef) {\n            reportMissingReference("missing-base-or-parent", i);\n            return ESLErrorMsg("参照先オブジェクトが見つかりません。");\n        }\n\t\tm_pRefParent = m_pRef ;')
        body=once(body,'\t\tif ( m_pRef == NULL )','        traceSteps.push_back({m_pRefParent,m_pRef,m_iParentRef});\n\t\tif ( m_pRef == NULL )')
        body=once(body,'\t\t\tm_pRefParent = NULL ;','            reportMissingReference("missing-intermediate-target", i);\n\t\t\tm_pRefParent = NULL ;')
        marker='\tm_dimIndex.RemoveAll( ) ;' 
        body=once(body,marker,'''    if (m_omBaseMode == csomStack)
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Primary reference resolved ref=%p target=%p parent=%p slot=%d dimensions=%u",this,m_pRef,m_pRefParent,m_iParentRef,m_dimIndex.GetSize());
'''+marker)
        return text[:start]+body+text[end:]
    if Path(relative_path).name!='glscs_context.cpp':return text
    helper=(Path(__file__).resolve().parents[1]/'native/legacy_primary_context_trace.inc').read_text()
    text=once(text,'#include <gls.h>','#include <gls.h>\n'+helper)
    start=text.index('ESLError ECSContext::Save( ESLFileObject & file )');end=text.index('ESLError ECSContext::SaveProcessorContext',start)
    body=once(text[start:end],'m_arg.IndexAllMember( ) ;','m_arg.IndexAllMember( ) ;\n    LegacyTracePrimary(*this,"save-indexed");')
    text=text[:start]+body+text[end:]
    start=text.index('ESLError ECSContext::Load( ESLFileObject & file )');end=text.index('ESLError ECSContext::LoadProcessorContext',start)
    body=text[start:end]
    for expr,stage in [('m_pcsxi->m_csgGlobal.CommitAllReference( *this )','before-global-commit'),('m_stack.CommitAllReference( *this )','before-stack-commit'),('m_arg.CommitAllReference( *this )','before-arg-commit'),('CommitLoadedProcessorContext()','before-processor-commit')]:
        needle='err = '+expr+' ;'
        body=once(body,needle,'LegacyTracePrimary(*this,"'+stage+'");\n\t'+needle+'\n    if (err) study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Primary restore failed stage='+stage+' code=%d message=%s",int(err),GetESLErrorMsg(err));')
    body=once(body,'return\teslErrSuccess ;','LegacyTracePrimary(*this,"load-complete");\n\treturn\teslErrSuccess ;')
    text=text[:start]+body+text[end:]
    start=text.index('ESLError ECSContext::ExecuteEnter( void )');end=text.index('ESLError ECSContext::ExecuteLeave',start)
    body=text[start:end]
    body=once(body,'return\teslErrSuccess ;','if (*pwstrFuncName == L"UISave::Release") LegacyTracePrimary(*this,"release-enter");\n\treturn\teslErrSuccess ;')
    text=text[:start]+body+text[end:]
    start=text.index('ESLError ECSContext::ExecuteLoad( void )');end=text.index('ESLError ECSContext::ExecuteStore',start)
    body=text[start:end]
    needle='err = pObj->GetVariableIndex( nIndex, iElement ) ;'
    body=once(body,needle,needle+'\n                if (err && csomType == csomThis) LegacyTracePrimary(*this,"this-index-failed");')
    return text[:start]+body+text[end:]
