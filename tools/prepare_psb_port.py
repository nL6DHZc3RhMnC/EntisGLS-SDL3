#!/usr/bin/env python3
"""Generate a native PSB reader from the imported parser, without TJS bindings.

The raw node and ownership implementation is retained. Only TJS file/Variant
entry points are omitted; callers supply a validated, decrypted byte buffer.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'vendor/kirikiroid2/psbfile'
OUT = ROOT / 'build/generated/psb'

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name in ('PSBRawFile.h', 'PSBPackedInternal.h', 'PSBRawFile.cpp'):
        text = (SRC / name).read_text()
        for header in ('tjs.h', 'MsgIntf.h', 'StorageIntf.h', 'tjsUtils.h'):
            text = text.replace(f'#include "{header}"', '#include "psb_support.h"')
        if name.endswith('.cpp'):
            text = '#include "psb_v4_resource.h"\n' + text
            start = text.index('    bool PSBFile::Load(tTJSVariant value)')
            end = text.index('    bool PSBFile::Adopt(', start)
            text = text[:start] + text[end:]
            signature = 'const std::uint8_t *PSBRawNode::GetResource(std::uint32_t &size) const {'
            text = text.replace(signature, signature + '''
        if (GetOwner()->GetHeader()->version == 4 && node_[0] >= 0x22 && node_[0] <= 0x25) {
            return studysteady::extraResource(GetOwner()->GetData(), GetOwner()->GetSize(), node_, size);
        }
''')
        if name == 'PSBRawFile.h':
            start = text.index('        [[nodiscard]] bool Load(tTJSVariant value);')
            end = text.index('        [[nodiscard]] bool Adopt(', start)
            text = text[:start] + text[end:]
        if name == 'PSBPackedInternal.h':
            # Restrict this adaptation to the classifier. Keep integer/float
            # decoding unchanged; this port only admits validated PSB v4.
            start = text.index('inline int GetTypeCategory_guess')
            end = text.index('inline tjs_int DecodeInteger32_guess', start)
            classifier = text[start:end]
            for tag in ('0x23', '0x24', '0x25'):
                classifier = classifier.replace(f'                case {tag}:\n', '')
            classifier = classifier.replace('                case 0x19:',
                '                case 0x22:\n                case 0x23:\n                case 0x24:\n                case 0x25:\n                case 0x19:')
            text = text[:start] + classifier + text[end:]
        (OUT / name).write_text('// Generated from vendor/kirikiroid2; see provenance.json and LICENSE.\n' + text)

if __name__ == '__main__':
    main()
