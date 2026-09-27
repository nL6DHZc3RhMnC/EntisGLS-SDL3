#!/usr/bin/env python3
"""Generate SDL platform copies of official SSystem sources without editing SDK inputs.

The original initialization, finalization, global state, atomic operations and
platform-independent UI models are retained. Only named platform entry points
are replaced. Unsupported native dialogs report errNotSupported explicitly.
"""

# Support direct execution from any working directory.
import sys as _sys
from pathlib import Path as _BootstrapPath
_sys.path.insert(0, str(next(parent / "tools" for parent in _BootstrapPath(__file__).resolve().parents
                             if (parent / "tools" / "_bootstrap.py").is_file())))
from _bootstrap import ROOT


import argparse
import hashlib
import json
from pathlib import Path
import re

from entis_sdk import COTOPHA


def function_body(source, name):
    pattern = r"\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*(?:const\s*)?\{"
    matches = list(re.finditer(pattern, source))
    if len(matches) != 1:
        raise RuntimeError(f"Expected exactly one {name}, found {len(matches)}")
    start = matches[0].end() - 1
    depth = 0
    # Ignore braces in C++ strings, character literals and comments.
    token = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/|[{}]', re.S)
    for match in token.finditer(source, start):
        if match.group() == "{":
            depth += 1
        elif match.group() == "}":
            depth -= 1
            if not depth:
                return start, match.end()
    raise RuntimeError(f"Unclosed function body: {name}")


def replace_body(source, name, body):
    start, end = function_body(source, name)
    return source[:start] + "{\n" + body.strip() + "\n}" + source[end:]


def replace_once(source, old, new):
    if source.count(old) != 1:
        raise RuntimeError(f"Expected one controlled replacement for {old[:100]!r}")
    return source.replace(old, new, 1)


def unsupported(name):
    return f'SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "{name}: not supported by the SDL backend yet");\nreturn errNotSupported;'


def stdapi(source):
    start = source.index("#include <esl/esl_java_object.h>")
    end = source.index("using namespace SSystem")
    source = source[:start] + '''#include "platform/sdl/system.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>

''' + source[end:]
    start, end = function_body(source, "SSystem::Initialize")
    body = source[start + 1:end - 1]
    scheme_start = body.index("\t\tSFileOpener::SetDefaultOpener")
    # The remainder of the original function is exclusively Android file-scheme
    # registration. The shared backend preserves those schemes with caller paths.
    body = "\n    study::platform::sdl::RequireSystemPaths();\n" + body[:scheme_start] + "    study::platform::sdl::RegisterSystemSchemes();\n    }\n"
    source = source[:start] + "{" + body + "}" + source[end:]
    source = replace_once(source, "SSystem::eslHeapUninitialize() ;", "SSystem::eslHeapUninitialize() ;\n        study::platform::sdl::ReleaseSystemPaths();")
    replacements = {
        "SSystem::GetMemoryStatus": "study::platform::sdl::FillMemoryStatus(mstatus);",
        "SSystem::ResetCurrentMilliSec": "g_nLastTime = SDL_GetTicks() - timeStart;",
        "SSystem::CurrentMilliSec": "return SDL_GetTicks() - g_nLastTime;",
        "SSystem::GetPerformanceCounter": "return static_cast<int64_t>(SDL_GetPerformanceCounter());",
        "SSystem::GetPerformanceFrequency": "return static_cast<int64_t>(SDL_GetPerformanceFrequency());",
        "SSystem::SleepMilliSec": "if (msec >= 0) SDL_Delay(static_cast<Uint32>(msec));",
        "SSystem::CurrentLocalDate": "study::platform::sdl::FillLocalDate(date);",
        "SSystem::DifferenceInLocalTime": "return study::platform::sdl::LocalTimeDifference(pwszName, nNameCapacity);",
        "SSystem::GetPlatformInformation": "study::platform::sdl::FillPlatformInformation(pi);",
        "SSystem::GetCPUFamily": "return static_cast<CPU_Family>(study::platform::sdl::CPUFamily());",
        "SSystem::GetCPUFeatures": "return study::platform::sdl::CPUFeatures();",
        "SSystem::GetLogicalProcessorCount": "return static_cast<unsigned int>(SDL_GetNumLogicalCPUCores());",
        "SSystem::GetModuleExportFunction": "return static_cast<ulong_ptr_t>(study::platform::sdl::ResolveSymbol(pszFuncName, pszReserved));",
        "SSystem::Trace": '''va_list arguments;
va_start(arguments, pszTrace);
SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, pszTrace, arguments);
va_end(arguments);''',
        "SSystem::Assert": '''SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EntisGLS assertion %s(%d): %s", pszFile, nLineNum, pszExpr);''',
        "SSystem::MessageBox": "return study::platform::sdl::ShowMessageBox(pwszMsg, pwszCaption, nStyles);",
    }
    for name, body in replacements.items():
        source = replace_body(source, name, body)
    return source


def std_ui(source):
    source = replace_once(source, "#include <sakura/sakura.h>", "#include <sakura/sakura.h>\n#include <SDL3/SDL.h>")
    source = replace_once(source, "#include <sakuragl/sgl_window.h>", "// Native dialogs do not depend on the renderer/window implementation.")
    source = replace_body(source, "SSystem::OpenShellFile", '''if (pResult) { pResult->nFlags = 0; pResult->nExitCode = 0; }
if (!pwszURI) return errInvalidParam;
if (actShell != shellOpenURI || pwszAppPath || pwszAppPlacement || nFlags)
    return errNotSupported;
const auto bytes = SString(pwszURI).ToUTF8();
SArray<char> uri;
uri.SetLength(bytes.GetLength() + 1);
if (bytes.GetLength()) eslCopyMemory(uri.GetArray(), bytes.GetConstArray(), bytes.GetLength());
uri.GetArray()[bytes.GetLength()] = 0;
if (!SDL_OpenURL(uri.GetConstArray())) return errFailed;
if (pResult) pResult->nFlags = shellResultSuccess;
return errSuccess;''')
    for name in ["SSystem::ActivateWindow", "SSystem::BrowseDirectoryDialog",
                 "SSystem::BrowseOpenFileDialog", "SSystem::BrowseSaveFileDialog",
                 "SSystem::MessageEditBox", "SProgressiveDialog::Create",
                 "SProgressiveDialog::Close", "SProgressiveDialog::SetCaption",
                 "SProgressiveDialog::SetMessage", "SProgressiveDialog::SetStatus",
                 "SCustomDialog::DoModal"]:
        source = replace_body(source, name, unsupported(name))
    source = replace_body(source, "SProgressiveDialog::IsCanceled", '''// Creation is unsupported, so there is no active native progress dialog.
return true;''')
    # These branches run only while a platform custom dialog is modal. DoModal
    # above cannot enter that state; report unsupported if a caller forces it.
    source, count = re.subn(r'^[ \t]*#error[ \t]+no implement[^\n]*$',
        '        throw SException(errNotSupported, L"Native custom dialog operation is not supported by the SDL backend");',
        source, flags=re.M)
    if count != 6:
        raise RuntimeError(f"Expected 6 unsupported custom-dialog branches, found {count}")
    return source


def files(source):
    source = replace_body(source, "SFile::GetDefaultDirectory", '''return static_cast<SError>(
    study::platform::sdl::ResolveDefaultDirectory(strDirPath, pwszPlacementId, pwszOption));''')
    # Darwin uses a 64-bit POSIX off_t and has no Linux-specific lseek64 alias.
    # Keep a build-time guarantee so a future 32-bit target cannot truncate NOA
    # offsets silently; such targets must enable _FILE_OFFSET_BITS=64.
    for old, new, expected in [("off64_t", "off_t", 4), ("lseek64", "lseek", 5)]:
        source, count = re.subn(r"\b" + old + r"\b", new, source)
        if count != expected:
            raise RuntimeError(f"Expected {expected} occurrences of {old}, found {count}")
    source = replace_once(source, "#include <sys/stat.h>",
        '#include <sys/stat.h>\nstatic_assert(sizeof(off_t) >= 8, "SDL file backend requires 64-bit file offsets");')
    # The original Android path uses Java consoles instead of these POSIX
    # branches. Use the correct standard descriptors and own duplicates so
    # engine finalization cannot close the host application's stdin/stdout.
    source = replace_once(source, "m_fdFile = 0 ;", "m_fdFile = dup(STDOUT_FILENO) ;\n                if (m_fdFile < 0) return errFailed;")
    source = replace_once(source, "m_fdFile = 1 ;", "m_fdFile = dup(STDIN_FILENO) ;\n                if (m_fdFile < 0) return errFailed;")
    source = replace_once(source,
        "mode_t\tmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;",
        "mode_t\tmode = S_IRWXU ; // Newly created private directories must be searchable.")
    return source


def prepare(sdk, output):
    if output.resolve() == sdk.resolve() or sdk.resolve() in output.resolve().parents:
        raise ValueError("Generated sources must be outside the read-only SDK")
    output.mkdir(parents=True, exist_ok=True)
    entries = [
        ("Source/android/sakura/ssys_stdapi.cpp", "ssys_stdapi.cpp", stdapi),
        ("Source/common/sakura/ssys_std_ui.cpp", "ssys_std_ui.cpp", std_ui),
        ("Source/common/sakura/ssys_file.cpp", "ssys_file.cpp", files),
    ]
    manifest = {"description": "Generated SDL replacements; official SDK source files are read-only", "sources": []}
    for relative, name, convert in entries:
        path = sdk / relative
        raw = path.read_bytes()
        content = raw.decode("utf-8-sig").replace("\r\n", "\n")
        # Load C++ standard headers before the SDK's historical <stdatomic.h>
        # include, which otherwise shadows libc++'s C++17 atomic declarations.
        result = ('// GENERATED by tools/sdk/prepare_sdl_system.py; do not edit.\n'
                  '#include "platform/sdl/system.h"\n#include <cstdarg>\n' + convert(content))
        destination = output / name
        if not destination.exists() or destination.read_text() != result:
            destination.write_text(result)
        manifest["sources"].append({"input": relative, "input_sha256": hashlib.sha256(raw).hexdigest(),
            "output": name, "output_sha256": hashlib.sha256(result.encode()).hexdigest()})
    manifest_path = output / "system-source-manifest.json"
    manifest_text = json.dumps(manifest, indent=2) + "\n"
    if not manifest_path.exists() or manifest_path.read_text() != manifest_text:
        manifest_path.write_text(manifest_text)
    return [output / name for _, name, _ in entries]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, default=COTOPHA)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    for path in prepare(args.sdk, args.output):
        print(path)


if __name__ == "__main__":
    main()
