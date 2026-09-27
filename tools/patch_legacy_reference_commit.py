"""Keep consumed reference paths and owned-object backlinks stable on re-commit."""
from pathlib import Path


def patch_source(relative_path,text):
    if Path(relative_path).name!='glscsobj_reference.cpp':return text
    start=text.index('ESLError ECSReference::CommitAllReference');end=text.index('ESLError ECSReference::Save',start)
    body=text[start:end]
    old='''\tif ( m_pOwnObj != NULL )
\t{
\t\tAddReferenceBackLinkChain( ) ;
\t\treturn\tm_pOwnObj->CommitAllReference( context ) ;
\t}'''
    if body.count(old)!=1:raise ValueError('owned reference commit anchor')
    body=body.replace(old,'''\tif (m_pOwnObj != nullptr) {
        // Load creates the owner without a back-link; a repeated commit must
        // not insert this same owner twice and create a self-referential list.
        if (!m_pPrevBackRef && !m_pNextBackRef && m_pOwnObj->m_pBackRef != this)
            AddReferenceBackLinkChain();
        // Children can still contain newly loaded pending references.
        return m_pOwnObj->CommitAllReference(context);
    }
    // A successful resolution consumes m_dimIndex. Keep its real target on
    // repeated graph visits instead of replacing it with the storage root.
    if (m_pRef != nullptr && m_dimIndex.GetSize() == 0) return eslErrSuccess;''')
    return text[:start]+body+text[end:]
