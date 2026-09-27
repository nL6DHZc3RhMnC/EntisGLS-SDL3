"""Checked platform fixes for the real GLS3 compiler, never a replacement parser."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
import re

def patch_source(relative_path, text):
    name=Path(relative_path).name
    if name in ('glscs_compiler.cpp','glscs_assembler.cpp'):
        # MSVC accepted two user-defined conversions in copy-initialization.
        # Direct-initialization selects the same wide-string constructors.
        text=re.sub(r'\b(ECSSourceStream|SYMBOL_NAMESPACE|EString)(\s+\w+)\s*=\s*([^;]+);',
                    lambda m:m[1]+m[2]+'( '+m[3].strip()+' ) ;',text)
    if name=='glscs_compiler.cpp':
        start=text.index('ESLFileObject * ECSCompiler::OpenScriptFile')
        end=text.index('// エラーを出力する',start)
        text=text[:start]+'''ESLFileObject * ECSCompiler::OpenScriptFile(const char *path) {
    if (!path) return nullptr;
    if (auto *environment = m_ctxExpr.GetEnvironment())
        return environment->OpenFileObject(path, ESLFileObject::modeRead | ESLFileObject::shareRead);
    const EWideString wide(path);
    constexpr int flags = ESLFileObject::modeRead | ESLFileObject::shareRead;
    auto *file = SSystem::SFileOpener::DefaultNewOpenFile(wide, flags);
    return file ? new LegacyFileAdapter(file, flags) : nullptr;
}
\n'''+text[end:]
        # File adapters can represent NOA members with no standalone OS path.
        # Preserve the supplied source name for diagnostics and relative includes.
        text,n=re.subn(r'\tERawFile \*\tpRawFile = ESLTypeCast<ERawFile>\( pfile \) ;\n\tif \( pRawFile != NULL \)\n\t\{.*?\n\t\}', '',text,count=1,flags=re.S)
        if n!=1:raise ValueError('compiler raw-file diagnostic patch did not match')
    if name=='glscs_execution_image_compiler.cpp':
        # String.Calculate evaluates ECSObjects directly, and never enters the
        # optional Sakura2 bytecode optimizer. Keep emitted code unoptimized if
        # a future caller uses this image class; interpretation is unchanged.
        start=text.index('void ECSExecutionImageCompiler::FinishSakura2Optimize')
        text=text[:start]+'''void ECSExecutionImageCompiler::FinishSakura2Optimize(int, int) {
    m_dwOptimizeStart = 0;
    m_dwOptimizeEnd = 0;
}
'''
    if name=='glscsobj_string.cpp':
        start=text.index('ESLError ECSString::Call_Calculate')
        end=text.index('ESLError ECSString::Call_Execute',start)
        text=text[:start]+'''ESLError LegacyCalculateString(ECSString &, ECSContext &, ECSObjArray<ECSObject> &);
ESLError ECSString::Call_Calculate(ECSContext &context, ECSObjArray<ECSObject> &args) {
    return LegacyCalculateString(*this, context, args);
}
\n'''+text[end:]
    return text
