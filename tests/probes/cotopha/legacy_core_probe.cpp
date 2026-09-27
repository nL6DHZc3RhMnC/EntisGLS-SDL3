#include "compatibility/sdk/legacy/gls.h"
#include "legacy_core_probe.h"
#include "runtime/cotopha_port/legacy_script_file.h"
#include "runtime/cotopha_port/legacy_thread.h"
#include "legacy_native_binding_probe.h"
#include "legacy_context_probe.h"
#include "platform/log.h"
#include <array>
#include <cstring>
#include <memory>
#include <thread>

namespace {
bool Check(bool success, const char* detail) {
    if (!success) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
        "Legacy core probe FAIL: %s", detail);
    return success;
}
void Stage(const char* name) {
    study::platform::LogPrint(study::platform::LogPriority::Info, "StudySteady", "Legacy core probe: %s", name);
}
bool ThreadStep(ECSThread& thread, const char* step, ESLError error) {
    study::platform::LogPrint(error ? study::platform::LogPriority::Error : study::platform::LogPriority::Info, "StudySteady",
        "Legacy core thread %s: result=%s status=%d suspend_count=%u running=%d ip=0x%08x",
        step, GetESLErrorMsg(error), int(thread.GetContext().GetStatus()),
        thread.GetSuspendCount(), thread.IsThreadRunning(), thread.GetContext().m_ip);
    return Check(error == eslErrSuccess, step);
}
class ProbeImage final : public ECSExecutionImage {
public:
    ProbeImage() {
        // obj.enter ""(); obj.load int32(42); obj.ex.return 1;
        // at offset 18: obj.jump -5 (cooperative suspend/abort test loop).
        static const BYTE code[] = {
            4, 0,0,0,0, 0,0,0,0,
            2,0,4,42,0,0,0, 18,1,
            6,0xfb,0xff,0xff,0xff,
            // offset 23: global[0] = 7 using the real 0xff plain-store byte.
            4,0,0,0,0,0,0,0,0,
            2,3,4,0,0,0,0,
            2,0,4,7,0,0,0,
            3,0xff,1,18,0
        };
        std::memset(&m_exiHeader, 0, sizeof(m_exiHeader));
        m_exiHeader.nVersion = 1;
        m_exiHeader.nIntBase = 64;
        m_exiHeader.nStackSize = 4096;
        m_exiHeader.nHeapSize = 4096;
        m_exiHeader.fnEntryPoint = 0;
        m_exiHeader.fnStaticInitialize = UINT32_MAX;
        m_exiHeader.fnResumePrepare = UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)), code, sizeof(code));
        m_bufImage.Flush(sizeof(code));
        m_pImage = static_cast<BYTE*>(m_bufImage.ModifyBuffer(0, sizeof(code)));
        m_dwImageSize = sizeof(code);
    }
};
class FailingSaveFile final : public EMemoryFile {
    unsigned writes_ = 0;
public:
    unsigned long Write(const void* bytes, unsigned long count) override {
        // Allow the container header, then inject an I/O failure after Save has
        // notified the live script threads that a snapshot is starting.
        if (++writes_ > 2) return 0;
        return EMemoryFile::Write(bytes, count);
    }
};
struct ContextScope {
    ECSContext& context;
    ECSContext* previousPrimary = ECotophaScript::GetPrimaryContext();
    bool initialized = false;
    ~ContextScope() {
        if (initialized) {
            context.ReleaseContext(true);
            // Releasing an already empty context must not double-free VA pages.
            context.ReleaseContext(false);
        }
        ECotophaScript::SetPrimaryContext(previousPrimary);
    }
};
struct CurrentThreadScope {
    ECSThread* previous = ECotophaScript::GetCurrentThread();
    explicit CurrentThreadScope(ECSThread* current) { ECotophaScript::SetCurrentThread(current); }
    ~CurrentThreadScope() { ECotophaScript::SetCurrentThread(previous); }
};
bool IsExpectedString(ECSObject* value, const wchar_t* expected) {
    auto* string = ESLTypeCast<ECSString>(value);
    return string && string->m_varStr == expected;
}
}

bool CheckLegacyCore(ECSEnvironment& environment) {
    Stage("initialize empty image");
    ProbeImage image;
    image.AttachCSEnvironment(&environment);
    ECSContext context;
    ContextScope release{context};
    if (!Check(context.InitializeContext(&image) == eslErrSuccess, "empty image/context initialize")) return false;
    release.initialized = true;
    if (!CheckLegacyNativeBinding(context)) return false;
    if (!CheckLegacyProcessorState(context)) return false;

    Stage("reference null transitions and object ownership");
    {
        ECSReference first, second;
        for (int attempt = 0; attempt < 3; ++attempt) {
            std::unique_ptr<ECSInteger> target(new ECSInteger(123));
            first.SetReference(nullptr, &context);
            first.SetReference(target.get(), &context);
            second.SetReference(target.get(), &context);
            if (!Check(first.m_pRef == target.get() && second.m_pRef == target.get() && target->m_pBackRef,
                "reference null-to-object backlinks")) return false;
            first.SetReference(nullptr, &context);
            if (!Check(!first.m_pRef && second.m_pRef == target.get() && target->m_pBackRef,
                "clearing one reference retains the other")) return false;
            second.SetReference(nullptr, &context);
            if (!Check(!second.m_pRef && !target->m_pBackRef, "clearing last reference removes backlinks")) return false;
            first.SetReference(target.get(), &context);
            target.reset();
            if (!Check(!first.m_pRef, "object destruction clears non-owning reference")) return false;
            first.SetOwnObject(new ECSInteger(456));
            if (!Check(first.m_pRef != nullptr, "owned object assignment")) return false;
            first.SetOwnObject(nullptr);
            first.SetOwnObject(nullptr);
            if (!Check(!first.m_pRef, "owned object repeated null release")) return false;
        }
    }

    Stage("plain assignment opcode 0xff");
    ECSInteger* assigned = new ECSInteger(0);
    image.m_csgGlobal.AddVariable(L"probeAssigned", assigned);
    ECSObjArray<ECSObject> noArguments;
    if (!Check(!context.CallFunction(DWORD(23), noArguments) && assigned->GetValue() == 7,
        "obj.store 0xff performs assignment")) return false;

    Stage("object arithmetic and UTF-16 object serialization");
    ECSInteger number(INT64(0x100000000LL)), increment(7);
    if (!Check(!number.Operate(context, csotAdd, increment) &&
        number.GetValue() == INT64(0x100000007LL), "64-bit integer addition")) return false;
    ECSInteger zero(0);
    if (!Check(number.Operate(context, csotDiv, zero) != eslErrSuccess,
        "division by zero reports an error")) return false;
    const wchar_t* text = L"日本語\U0001f642";
    ECSString original(text);
    Stage("naked UTF-16 memory bridge");
    ECSSakura2Processor::LinearAddressCache stringSegment;
    if (!Check(original.GetSegmentBuffer(stringSegment) && stringSegment.limitSegment == 12 &&
        stringSegment.pbytBuffer[6] == 0x3d && stringSegment.pbytBuffer[7] == 0xd8 &&
        stringSegment.pbytBuffer[8] == 0x42 && stringSegment.pbytBuffer[9] == 0xde,
        "String segment exposes UTF-16 surrogate pair")) return false;
    EWideString nakedString;
    if (!Check(context.AtomicLoadString(nakedString, &original, 0) && nakedString == text &&
        context.AtomicLoadString(nakedString, &original, 6, 2) && nakedString == L"\U0001f642" &&
        !context.AtomicLoadString(nakedString, &original, 10, 2),
        "naked UTF-16 loading uses byte bounds")) return false;
    ECSString edited(L"old");
    if (!Check(edited.LockLegacyUtf16(5) && edited.GetSegmentBuffer(stringSegment),
        "lock UTF-16 String buffer")) return false;
    const BYTE editedUtf16[] = {0x2d,0x4e,0x3d,0xd8,0x42,0xde,0,0}; // 中🙂
    std::memcpy(stringSegment.pbytBuffer, editedUtf16, sizeof(editedUtf16));
    if (!Check(edited.UnlockLegacyUtf16(-1) && edited.m_varStr == L"中\U0001f642",
        "unlock commits UTF-16 String writes")) return false;
    BYTE* writableString = static_cast<BYTE*>(edited.GetBuffer(0, 2, true));
    if (!Check(writableString != nullptr, "String object writable buffer")) return false;
    writableString[0] = 'A'; writableString[1] = 0;
    edited.FlushBuffer(0, 2, writableString, true);
    if (!Check(edited.m_varStr == L"A\U0001f642", "String FlushBuffer commits writes")) return false;

    Stage("UTF-16 object serialization");
    EMemoryFile serialized;
    if (!Check(!serialized.Create(8) && !context.SaveObject(serialized, &original),
        "save UTF-16 object")) return false;
    const auto* wire = static_cast<const BYTE*>(serialized.GetBuffer());
    // Type byte + uint32 UTF-16-unit count; Japanese(3) + surrogate pair(2).
    if (!Check(serialized.GetLength() >= 19 && wire[0] == csvtString &&
        wire[1] == 5 && wire[2] == 0 && wire[3] == 0 && wire[4] == 0,
        "saved string uses five UTF-16 units")) return false;
    serialized.Seek(0, ESLFileObject::FromBegin);
    ECSObject* loadedRaw = nullptr;
    if (!Check(!context.LoadObject(serialized, loadedRaw), "load UTF-16 object")) return false;
    std::unique_ptr<ECSObject> loaded(loadedRaw);
    if (!Check(IsExpectedString(loaded.get(), text), "UTF-16 object content roundtrip")) return false;

    Stage("script File binary and compressed object roundtrip");
    ECSFile file;
    ECSInteger integer(INT64(0x11223344)), readInteger;
    if (!Check(!file.CreateMemoryFile(2), "create File memory")) return false;
    file.SetCharacterEncoding("utf-16");
    if (!Check(file.WriteBinary(integer, 0) == 4 && file.WriteBinary(original, 0) == 10,
        "File UTF-16 binary byte length")) return false;
    file.Seek(0, ESLFileObject::FromBegin);
    ECSString readString;
    if (!Check(file.ReadBinary(readInteger, 0) == 4 && readInteger.GetValue() == integer.GetValue() &&
        file.ReadBinary(readString, 10) == 10 && readString.m_varStr == text,
        "File binary readback")) return false;
    if (!Check(!file.CreateMemoryFile(16) && !file.SaveObject(original, nullptr, context),
        "File compressed SaveObject")) return false;
    file.Seek(0, ESLFileObject::FromBegin);
    ECSObject* decompressedRaw = nullptr;
    ESLError loadError = file.LoadObject(decompressedRaw, context);
    if (loadError) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
        "Legacy core probe compressed LoadObject detail: %s", GetESLErrorMsg(loadError));
    if (!Check(!loadError, "File compressed LoadObject")) return false;
    std::unique_ptr<ECSObject> decompressed(decompressedRaw);
    if (!Check(IsExpectedString(decompressed.get(), text), "File ERISAN object content")) return false;
    if (!Check(!file.Close() && !file.Close(), "File repeated Close")) return false;

    Stage("script File NOA entry");
    if (!Check(!file.Open(L"$(CURRENT)\\script.noa", ESLFileObject::modeRead | ESLFileObject::shareRead, &context) &&
        !file.OpenArchive() && !file.OpenArchiveFile("script.csx", "", false), "File NOA open/decode")) return false;
    std::array<BYTE,8> signature{};
    if (!Check(file.Read(signature.data(), signature.size()) == signature.size() &&
        !std::memcmp(signature.data(), "Entis\x1a\0\0", 8), "File NOA CSX signature")) return false;
    if (!Check(!file.CloseArchiveFile() && !file.CloseArchive() && !file.Close() && !file.Close(),
        "File NOA close sequence")) return false;

    Stage("event wake, reset and repeated deletion");
    ECSThreadEvent event;
    if (!Check(!event.CreateEvent(false) && event.WaitEvent(0, context) == eslErrTimeout,
        "unsignalled event times out")) return false;
    std::thread signal([&] { ::Sleep(10); event.SetEvent(); });
    ESLError waited = event.WaitEvent(2000, context);
    signal.join();
    if (!Check(waited == eslErrSuccess && event.WaitEvent(0, context) == eslErrSuccess,
        "manual-reset event wakes and stays signalled")) return false;
    event.ResetEvent();
    if (!Check(event.WaitEvent(0, context) == eslErrTimeout && !event.DeleteEvent() && !event.DeleteEvent(),
        "event reset and repeated DeleteEvent")) return false;

    Stage("thread executes object bytecode, restarts, suspends and aborts");
    ECSThread thread(context);
    ECSObjArray<ECSObject> arguments;
    for (int attempt = 0; attempt < 2; ++attempt) {
        if (!Check(!thread.BeginThread(DWORD(0), arguments), "start object thread")) return false;
        if (!Check(::WaitForSingleObject(thread.Handle(), 2000) == WAIT_OBJECT_0,
            "object thread completion")) return false;
        auto* result = ESLTypeCast<ECSInteger>(thread.GetContext().m_pRetObj);
        if (!Check(result && result->GetValue() == 42 && !thread.IsThreadRunning(),
            "thread returned integer 42")) return false;
        if (!Check(!thread.CloseThread(), "join completed thread")) return false;
    }
    if (!ThreadStep(thread, "begin loop", thread.BeginThread(18, arguments))) return false;
    if (!ThreadStep(thread, "suspend loop", thread.SuspendThread(2000))) return false;
    if (!ThreadStep(thread, "resume loop", thread.ResumeThread())) return false;
    {
        FailingSaveFile failed;
        failed.Create(4096);
        const auto saveError=context.Save(failed);
        const auto suspendCount=thread.GetSuspendCount();
        const bool running=thread.IsThreadRunning();
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
            "Legacy failed-save thread state: save=%s suspend=%u running=%d status=%d ip=0x%08x error=%s",
            GetESLErrorMsg(saveError),suspendCount,running,int(thread.GetContext().GetStatus()),
            thread.GetContext().m_ip,thread.GetContext().m_strErrMsg.CharPtr());
        if (!Check(saveError != eslErrSuccess && suspendCount == 0 && running,
                   "failed context save resumes the live script thread")) return false;
    }
    if (!ThreadStep(thread, "abort loop", thread.AbortThread(2000))) return false;
    if (!ThreadStep(thread, "repeat abort", thread.AbortThread(0))) return false;
    if (!Check(!thread.BeginThread(DWORD(0), arguments) &&
        ::WaitForSingleObject(thread.Handle(), 2000) == WAIT_OBJECT_0 && !thread.CloseThread(),
        "thread can restart after abort")) return false;

    Stage("recursive script mutex ownership");
    {
        CurrentThreadScope identity(&thread);
        ECSThreadMutex mutex;
        if (!Check(!mutex.CreateEvent(false) && !mutex.WaitEvent(1000, context) &&
            !mutex.WaitEvent(1000, context) && mutex.GetEventState() == 2,
            "recursive mutex acquire")) return false;
        mutex.ResetEvent();
        if (!Check(mutex.GetEventState() == 1, "mutex first release retains ownership")) return false;
        mutex.ResetEvent();
        if (!Check(mutex.GetEventState() == 0 && !mutex.DeleteEvent() && !mutex.DeleteEvent(),
            "mutex last release and repeated deletion")) return false;
    }
    Stage("PASS: object arithmetic, UTF-16/ERISAN serialization, File/NOA, event and thread lifecycle");
    return true;
}
