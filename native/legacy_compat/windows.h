#pragma once

// The legacy foundation only uses these Win32 scalar/synchronization concepts.
// Actual implementations live in platform.cpp; rendering/window APIs are absent.
#define __ENTIS_GLS__ 1
#include <sakura/sakura_cpp_presets.h>
#include <esl/esl_stddefs.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <pthread.h>
#include <algorithm>

#define GLSEXPORT
#define _DISABLE_ESL_NEW 1
using HANDLE = void *;
using HMODULE = void *;
using HINSTANCE = void *;
using HICON = void *;
using HCURSOR = void *;
using BOOL = int;
using LPVOID = void *;
using LPCVOID = const void *;
using FARPROC = void (*)();
using CRITICAL_SECTION = pthread_mutex_t;
struct SYSTEMTIME { WORD wYear,wMonth,wDayOfWeek,wDay,wHour,wMinute,wSecond,wMilliseconds; };
using LPSYSTEMTIME = SYSTEMTIME *;
#define WINAPI
#define CALLBACK
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#define TRUE 1
#define FALSE 0
#define INFINITE 0xffffffffu
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define MEM_COMMIT 0x1000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 4
#define CP_ACP 0
#define MB_PRECOMPOSED 1
#define __max(a,b) ((a) > (b) ? (a) : (b))
#define __min(a,b) ((a) < (b) ? (a) : (b))
#define WAIT_OBJECT_0 0u
#define WAIT_TIMEOUT 258u
#define WAIT_FAILED 0xffffffffu
#define WAIT_ABANDONED 0x80u
struct MEMORYSTATUS { DWORD dwLength, dwMemoryLoad; size_t dwTotalPhys,dwAvailPhys,dwTotalPageFile,dwAvailPageFile,dwTotalVirtual,dwAvailVirtual; };
using LPCTSTR = const char *;
using LPSECURITY_ATTRIBUTES = void *;
using LPLONG = LONG *;

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL manualReset, BOOL initialState, LPCTSTR name);
HANDLE CreateMutex(LPSECURITY_ATTRIBUTES, BOOL initialOwner, LPCTSTR name);
BOOL CloseHandle(HANDLE handle);
BOOL SetEvent(HANDLE handle);
BOOL ResetEvent(HANDLE handle);
BOOL ReleaseMutex(HANDLE handle);
DWORD WaitForSingleObject(HANDLE handle, DWORD timeout);
DWORD WaitForMultipleObjects(DWORD count, const HANDLE *handles, BOOL waitAll, DWORD timeout);
LONG InterlockedIncrement(volatile LONG *value);
LONG InterlockedDecrement(volatile LONG *value);
LONG InterlockedExchange(volatile LONG *value, LONG replacement);
DWORD timeGetTime();
void Sleep(DWORD milliseconds);
void GetLocalTime(SYSTEMTIME *value);
void GlobalMemoryStatus(MEMORYSTATUS *value);
HMODULE GetModuleHandle(const char *name);
FARPROC GetProcAddress(HMODULE module, const char *name);
void OutputDebugString(const char *text);

void InitializeCriticalSection(CRITICAL_SECTION *cs);
void DeleteCriticalSection(CRITICAL_SECTION *cs);
void EnterCriticalSection(CRITICAL_SECTION *cs);
void LeaveCriticalSection(CRITICAL_SECTION *cs);
void *VirtualAlloc(void *address, size_t bytes, DWORD allocation, DWORD protection);
BOOL VirtualFree(void *address, size_t bytes, DWORD freeType);
int MultiByteToWideChar(UINT codepage, DWORD flags, const char *src, int length, wchar_t *dst, int capacity);
int WideCharToMultiByte(UINT codepage, DWORD flags, const wchar_t *src, int length, char *dst, int capacity, const char *, BOOL *);
BOOL IsDBCSLeadByte(BYTE value);
