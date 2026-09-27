"""Stop execution when a destructive primary-context restore cannot finish."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path


def patch_source(relative_path, text):
    if Path(relative_path).name != 'glscs_context.cpp':
        return text
    start = text.index('ESLError ECSContext::Load( ESLFileObject & file )')
    end = text.index('ESLError ECSContext::LoadProcessorContext', start)
    body = text[start:end]
    anchor = '\tECSThread *\tpNextThread ;'
    if body.count(anchor) != 1:
        raise ValueError('context restore guard start anchor')
    body = body.replace(anchor, '''    // The EMC header has passed validation. OnBeginningLoad and every step
    // below can replace live thread/factory/register/graph state. There is no
    // rollback to the caller's old stack; never execute a partially loaded one.
    struct LegacyRestoreAttempt {
        ECSContext& context;
        bool complete = false;
        ~LegacyRestoreAttempt() { if (!complete) context.SetStatus(ECSContext::xsHalt); }
    } restoreAttempt{*this};
''' + anchor)
    anchor = '\treturn\teslErrSuccess ;'
    if body.count(anchor) != 1:
        raise ValueError('context restore guard completion anchor')
    body = body.replace(anchor, '    restoreAttempt.complete = true;\n' + anchor)
    return text[:start] + body + text[end:]
