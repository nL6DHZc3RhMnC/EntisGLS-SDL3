"""Commit prototypes while preserving the original container child-error policy."""
from pathlib import Path


def patch_source(relative_path: str, text: str) -> str:
    classes={'glscsobj_array.cpp':'ECSArray', 'glscsobj_hash.cpp':'ECSHash'}
    cls=classes.get(Path(relative_path).name)
    if not cls:
        return text
    start = text.index(f'ESLError {cls}::CommitAllReference')
    end = text.index(f'ESLError {cls}::Save', start)
    body = text[start:end]
    before = '//\t\t\t\treturn\terr ;'
    if body.count(before) != 1:
        raise ValueError(f'Expected one suppressed {cls}::CommitAllReference error')
    # Original GLS3 and ststeady.exe Array/Hash deliberately continue after
    # child reference errors. Static caches are rebuilt by OnContextLoaded,
    # after this traversal; later stack references still need to be committed.
    # Keep the original suppressed return exactly as supplied by the SDK.
    before='\treturn\teslErrSuccess ;'
    if body.count(before)!=1:
        raise ValueError(f'Expected one final {cls}::CommitAllReference success')
    body=body.replace(before,'\treturn m_pDefObj ? m_pDefObj->CommitAllReference(context) : eslErrSuccess;')
    return text[:start]+body+text[end:]
