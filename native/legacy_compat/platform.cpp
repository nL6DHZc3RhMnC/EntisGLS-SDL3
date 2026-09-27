#include <windows.h>
#include <esl.h>
#include <esl/esl_charset.h>
#include <algorithm>
#include <cstring>
#include <new>
#include <sys/mman.h>
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include "../platform/sdl/memory_info.h"
#else
#include <sys/sysinfo.h>
#endif
#include <dlfcn.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <ctime>
#include "../platform/log.h"

void InitializeCriticalSection(CRITICAL_SECTION *cs) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(cs, &attr);
    pthread_mutexattr_destroy(&attr);
}
void DeleteCriticalSection(CRITICAL_SECTION *cs) { pthread_mutex_destroy(cs); }
void EnterCriticalSection(CRITICAL_SECTION *cs) { pthread_mutex_lock(cs); }
void LeaveCriticalSection(CRITICAL_SECTION *cs) { pthread_mutex_unlock(cs); }

// The subset uses VirtualAlloc only for its fixed-size EFileMappedBuffer pages.
// Header-based ownership also preserves the legacy zero-size VirtualFree call.
struct alignas(max_align_t) Allocation { size_t length; };
void *VirtualAlloc(void *, size_t bytes, DWORD allocation, DWORD protection) {
    if (allocation != MEM_COMMIT || protection != PAGE_READWRITE) return nullptr;
    Allocation *block = static_cast<Allocation *>(std::calloc(1, sizeof(Allocation) + bytes));
    if (!block) return nullptr;
    block->length = bytes;
    return block + 1;
}
BOOL VirtualFree(void *address, size_t bytes, DWORD freeType) {
    if (bytes != 0 || freeType != MEM_RELEASE) return FALSE;
    if (address) std::free(static_cast<Allocation *>(address) - 1);
    return TRUE;
}

BOOL IsDBCSLeadByte(BYTE value) { return ESLCharset::IsLeadByteShiftJIS(value); }
int MultiByteToWideChar(UINT codepage, DWORD, const char *src, int length, wchar_t *dst, int capacity) {
    if (codepage != CP_ACP || !src || length == 0 || length < -1 || capacity < 0) return 0;
    const bool terminated = length == -1;
    const unsigned count = terminated ? static_cast<unsigned>(std::strlen(src) + 1) : length;
    return ESLCharset::ShiftJIStoUNICODE(reinterpret_cast<const BYTE *>(src), count, dst, capacity);
}
int WideCharToMultiByte(UINT codepage, DWORD, const wchar_t *src, int length, char *dst, int capacity, const char *, BOOL *defaulted) {
    if (defaulted) *defaulted = FALSE;
    if (codepage != CP_ACP || !src || length == 0 || length < -1 || capacity < 0) return 0;
    const unsigned count = length == -1 ? static_cast<unsigned>(std::wcslen(src) + 1) : length;
    return ESLCharset::UNICODEtoShiftJIS(src, count, reinterpret_cast<BYTE *>(dst), capacity);
}

extern "C" void *eslHeapAllocate(HESLHEAP heap, DWORD size, DWORD flags) {
    // This foundation has no custom legacy heap instances yet. Failing an
    // unsupported heap is safer than accepting it and silently mixing owners.
    if (heap) return nullptr;
    Allocation *block = static_cast<Allocation *>(std::malloc(sizeof(Allocation) + size));
    if (!block) return nullptr;
    block->length = size;
    if (flags & ESL_HEAP_ZERO_INIT) std::memset(block + 1, 0, size);
    return block + 1;
}
extern "C" void eslHeapFree(HESLHEAP heap, void *ptr, DWORD) {
    if (!heap && ptr) std::free(static_cast<Allocation *>(ptr) - 1);
}
extern "C" void *eslHeapReallocate(HESLHEAP heap, void *ptr, DWORD size, DWORD flags) {
    if (heap) return nullptr;
    if (!ptr) return eslHeapAllocate(heap, size, flags);
    Allocation *old = static_cast<Allocation *>(ptr) - 1;
    const size_t oldSize = old->length;
    Allocation *block = static_cast<Allocation *>(std::realloc(old, sizeof(Allocation) + size));
    if (!block) return nullptr;
    block->length = size;
    if ((flags & ESL_HEAP_ZERO_INIT) && size > oldSize)
        std::memset(static_cast<unsigned char *>(static_cast<void *>(block + 1)) + oldSize, 0, size - oldSize);
    return block + 1;
}
extern "C" DWORD eslHeapGetLength(HESLHEAP heap, void *ptr) {
    return (!heap && ptr) ? (static_cast<Allocation *>(ptr) - 1)->length : 0;
}
extern "C" HESLHEAP eslGetGlobalHeap() { return nullptr; } // nullptr selects the process heap, as in eslHeapAllocate.

namespace {
// One condition variable protects all handle states, so WaitAll's test and
// acquisition are atomic. Manual events and recursive mutex ownership retain
// the semantics used by the interpreter and cooperative script threads.
struct SyncHandle {
    bool mutex;
    bool manual;
    bool signaled;
    std::thread::id owner;
    unsigned recursion = 0;
};
std::mutex syncMutex;
std::condition_variable syncChanged;
bool ready(SyncHandle *h, std::thread::id thread) {
    return h->mutex ? (!h->recursion || h->owner == thread) : h->signaled;
}
void acquire(SyncHandle *h, std::thread::id thread) {
    if (h->mutex) { h->owner = thread; ++h->recursion; }
    else if (!h->manual) h->signaled = false;
}
}
HANDLE CreateEvent(LPSECURITY_ATTRIBUTES attributes, BOOL manual, BOOL initial, LPCTSTR name) {
    if (attributes || name) return nullptr; // Named/process-shared events are unsupported.
    return new (std::nothrow) SyncHandle{false, !!manual, !!initial};
}
HANDLE CreateMutex(LPSECURITY_ATTRIBUTES attributes, BOOL initial, LPCTSTR name) {
    if (attributes || name) return nullptr;
    auto *h = new (std::nothrow) SyncHandle{true, false, false};
    if (h && initial) { h->owner = std::this_thread::get_id(); h->recursion = 1; }
    return h;
}
BOOL CloseHandle(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    // As with Win32, closing a handle while another thread waits on it is not
    // permitted. Owners must stop/join script threads before freeing handles.
    std::lock_guard<std::mutex> guard(syncMutex);
    delete static_cast<SyncHandle *>(handle);
    return TRUE;
}
BOOL SetEvent(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    std::lock_guard<std::mutex> guard(syncMutex);
    auto *h = static_cast<SyncHandle *>(handle);
    if (h->mutex) return FALSE;
    h->signaled = true;
    syncChanged.notify_all();
    return TRUE;
}
BOOL ResetEvent(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    std::lock_guard<std::mutex> guard(syncMutex);
    auto *h = static_cast<SyncHandle *>(handle);
    if (h->mutex) return FALSE;
    h->signaled = false;
    return TRUE;
}
BOOL ReleaseMutex(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    std::lock_guard<std::mutex> guard(syncMutex);
    auto *h = static_cast<SyncHandle *>(handle);
    if (!h->mutex || !h->recursion || h->owner != std::this_thread::get_id()) return FALSE;
    if (--h->recursion == 0) { h->owner = {}; syncChanged.notify_all(); }
    return TRUE;
}
DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles, BOOL waitAll, DWORD timeout) {
    if (!count || count > 64 || !handles) return WAIT_FAILED;
    for (DWORD i = 0; i < count; ++i)
        if (!handles[i] || handles[i] == INVALID_HANDLE_VALUE) return WAIT_FAILED;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
    const auto self = std::this_thread::get_id();
    std::unique_lock<std::mutex> lock(syncMutex);
    for (;;) {
        if (waitAll) {
            bool allReady = true;
            for (DWORD i = 0; i < count; ++i) allReady &= ready(static_cast<SyncHandle *>(handles[i]), self);
            if (allReady) {
                for (DWORD i = 0; i < count; ++i) acquire(static_cast<SyncHandle *>(handles[i]), self);
                return WAIT_OBJECT_0;
            }
        } else {
            for (DWORD i = 0; i < count; ++i) {
                auto *h = static_cast<SyncHandle *>(handles[i]);
                if (ready(h, self)) { acquire(h, self); return WAIT_OBJECT_0 + i; }
            }
        }
        if (timeout == 0) return WAIT_TIMEOUT;
        if (timeout == INFINITE) syncChanged.wait(lock);
        else if (syncChanged.wait_until(lock, deadline) == std::cv_status::timeout) return WAIT_TIMEOUT;
    }
}
DWORD WaitForSingleObject(HANDLE handle, DWORD timeout) { return WaitForMultipleObjects(1, &handle, FALSE, timeout); }
LONG InterlockedIncrement(volatile LONG *value) { return __atomic_add_fetch(value, 1, __ATOMIC_SEQ_CST); }
LONG InterlockedDecrement(volatile LONG *value) { return __atomic_sub_fetch(value, 1, __ATOMIC_SEQ_CST); }
LONG InterlockedExchange(volatile LONG *value, LONG replacement) { return __atomic_exchange_n(value, replacement, __ATOMIC_SEQ_CST); }
DWORD timeGetTime() {
    return static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}
void Sleep(DWORD milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
void GetLocalTime(SYSTEMTIME *value) {
    const auto now = std::chrono::system_clock::now();
    const time_t stamp = std::chrono::system_clock::to_time_t(now);
    struct tm local{};
    localtime_r(&stamp, &local);
    *value = SYSTEMTIME{static_cast<WORD>(local.tm_year + 1900), static_cast<WORD>(local.tm_mon + 1), static_cast<WORD>(local.tm_wday), static_cast<WORD>(local.tm_mday), static_cast<WORD>(local.tm_hour), static_cast<WORD>(local.tm_min), static_cast<WORD>(local.tm_sec), static_cast<WORD>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000)};
}
void GlobalMemoryStatus(MEMORYSTATUS *value) {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    study::platform::sdl::MemoryInfo info;
    if (!study::platform::sdl::QueryMemoryInfo(info) || !info.availablePhysicalKnown) {
        // The old Win32-shaped API cannot mark individual fields unknown. Keep
        // its existing all-zero query-failure result instead of inventing RAM.
        std::memset(value, 0, sizeof(*value));
        return;
    }
    const size_t total = size_t(info.totalPhysical);
    const size_t free = size_t(info.availablePhysical);
    *value = MEMORYSTATUS{sizeof(MEMORYSTATUS), static_cast<DWORD>(total ? 100 * (total - free) / total : 0), total, free,
                          info.swapKnown ? size_t(info.totalSwap) : 0,
                          info.swapKnown ? size_t(info.availableSwap) : 0, total, free};
#else
    struct sysinfo info{};
    if (sysinfo(&info) != 0) { std::memset(value, 0, sizeof(*value)); return; }
    const size_t total = size_t(info.totalram) * info.mem_unit;
    const size_t free = size_t(info.freeram) * info.mem_unit;
    *value = MEMORYSTATUS{sizeof(MEMORYSTATUS), static_cast<DWORD>(total ? 100 * (total - free) / total : 0), total, free, size_t(info.totalswap) * info.mem_unit, size_t(info.freeswap) * info.mem_unit, total, free};
#endif
}
HMODULE GetModuleHandle(const char *name) {
    static HMODULE processModule = dlopen(nullptr, RTLD_LAZY);
    return name ? nullptr : processModule;
}
FARPROC GetProcAddress(HMODULE module, const char *name) { return module && name ? reinterpret_cast<FARPROC>(dlsym(module, name)) : nullptr; }
void OutputDebugString(const char *text) { study::platform::LogWrite(study::platform::LogPriority::Debug, "LegacyCotopha", text ? text : ""); }
