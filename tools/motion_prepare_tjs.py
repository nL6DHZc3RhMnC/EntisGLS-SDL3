#!/usr/bin/env python3
"""Adapt NCB's engine include boundary to the native host without altering NCB."""
from pathlib import Path
import json
import re
import html
ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'vendor/kirikiroid2/ncbind'
OUT = ROOT / 'build/generated/motion-ncbind'


def write_changed(path, text):
    if not path.exists() or path.read_text() != text:
        path.write_text(text)

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    config = (ROOT / 'vendor/kirikiroid2/tjs2/tjsConfig.cpp').read_text()
    # A bounded, non-NUL-terminated source (including tTJSString(tjs_char))
    # must not read source[len]. ASAN caught the original order doing so.
    old = '        len++;\n        while((ch = *s) != 0 && --len)\n            *(d++) = ch, s++;'
    assert config.count(old) == 1
    config = config.replace(old, '        while(len != 0 && (ch = *s) != 0) {\n            *(d++) = ch; ++s; --len;\n        }')
    write_changed(OUT / 'tjsConfig.cpp', '// Generated bounded-copy fix; vendor source is unchanged.\n' + config)
    for name in ['ncbind.hpp','ncbind.cpp','ncb_invoke.hpp','ncb_foreach.inc']:
        text = (SOURCE / name).read_text()
        if name == 'ncbind.hpp':
            # NCB uses only script-global access, expression evaluation,
            # exception reporting and logging from these engine headers.
            # The adapter implements all of them using the real TJS instance.
            for header in ['StorageImpl.h','ScriptMgnIntf.h','DebugIntf.h','MsgIntf.h','CharacterSet.h']:
                text = text.replace('#include "' + header + '"', '#include "tjs_host.h"')
            text = text.replace('#include <map>', '#include <set>\n#include <map>')
        write_changed(OUT / name, '// Generated from imported NCB; see vendor/motion-deps/provenance.json.\n' + text)
    pairs = [(k, html.unescape(v)) for k, v in re.findall(r'<Item\s+id="([^"]+)"\s+text="([^"]*)"', (ROOT / 'vendor/kirikiroid2/tjs_support/en_us.xml').read_text())]
    text = '#include \"tjs.h\"\n#include <unordered_map>\nttstr TVPGetMessageByLocale(const std::string &key) {\n    static const std::unordered_map<std::string,std::string> messages = {\n'
    text += ''.join('        {' + json.dumps(k, ensure_ascii=False) + ',' + json.dumps(v, ensure_ascii=False) + '},\n' for k, v in pairs)
    text += '    };\n    const auto item = messages.find(key);\n    return ttstr(item == messages.end() ? key : item->second);\n}\n'
    write_changed(OUT / 'locale.cpp', text)

if __name__ == '__main__': main()
