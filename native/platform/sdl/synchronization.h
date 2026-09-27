#pragma once

#include <cstdint>
#include <ctime>

namespace study::platform::sdl {

// Only the process-local operations used by the original SSystem synchronism
// implementation are supported. This is not a general Linux syscall emulator.
inline constexpr int FutexWait = 0;
inline constexpr int FutexWake = 1;

// Returns the Linux-compatible WAIT/WAKE result, or -1 with errno set. The word
// must remain alive and 32-bit aligned until its last waiter has completed.
// Callers must publish word changes before Wake and recheck their own predicate.
int Futex(int* address, int operation, int value, const timespec* timeout,
          int* secondAddress, int thirdValue);

// SSystem stores thread ownership in pid_t. A process-local ID avoids truncating
// the pointer-sized pthread/SDL thread ID on Apple platforms. Zero is reserved.
std::int32_t CurrentThreadId();

} // namespace study::platform::sdl
