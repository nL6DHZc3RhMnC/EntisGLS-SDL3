#!/usr/bin/env python3
"""Reuse SDK window semantics with a project-owned SDL platform boundary."""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT

import argparse
import hashlib
from pathlib import Path
import re
from entis_sdk import COTOPHA


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def prepare(output):
    header_source = COTOPHA / 'Include/android/sakuragl/sgl_generic_window.h'
    source = COTOPHA / 'Source/android/sakuragl/sgl_generic_window.cpp'
    header = header_source.read_text(encoding='utf-8-sig')
    header = header.replace('#include <esl/esl_java_object.h>', '#include <SDL3/SDL.h>')
    header = header.replace(', public JNI::JavaObject', '')
    header = re.sub(r'\s*JNI::JSmartObject\s+m_jsobjMenu\s*;', '', header)
    header = header.replace('ESL_DECLARE_CLASS_INFO2\n\t\t\t( SGLGenericWindow, SGLAbstractWindow, JavaObject )',
                            'ESL_DECLARE_CLASS_INFO( SGLGenericWindow, SGLAbstractWindow )')
    start = header.index('\tprotected:\t// Java')
    end = header.index('\tpublic:', start)
    header = header[:start] + header[end:]
    start = header.index('\t\tstatic const int\tg_joyButtonFromAndroidKeyCode')
    end = header.index('\t\tint64_t GetJoyButtonMask', start)
    header = header[:start] + '\tpublic:\n' + header[end:]
    header = header.replace('void OnDraw( void ) ;',
        'void OnDraw( void ) ;\n\t\tvoid OnSDLEvent(const SDL_Event& event);\n\t\tvoid DrawSDLFrame();')
    text = source.read_text(encoding='utf-8-sig')
    text = text.replace('ESL_IMPLEMENT_CLASS_INFO2\n\t( SakuraGL::SGLGenericWindow, SGLAbstractWindow, JavaObject )',
                        'ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLGenericWindow, SGLAbstractWindow )')
    replaced = [
        'CreateDisplay', 'SetOptionalFlags', 'ChangeCooperationLevel',
        'PostUpdate', 'UpdateWindow', 'PostRenderingThread', 'PostUIThread', 'IsWindowActive',
        'SetWindowCaption', 'ShowCursor', 'IsShowCursor', 'SetCursor',
        'GetMonitorFrequency', 'AttachMenu', 'CaptureMouse', 'ReleaseMouse',
        'MoveCursorPosition', 'EnableChangePhysicalMode',
        'CreateWindowSimply', 'CloseWindowSimply', 'OnKeyDown', 'OnKeyUp',
        'java_EntisGLS_getMainSurfaceView', 'java_EntisGLS_postUpdateView',
        'java_EntisGLS_callNativeOnUIThread', 'java_EntisGLS_callNativeOnRenderingThread',
        'java_EntisGLS_callNativeOnAsyncNoRenderingThread']
    for name in replaced:
        pattern = r'(?ms)^[\w:*& \t]+\s+SGLGenericWindow::' + name + r'\s*\([^)]*\)\s*\{.*?^\}'
        text, count = re.subn(pattern, '', text)
        if count != 1:
            raise ValueError(f'{name}: expected one SDK definition, found {count}')
    text = text[:text.index('const int\tSGLGenericWindow::g_joyButtonFromAndroidKeyCode')]
    for forbidden in ('JNI::', 'JavaObject', 'jobject', 'jmethodID'):
        if forbidden in text or forbidden in header:
            raise ValueError(f'Unexpected platform dependency remains: {forbidden}')
    text += '\n#include "platform/sdl/window_implementation.inc"\n'
    provenance = '// Generated from supplied SDK SHA-256 '
    write(output/'include/sakuragl/sgl_generic_window.h',
          provenance + hashlib.sha256(header_source.read_bytes()).hexdigest() + '\n' + header)
    write(output/'source/sgl_generic_window.cpp',
          provenance + hashlib.sha256(source.read_bytes()).hexdigest() + '\n' + text)


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    prepare(p.parse_args().output)
