#!/usr/bin/env python3
"""Build the genuine PSB raw owner + TJS value dispatch with game v4 resources.

Storage and PSBFile script-class registration are separate integration surfaces;
this target adopts already-decoded bytes and retains the complete value dispatch.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

from pathlib import Path
SOURCE = ROOT / 'vendor/kirikiroid2/psbfile'
OUT = ROOT / 'build/generated/motion-tjs-psb/psbfile'


def write_changed(path, text):
    if not path.exists() or path.read_text() != text:
        path.write_text(text)

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name in ['PSBRawFile.h','PSBRawFile.cpp','PSBPackedInternal.h','PSBDispatch.h','PSBFile.cpp']:
        text = (SOURCE / name).read_text()
        text = text.replace('#include "MsgIntf.h"','#include "tjs_host.h"')
        if name == 'PSBRawFile.cpp':
            text = '#include "extensions/emote/psb/psb_v4_resource.h"\n' + text
            text = text.replace('#include "StorageIntf.h"','')
            start = text.index('    bool PSBFile::Load(tTJSVariant value)')
            end = text.index('    bool PSBFile::Adopt(',start)
            text = text[:start] + text[end:]
            signature = 'const std::uint8_t *PSBRawNode::GetResource(std::uint32_t &size) const {'
            text = text.replace(signature,signature + '''
        if (GetOwner()->GetHeader()->version == 4 && node_[0] >= 0x22 && node_[0] <= 0x25)
            return studysteady::extraResource(GetOwner()->GetData(), GetOwner()->GetSize(), node_, size);
''')
        if name == 'PSBFile.cpp':
            text = text[:text.index('\ntemplate <typename T>\nclass PSBFileConvertor')]
            for header in ['PSBMediaRegistry.h','ncbind.hpp']:
                text = text.replace('#include "' + header + '"','')
            text = '#include "extensions/emote/psb/psb_v4_resource.h"\n' + text
            signature = 'const std::uint8_t *node, std::uint32_t &size) const {'
            text = text.replace(signature, signature + '''
        if (value_.GetOwner()->GetHeader()->version == 4 && node[0] >= 0x22 && node[0] <= 0x25)
            return studysteady::extraResource(value_.GetOwner()->GetData(), value_.GetOwner()->GetSize(), node, size);
''')
        if name == 'PSBPackedInternal.h':
            start = text.index('inline int GetTypeCategory_guess')
            end = text.index('inline tjs_int DecodeInteger32_guess',start)
            part = text[start:end]
            for tag in ['0x23','0x24','0x25']: part = part.replace(f'                case {tag}:\n','')
            part = part.replace('                case 0x19:', '                case 0x22:\n                case 0x23:\n                case 0x24:\n                case 0x25:\n                case 0x19:')
            text = text[:start] + part + text[end:]
        write_changed(OUT / name, '// Generated from imported psbfile; see vendor/kirikiroid2/provenance.json.\n' + text)

if __name__ == '__main__': main()
