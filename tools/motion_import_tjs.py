#!/usr/bin/env python3
"""Copy only TJS/NCB and their selected source dependencies, with provenance."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path('/Users/fenghengzhi/Developer/kirikiroid2-web')
PACKAGES = SOURCE / 'out/macos/debug/vcpkg_installed/x64-osx'
ONIG = Path('/Users/fenghengzhi/Developer/toolchains/krkr2/vcpkg/buildtrees/oniguruma/src/v6.9.10-eca92fc9ff.clean')
MANIFEST = ROOT / 'vendor/motion-deps/provenance.json'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--collect-boost-from', action='append', default=[])
    args = parser.parse_args()
    manifest = json.loads(MANIFEST.read_text()) if MANIFEST.exists() else {'files': {}}
    manifest['source_repository'] = str(SOURCE)
    manifest['note'] = 'Selected working-tree sources unchanged; generated platform adapters are outside vendor.'

    def copy(src, target):
        if not src.is_file():
            raise FileNotFoundError(src)
        target = ROOT / target
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, target)
        manifest['files'][str(target.relative_to(ROOT))] = {
            'source': str(src), 'sha256': hashlib.sha256(src.read_bytes()).hexdigest()}

    if not args.collect_boost_from:
        for src in sorted((SOURCE / 'cpp/core/tjs2').rglob('*')):
            if src.is_file():
                copy(src, Path('vendor/kirikiroid2/tjs2') / src.relative_to(SOURCE / 'cpp/core/tjs2'))
        for name in ['ncbind.cpp', 'ncbind.hpp', 'ncb_invoke.hpp','ncb_foreach.inc']:
            copy(SOURCE / 'cpp/core/plugin' / name, Path('vendor/kirikiroid2/ncbind') / name)
        copy(SOURCE / 'cpp/core/common/LogoTrace.h', 'vendor/kirikiroid2/tjs_support/LogoTrace.h')
        copy(SOURCE / 'cpp/core/visual/ComplexRect.h', 'vendor/kirikiroid2/tjs_support/ComplexRect.h')
        copy(SOURCE / 'cpp/core/visual/ComplexRect.cpp', 'vendor/kirikiroid2/tjs_support/ComplexRect.cpp')
        copy(SOURCE / 'cpp/core/visual/RenderManager.h', 'vendor/kirikiroid2/tjs_support/RenderManager.h')
        copy(SOURCE / 'cpp/core/visual/ogl/RenderManager_ogl.cpp', 'vendor/kirikiroid2/tjs_support/RenderManager_ogl_reference.cpp')
        copy(SOURCE / 'cpp/core/environ/cpu_types.h', 'vendor/kirikiroid2/tjs_support/cpu_types.h')
        copy(SOURCE / 'LICENSE', 'vendor/kirikiroid2/tjs2/LICENSE')
        copy(SOURCE / 'LICENSE', 'vendor/kirikiroid2/ncbind/LICENSE')
        copy(SOURCE / 'ui/cocos-studio/locale/en_us.xml', 'vendor/kirikiroid2/tjs_support/en_us.xml')
        for library in ['fmt','spdlog']:
            for src in sorted((PACKAGES / 'include' / library).rglob('*')):
                if src.is_file(): copy(src, Path('vendor/motion-deps/include') / src.relative_to(PACKAGES / 'include'))
            copy(PACKAGES / 'share' / library / 'copyright', Path('vendor/motion-deps/licenses') / (library + '.txt'))
        for src in sorted((ONIG / 'src').rglob('*')):
            if src.is_file(): copy(src, Path('vendor/motion-deps/oniguruma') / src.relative_to(ONIG))
        for name in ['CMakeLists.txt','COPYING','cmake/Config.cmake.in','oniguruma.pc.cmake.in','onig-config.cmake.in']:
            copy(ONIG / name, Path('vendor/motion-deps/oniguruma') / name)
        copy(PACKAGES / 'share/boost-locale/copyright', 'vendor/motion-deps/licenses/boost-locale.txt')
    for directory in args.collect_boost_from:
        for dep in Path(directory).rglob('*.d'):
            for path in re.findall(r'/[^\s\\]+',dep.read_text()):
                src = Path(path)
                try: relative = src.relative_to(PACKAGES / 'include')
                except ValueError: continue
                if relative.parts[0] == 'boost': copy(src, Path('vendor/motion-deps/include') / relative)
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')
    print(f'{len(manifest["files"])} source/dependency files in {MANIFEST}')

if __name__ == '__main__': main()
