#include <gls.h>
#include "compatibility/sdk/legacy/runtime_support.h"
#include <atomic>
#include <stdexcept>
#include "platform/log.h"

namespace {
LegacyPlatformObjectFactory platformFactory = nullptr;
std::atomic<unsigned> references{0};
ECSContext *primaryContext = nullptr;
thread_local ECSThread *currentThread = nullptr;
CRITICAL_SECTION referenceMutex;
std::atomic<bool> multithreadReference{false};
struct FunctionTable { ECSStrTagArray **list; const wchar_t **names; };
#define ENTRY(T) {&T::m_staFuncName, T::m_pwszFuncName}
FunctionTable functions[] = {ENTRY(ECSContext), ENTRY(ECSReference), ENTRY(ECSInteger),
    ENTRY(ECSReal), ENTRY(ECSString), ENTRY(ECSArray), ENTRY(ECSHash), ENTRY(ECSBuffer),
    ENTRY(ECSFile), ENTRY(ECSResource),
    ENTRY(ECSThread), ENTRY(ECSThreadEvent), ENTRY(ECSThreadMutex)};
#undef ENTRY
}
CRITICAL_SECTION ECotophaScript::g_csCotopha;
void LegacySetPlatformObjectFactory(LegacyPlatformObjectFactory factory) { platformFactory = factory; }
ECSObject *LegacyCreatePlatformObject(ECSContext &context, const wchar_t *name) {
    if (name && !EWideString::Compare(name, L"InputFilter")) return new ECSInputFilter;
    if (name && !EWideString::Compare(name, L"Window")) return new ECSWindow;
    if (name && !EWideString::Compare(name, L"Setup")) return new ECSSetup;
    if (name && !EWideString::Compare(name, L"MessageSprite")) return new ECSMessageSprite;
    if (name && !EWideString::Compare(name, L"EmoteSprite")) return new ECSEmoteSprite;
    if (name && !EWideString::Compare(name, L"SuperSprite")) return new ECSSuperSprite;
    if (name && !EWideString::Compare(name, L"ParticleSprite")) return new ECSParticleSprite;
    if (name && !EWideString::Compare(name, L"ToneFilter")) return new ECSToneFilter;
    if (name && !EWideString::Compare(name, L"AudioPlayer")) return new ECSAudioPlayer;
    if (name && !EWideString::Compare(name, L"MovieSprite")) return new ECSMovieSprite;
    if (name && !EWideString::Compare(name, L"EmoteDevice")) {
        try {
            if (auto* environment = context.GetEnvironment()) return new ECSEmoteDevice(*environment);
        } catch (const std::exception& error) {
            study::platform::LogPrint(study::platform::LogPriority::Error, "LegacyCotopha", "EmoteDevice owner creation: %s", error.what());
        }
        return nullptr;
    }
    if (name && !EWideString::Compare(name, L"ResourceManager")) return new ECSResourceManager;
    if (platformFactory) return platformFactory(context, name);
    study::platform::LogPrint(study::platform::LogPriority::Error, "LegacyCotopha", "Unimplemented platform object: %ls", name);
    return nullptr;
}
unsigned LegacyProcessorFeatures() { return 0; } // GLS3 feature mask is x86-only.
void LegacyLogScriptError(const char* message, unsigned instruction) {
    study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Cotopha script exception before catch: %s (ip=0x%08x)",
        message ? message : "unknown", instruction);
}

void ECotophaScript::Initialize(DWORD, HESLHEAP imported) {
    if (imported) throw std::invalid_argument("Imported Windows heaps are unsupported on Android");
    if (references.fetch_add(1) != 0) return;
    ECSSakura2Processor::Initialize();
    InitializeCriticalSection(&g_csCotopha);
    InitializeCriticalSection(&referenceMutex);
    for (auto &table : functions) {
        *table.list = new ECSStrTagArray;
        for (const wchar_t **name = table.names; *name; ++name) (*table.list)->Add(*name);
    }
}
void ECotophaScript::Release() {
    if (references.fetch_sub(1) != 1) return;
    for (auto &table : functions) { delete *table.list; *table.list = nullptr; }
    primaryContext = nullptr;
    currentThread = nullptr;
    DeleteCriticalSection(&referenceMutex);
    DeleteCriticalSection(&g_csCotopha);
    ECSSakura2Processor::Close();
}
void ECotophaScript::SetPrimaryContext(ECSContext *context) { primaryContext = context; }
ECSContext *ECotophaScript::GetPrimaryContext() { return primaryContext; }
void ECotophaScript::SetCurrentThread(ECSThread *thread) { currentThread = thread; }
ECSThread *ECotophaScript::GetCurrentThread() { return currentThread; }
void ECotophaScript::MultithreadReference(bool enabled) { multithreadReference = enabled; }
void ECotophaScript::LockReference() { if (multithreadReference) EnterCriticalSection(&referenceMutex); }
void ECotophaScript::UnlockReference() { if (multithreadReference) LeaveCriticalSection(&referenceMutex); }
EGLDrawImage *ECotophaScript::CreateDrawImage() {
    study::platform::LogWrite(study::platform::LogPriority::Error, "LegacyCotopha", "Legacy graphics plugin API is unavailable");
    return nullptr;
}
const char *GetESLErrorMsg(ESLError error) {
    const intptr_t value = static_cast<intptr_t>(error);
    if (value > 4096 || value < -4096) return reinterpret_cast<const char *>(value);
    switch (error) {
    case eslErrSuccess: return "success";
    case eslErrNotSupported: return "unsupported operation";
    case eslErrAbort: return "aborted";
    case eslErrInvalidParam: return "invalid parameter";
    case eslErrTimeout: return "timeout";
    case eslErrPending: return "pending";
    case eslErrContinue: return "continue";
    default: return "legacy runtime error";
    }
}
