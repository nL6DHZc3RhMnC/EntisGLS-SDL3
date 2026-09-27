#!/usr/bin/env python3
"""Generate explicit SDL portability overlays, preserving supplied SDK sources."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
from entis_sdk import COTOPHA, ROOT
from prepare_sdl_window import prepare as prepare_window, write
from prepare_sdl_system import replace_body
from prepare_sdl_graphics import transform_context


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    out = parser.parse_args().output.resolve()
    prepare_window(out)
    subprocess.run([sys.executable, str(ROOT/'tools/prepare_sdl_sync.py'),
        '--source', str(COTOPHA/'Source/common/sakura/ssys_synchronism.cpp'),
        '--output', str(out/'source/ssys_synchronism.cpp')], check=True)
    subprocess.run([sys.executable, str(ROOT/'tools/prepare_sdl_system.py'),
        '--sdk', str(COTOPHA), '--output', str(out/'system')], check=True)
    records = []
    generated = {}
    def source_copy(relative, transform):
        src = COTOPHA/relative
        text = transform(src.read_text(encoding='utf-8-sig'))
        target = out/relative
        digest = hashlib.sha256(src.read_bytes()).hexdigest()
        generated[target] = '// SDL overlay input SHA-256 '+digest+'\n'+text
        records.append({'source': str(src.relative_to(ROOT)), 'sha256': digest,
                        'output': str(target.relative_to(out))})

    # C++17 libc++ must not see C stdatomic macros. SDK uses compiler atomic
    # builtins; only the memory-order constant is needed by its common sources.
    source_copy('Include/unix/esl/esl_stddefs.h', lambda s:
        s.replace('#include <stdatomic.h>', '#include <atomic>\nusing std::memory_order_seq_cst;'))
    source_copy('Include/common/sakura/sakura_cpp_presets.h', lambda s:
        '#include <cstdarg>\n'+s.replace('defined(__arm__)',
            '(defined(__arm__) || defined(__aarch64__) || defined(__arm64__))'))

    source_copy('Source/common/sakura/ssys_thread.cpp', lambda s:
        '#include "platform/sdl/synchronization.h"\n'+s.replace('gettid()', 'study::platform::sdl::CurrentThreadId()'))
    source_copy('Source/common/sakura/ssys_heap_memory.cpp', lambda s:
        s.replace('#include <malloc.h>', '#include <cstdlib>')
         .replace('sizePageUnit = PAGE_SIZE', 'sizePageUnit = sizeMinPage'))
    source_copy('Source/common/sakuragl/media/sgl_sound_player.cpp', lambda s:
        s.replace('#error no implement SGLSoundPlayer::Open',
                  '// SDL startup registers a real SGLSoundPlayerInterface creator.'))
    source_copy('Source/common/sakuragl/media/sgl_sound_recorder.cpp', lambda s:
        s.replace('#error no implement SGLSoundRecorder::EnumerateDevices',
                  'if (pwNameBuf && nNameBufLength) pwNameBuf[0] = 0;')
         .replace('#error no implement SGLSoundRecorder::Open',
                  'return sglErrNotSupported; // Recording is not part of this backend.'))
    def font(s):
        constructor = 'SGLFont::SGLFont( void )\n{\n\tm_pFont = NULL ;\n}'
        if s.count(constructor) != 1: raise ValueError('SDK default font constructor changed')
        s = s.replace(constructor, 'SGLFont::SGLFont( void )\n{\n\tm_pFont = NULL ;\n\tm_flagOwner = false ;\n}')
        old = '''\t\t\tm_pFont = pFont->NewFont( style ) ;
\t\t\tif ( m_pFont != NULL )
\t\t\t{
\t\t\t\tm_flagOwner = true ;
\t\t\t\treturn\tsglErrSuccess ;
\t\t\t}'''
        if s.count(old) != 1: raise ValueError('SDK stock font ownership block changed')
        # This branch already released QuickLock. Failed stock creation must
        # not fall through into the second unlock and a missing system backend.
        return s.replace(old, old + '\n\t\t\treturn sglErrFailed;')
    source_copy('Source/common/sakuragl/sgl2d/sgl_font.cpp', font)
    source_copy('Source/common/glscs/glscs_sakura2_jit_native_compiler.cpp', lambda s:
        s.replace('sizePageUnit = PAGE_SIZE', 'return nullptr'))
    def std_application(s):
        s = '#include <SDL3/SDL.h>\n#include <string>\n' + s
        s = s.replace('#error\tno implement SGLStdApplication::GetUserUniqueId',
            'Trace("SDL: machine identity is unavailable\\n");\n\treturn SString();')
        return replace_body(s, 'SGLStdApplication::EnumerateEnvironmentVariableNames', '''
lstVarNames.RemoveAll();
char** variables = SDL_GetEnvironmentVariables(SDL_GetEnvironment());
if (!variables) return;
for (size_t i = 0; variables[i]; ++i) {
    const char* end = strchr(variables[i], '=');
    if (!end) continue;
    std::string name(variables[i], size_t(end - variables[i]));
    auto* value = new SString;
    value->FromUTF8(reinterpret_cast<const uint8_t*>(name.c_str()));
    lstVarNames.Add(value);
}
SDL_free(variables);
''')
    source_copy('Source/common/sakuraglx/sglx_std_app.cpp', std_application)
    loquaty_file = ROOT/'vendor/official-loquaty/Loquaty/source/loquaty_file.cpp'
    loquaty_text = loquaty_file.read_text(encoding='utf-8-sig')
    loquaty_text = loquaty_text.replace('lseek64', 'lseek').replace('off64_t', 'off_t')
    write(out/'loquaty/loquaty_file.cpp', loquaty_text)
    records.append({'source': str(loquaty_file.relative_to(ROOT)),
        'sha256': hashlib.sha256(loquaty_file.read_bytes()).hexdigest(),
        'output': 'loquaty/loquaty_file.cpp'})

    # The original backend uses operating-system macros to select graphics APIs
    # and WGL ownership. SDL owns contexts on every OS; GLES is a separate choice.
    def graphics(s):
        return (s.replace('__PLATFORM_ANDROID__', 'STUDYSTEADY_GL_ES')
                 .replace('ANDROID_API_LEVEL', 'STUDYSTEADY_GL_API_LEVEL')
                 .replace('__PLATFORM_WINDOWS__', 'STUDYSTEADY_SDK_WGL'))
    for base in ('Include/opengl', 'Source/opengl'):
        for src in sorted((COTOPHA/base).rglob('*')):
            if src.suffix not in ('.h', '.hpp', '.cpp'): continue
            relative = src.relative_to(COTOPHA)
            source_copy(relative, graphics)

    producer = out/'Source/opengl/sakuragl/sgl_opengl_window_producer.cpp'
    text = generated[producer]
    old = '#if\tdefined(STUDYSTEADY_GL_ES)\n\tGLInitializeProcedure'
    if text.count(old) != 1: raise ValueError('SDK context initialization branch changed')
    # A host-supplied context applies equally to desktop GL and mobile GLES.
    text = text.replace(old, '#if !defined(STUDYSTEADY_SDK_WGL)\n\tGLInitializeProcedure')
    generated[producer] = text

    # Texture storage is GLES 3.0 core; compute is GLES 3.1. The SDK groups
    # them under one Android API guard, duplicating GLES3 core declarations.
    ext_header = out/'Include/opengl/sakuragl/sgl_opengl_extension.h'
    text = generated[ext_header]
    old_pointer_types = '''\t#if\tdefined(__PROCESSOR_INTEL_X86_64__)
//\t\ttypedef int64_t\t\tptrdiff_t ;
\t#else
\t\ttypedef int\t\t\tptrdiff_t ;
\t#endif
\ttypedef ptrdiff_t GLintptr;
\ttypedef ptrdiff_t GLsizeiptr;'''
    if text.count(old_pointer_types) != 1: raise ValueError('SDK GL pointer-sized types changed')
    text = '#include <cstddef>\n' + text.replace(old_pointer_types,
        '\ttypedef std::ptrdiff_t GLintptr;\n\ttypedef std::ptrdiff_t GLsizeiptr;')
    for name in ('glTexStorage2D', 'glTexStorage3D'):
        line = next(line for line in text.splitlines() if 'extern' in line and line.rstrip().endswith(name+' ;'))
        text = text.replace(line, '#if !defined(STUDYSTEADY_GL_ES) || (STUDYSTEADY_GL_API_LEVEL < 18)\n'+line+'\n#endif')
    generated[ext_header] = text

    original_header = (COTOPHA/'Include/opengl/sakuragl/sgl_opengl_extension.h').read_text(encoding='utf-8-sig')
    import re
    names = sorted(set(re.findall(r'API_(gl\w+)', original_header)))
    # SDL's desktop header declares more than OpenGL 1.1. Hide declarations
    # which the SDK intentionally loads into its own OpenGLExtension namespace.
    desktop_header = '#pragma once\n' + ''.join(f'#define {n} StudySDLUnusedDeclaration_{n}\n' for n in names)
    desktop_header += '#include <SDL3/SDL_opengl.h>\n'
    desktop_header += ''.join(f'#undef {n}\n' for n in names)
    write(out/'include/GL/gl.h', desktop_header)
    write(out/'include/GL/glu.h', '''#pragma once
#include <GL/gl.h>
extern "C" {
const GLubyte* gluErrorString(GLenum error);
GLint gluBuild2DMipmaps(GLenum target, GLint internalFormat, GLsizei width,
                      GLsizei height, GLenum format, GLenum type, const void* pixels);
}
''')
    extension = out/'Source/opengl/sakuragl/sgl_opengl_extension.cpp'
    text = generated[extension]
    old = '#define\tGLEX_LOAD_GLAPI(x)\t\tapi_supported = false'
    if text.count(old) != 1: raise ValueError('SDK GL loader fallback changed')
    text = '#include <SDL3/SDL.h>\n'+text.replace(old, '''#define GLEX_LOAD_GLAPI(x) \\
    OpenGLExtension::x = reinterpret_cast<OpenGLExtension::API_##x>(SDL_GL_GetProcAddress(#x)); \\
    if (!OpenGLExtension::x) api_supported = false''')
    for name in ('glTexStorage2D', 'glTexStorage3D'):
        declaration = 'GLEX_DEFINE_GLAPI('+name+') ;'
        text = text.replace(declaration, '#if !defined(STUDYSTEADY_GL_ES) || (STUDYSTEADY_GL_API_LEVEL < 18)\n'+declaration+'\n#endif')
    generated[extension] = text
    for target, content in generated.items():
        if target == out/'Source/opengl/sakuragl/sgl_opengl_context.cpp':
            content = transform_context(content)
        write(target, content)
    write(out/'source-manifest.json', json.dumps(records, indent=2)+'\n')


if __name__ == '__main__':
    main()
