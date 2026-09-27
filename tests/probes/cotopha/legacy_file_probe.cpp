#include "runtime/cotopha_port/legacy_file.h"
#include "legacy_file_probe.h"
#include "platform/log.h"
#include <array>
#include <memory>

bool CheckLegacyFileBridge(SSystem::SEnvironmentInterface &environment) {
    EMemoryFile memory;
    const BYTE contents[] = {0x10, 0x20, 0x30, 0x40};
    BYTE actual[4] = {};
    if (memory.Create(1) || memory.Write(contents, 4) != 4 || memory.GetLength() != 4 ||
        memory.Seek(-2, ESLFileObject::FromEnd) != 2 || memory.Read(actual, 4) != 2 ||
        actual[0] != 0x30 || actual[1] != 0x40 || memory.Read(actual, 1) != 0) return false;
    std::unique_ptr<ESLFileObject> duplicate(memory.Duplicate());
    if (!duplicate || duplicate->Read(actual, 4) != 4 || std::memcmp(actual, contents, 4)) return false;
    memory.Seek(2, ESLFileObject::FromBegin);
    if (memory.SetEndOfFile() || memory.GetLength() != 2 || duplicate->GetLength() != 4) return false;
    memory.Seek(7, ESLFileObject::FromBegin);
    if (memory.SetEndOfFile() || memory.GetLength() != 7) return false;
    memory.Seek(3, ESLFileObject::FromBegin);
    if (memory.Read(actual, 4) != 4 || actual[0] || actual[1] || actual[2] || actual[3]) return false;

    auto *scriptFile = environment.NewOpenFile(L"script.csx", SSystem::SFileOpener::shareRead);
    if (!scriptFile) return false;
    LegacyFileAdapter script(scriptFile, ESLFileObject::modeRead | ESLFileObject::shareRead);
    std::array<BYTE, 64> header{};
    if (script.Read(header.data(), header.size()) != header.size() ||
        std::memcmp(header.data(), "Entis\x1a\0\0", 8) || script.GetLargeLength() != 2017818) return false;
    std::unique_ptr<ESLFileObject> scriptCopy(script.Duplicate());
    std::array<BYTE, 64> copyHeader{};
    if (!scriptCopy || scriptCopy->SeekLarge(0, ESLFileObject::FromBegin) != 0 ||
        scriptCopy->Read(copyHeader.data(), copyHeader.size()) != copyHeader.size() ||
        header != copyHeader || script.GetLargePosition() != 64) return false;

    auto *largeFile = SSystem::SFileOpener::DefaultNewOpenFile(
        L"storage://game/psb.noa", SSystem::SFileOpener::shareRead);
    if (!largeFile) return false;
    LegacyFileAdapter large(largeFile, ESLFileObject::modeRead | ESLFileObject::shareRead);
    const UINT64 length = large.GetLargeLength();
    if (length != 4752776654ULL || large.SeekLarge(-16, ESLFileObject::FromEnd) != length - 16 ||
        large.Read(header.data(), 16) != 16 || large.Read(header.data(), 1) != 0) return false;
    const wchar_t *scratch = L".port-file-bridge-check.tmp";
    auto *writable = environment.GetWritableFileOpener();
    if (!writable) return false;
    {
        auto *output = writable->NewOpenFile(scratch, SSystem::SFileOpener::modeCreate);
        if (!output) return false;
        LegacyFileAdapter save(output, ESLFileObject::modeCreate);
        if (save.Write(contents, sizeof(contents)) != sizeof(contents)) return false;
    }
    {
        auto *input = writable->NewOpenFile(scratch, SSystem::SFileOpener::shareRead);
        if (!input) return false;
        LegacyFileAdapter save(input, ESLFileObject::modeRead);
        if (save.Read(actual, sizeof(actual)) != sizeof(actual) || std::memcmp(actual, contents, 4)) return false;
    }
    if (writable->RemoveSubFile(scratch)) return false;
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady",
        "Legacy file bridge PASS: memory roundtrip, compressed CSX, independent duplicate, NOA seek beyond 4 GiB, save read/write");
    return true;
}
