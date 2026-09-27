#include "compatibility/sdk/legacy/gls.h"
#include "legacy_context_probe.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include "platform/log.h"
#include <cstring>
#include <vector>

bool CheckLegacyProcessorState(ECSContext& context) {
    auto check = [](bool value, const char* detail) {
        if (!value) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Processor state FAIL: %s", detail);
        return value;
    };
    EMemoryFile original;
    if (!check(!original.Create(4096) && !context.SaveProcessorContext(original), "original snapshot")) return false;
    auto restore = [&] {
        original.Seek(0, ESLFileObject::FromBegin);
        return !context.LoadProcessorContext(original) && !context.CommitLoadedProcessorContext();
    };
    for (unsigned i = 0; i < 256; ++i) context.m_regset[i].ui = 0x1122334455660000ull + i;
    context.m_ip = 23;
    context.delete_CSObject(context.m_pRetObj);
    context.m_pRetObj = context.new_CSInteger(0x123456789abcdefLL);
    EMemoryFile saved;
    if (!check(!saved.Create(4096) && !context.SaveProcessorContext(saved), "fixed record write")) { restore(); return false; }
    const auto* bytes = static_cast<const uint8_t*>(saved.GetBuffer());
    if (!check(saved.GetLength() > 2084 && StudySteadyLegacyWire::Read32(bytes) == 23 &&
        StudySteadyLegacyWire::Read64(bytes + 36 + 255 * 8) == 0x11223344556600ffull,
        "Win32 fixed offsets and 256 raw 64-bit registers")) { restore(); return false; }
    context.m_ip = 7; context.m_regset[255].ui = 0;
    saved.Seek(0, ESLFileObject::FromBegin);
    if (!check(!context.LoadProcessorContext(saved) && !context.CommitLoadedProcessorContext(), "fixed record read")) { restore(); return false; }
    INT64 number = 0;
    if (!check(context.m_ip == 23 && context.m_regset[255].ui == 0x11223344556600ffull &&
        context.m_pRetObj && !context.m_pRetObj->OperateInteger(number) && number == 0x123456789abcdefLL,
        "execution state and return object restored")) { restore(); return false; }
    EMemoryFile rewritten; rewritten.Create(4096);
    if (!check(!context.SaveProcessorContext(rewritten) && rewritten.GetLength() == saved.GetLength() &&
        !std::memcmp(rewritten.GetBuffer(), saved.GetBuffer(), saved.GetLength()), "byte-exact processor roundtrip")) { restore(); return false; }
    for (const size_t length : {size_t(0), size_t(12), size_t(2083), size_t(saved.GetLength() - 1)}) {
        EMemoryFile truncated; truncated.Create(4096); truncated.Write(saved.GetBuffer(), length); truncated.Seek(0, ESLFileObject::FromBegin);
        auto* oldReturn = context.m_pRetObj;
        if (!check(context.LoadProcessorContext(truncated) != eslErrSuccess && context.m_ip == 23 &&
            context.m_pRetObj == oldReturn && context.m_regset[255].ui == 0x11223344556600ffull,
            "truncation preserves live processor state")) { restore(); return false; }
    }
    auto corrupt = std::vector<uint8_t>(bytes, bytes + saved.GetLength());
    StudySteadyLegacyWire::Write32(corrupt.data() + 8, 99);
    EMemoryFile invalid; invalid.Create(4096); invalid.Write(corrupt.data(), corrupt.size()); invalid.Seek(0, ESLFileObject::FromBegin);
    if (!check(context.LoadProcessorContext(invalid) != eslErrSuccess && context.m_ip == 23,
               "invalid status rejected before commit")) { restore(); return false; }
    if (!check(restore(), "restore probe context")) return false;
    study::platform::LogWrite(study::platform::LogPriority::Info, "StudySteady", "Processor state PASS: Win32 fixed wire, 256 registers, return object, exact roundtrip, truncated/invalid state rollback");
    return true;
}
