#include "synchronization.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <limits>
#include <iterator>
#include <list>
#include <mutex>
#include <new>
#include <system_error>

namespace study::platform::sdl {
namespace {

struct Waiter {
    int* address;
    bool notified = false;
    std::condition_variable changed;
};

struct Registry {
    std::mutex mutex;
    std::list<Waiter*> waiters;
};

Registry& GetRegistry() {
    // SDK synchronization objects can be used by static destructors. Keeping
    // this single registry alive avoids cross-translation-unit teardown order.
    static Registry* registry = new Registry;
    return *registry;
}

int Fail(int error) {
    errno = error;
    return -1;
}

} // namespace

int Futex(int* address, int operation, int value, const timespec* timeout,
          int* secondAddress, int thirdValue) {
    static_assert(sizeof(int) == sizeof(std::int32_t));
    if (operation != FutexWait && operation != FutexWake) return Fail(ENOSYS);
    if (!address) return Fail(EFAULT);
    if (reinterpret_cast<std::uintptr_t>(address) % alignof(std::int32_t)) return Fail(EINVAL);
    if (secondAddress || thirdValue) return Fail(EINVAL);
    if (operation == FutexWake && (value < 0 || timeout)) return Fail(EINVAL);
    if (timeout && (timeout->tv_sec < 0 || timeout->tv_nsec < 0 || timeout->tv_nsec >= 1000000000)) {
        return Fail(EINVAL);
    }

    using Clock = std::chrono::steady_clock;
    Clock::time_point deadline{};
    if (timeout) {
        const auto now = Clock::now();
        const auto maximum = Clock::time_point::max() - now;
        // Reject values that cannot be represented without wrapping the clock.
        if (timeout->tv_sec >= std::chrono::duration_cast<std::chrono::seconds>(maximum).count()) {
            return Fail(EOVERFLOW);
        }
        deadline = now + std::chrono::seconds(timeout->tv_sec)
                       + std::chrono::nanoseconds(timeout->tv_nsec);
    }

    Registry& registry = GetRegistry();
    std::unique_lock<std::mutex> lock(registry.mutex);
    if (operation == FutexWake) {
        int awakened = 0;
        for (Waiter* waiter : registry.waiters) {
            if (awakened == value) break;
            if (waiter->address == address && !waiter->notified) {
                waiter->notified = true;
                waiter->changed.notify_one();
                ++awakened;
            }
        }
        return awakened;
    }

    // Checking the word and joining the queue share the wake-side mutex. A
    // publisher therefore either changes the value before this check, or wakes
    // the queued waiter; it cannot slip a wake into the gap between the two.
    if (__atomic_load_n(address, __ATOMIC_SEQ_CST) != value) return Fail(EAGAIN);
    Waiter waiter{address, false, {}};
    try {
        registry.waiters.push_back(&waiter);
    } catch (const std::bad_alloc&) {
        return Fail(ENOMEM);
    }
    const auto position = std::prev(registry.waiters.end());
    bool signaled = true;
    try {
        if (timeout) {
            signaled = waiter.changed.wait_until(lock, deadline, [&] { return waiter.notified; });
        } else {
            waiter.changed.wait(lock, [&] { return waiter.notified; });
        }
    } catch (const std::system_error& error) {
        registry.waiters.erase(position);
        return Fail(error.code().value());
    }
    registry.waiters.erase(position);
    return signaled ? 0 : Fail(ETIMEDOUT);
}

std::int32_t CurrentThreadId() {
    static std::atomic<std::uint64_t> nextId{1};
    thread_local const std::int32_t id = [] {
        const auto value = nextId.fetch_add(1, std::memory_order_relaxed);
        // Reusing an ID could make an unrelated thread appear to own a mutex.
        if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
            std::abort();
        }
        return static_cast<std::int32_t>(value);
    }();
    return id;
}

} // namespace study::platform::sdl
