#include "platform/sdl/synchronization.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <future>
#include <limits>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace study::platform::sdl;
using namespace std::chrono_literals;

namespace {
void Check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int WakeUntil(int* word, int target, int batch) {
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    int awakened = 0;
    while (awakened < target && std::chrono::steady_clock::now() < deadline) {
        const int count = Futex(word, FutexWake, std::min(batch, target - awakened), nullptr, nullptr, 0);
        Check(count >= 0 && count <= batch, "WAKE returned an invalid count");
        awakened += count;
        std::this_thread::yield();
    }
    Check(awakened == target, "WAKE never found the expected waiters");
    return awakened;
}

void CheckErrorsAndTimeout() {
    int word = 5;
    Check(Futex(&word, FutexWait, 4, nullptr, nullptr, 0) == -1 && errno == EAGAIN,
          "WAIT did not reject a changed value");
    Check(Futex(&word, 128, 5, nullptr, nullptr, 0) == -1 && errno == ENOSYS,
          "unsupported operation was accepted");
    Check(Futex(nullptr, FutexWait, 0, nullptr, nullptr, 0) == -1 && errno == EFAULT,
          "null word was accepted");
    Check(Futex(&word, FutexWake, -1, nullptr, nullptr, 0) == -1 && errno == EINVAL,
          "negative wake count was accepted");
    Check(Futex(&word, FutexWake, 1, nullptr, &word, 0) == -1 && errno == EINVAL,
          "unsupported second address was accepted");
    timespec invalid{0, 1000000000};
    Check(Futex(&word, FutexWait, 5, &invalid, nullptr, 0) == -1 && errno == EINVAL,
          "invalid relative timeout was accepted");
    const timespec timeout{0, 25000000};
    const auto start = std::chrono::steady_clock::now();
    Check(Futex(&word, FutexWait, 5, &timeout, nullptr, 0) == -1 && errno == ETIMEDOUT,
          "WAIT without wake did not time out");
    const auto elapsed = std::chrono::steady_clock::now() - start;
    Check(elapsed >= 20ms && elapsed < 1s, "relative timeout duration is incorrect");
    Check(Futex(&word, FutexWake, 1, nullptr, nullptr, 0) == 0,
          "timed-out waiter remained in queue");
}

void CheckWakeCountAndIsolation() {
    int word = 0;
    int other = 0;
    std::atomic<int> completed{0};
    std::vector<std::future<int>> waiters;
    for (int i = 0; i < 6; ++i) {
        waiters.emplace_back(std::async(std::launch::async, [&] {
            const timespec timeout{3, 0};
            const int status = Futex(&word, FutexWait, 0, &timeout, nullptr, 0);
            if (status == 0) ++completed;
            return status;
        }));
    }
    auto isolated = std::async(std::launch::async, [&] {
        const timespec timeout{3, 0};
        return Futex(&other, FutexWait, 0, &timeout, nullptr, 0);
    });
    Check(Futex(&word, FutexWake, 0, nullptr, nullptr, 0) == 0, "zero wake count woke a waiter");
    WakeUntil(&word, 3, 3);
    const auto deadline = std::chrono::steady_clock::now() + 1s;
    while (completed < 3 && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
    Check(completed == 3, "three wakes did not release exactly three waiters");
    std::this_thread::sleep_for(25ms);
    Check(completed == 3, "one WAKE incorrectly released extra waiters");
    Check(isolated.wait_for(0ms) == std::future_status::timeout, "WAKE crossed address boundaries");
    WakeUntil(&word, 3, 1);
    WakeUntil(&other, 1, 1);
    for (auto& waiter : waiters) Check(waiter.get() == 0, "selected waiter failed to wake");
    Check(isolated.get() == 0, "second address did not wake");
    Check(word == 0, "WAKE unexpectedly modified its word");
    Check(Futex(&word, FutexWake, 100, nullptr, nullptr, 0) == 0, "wake left queued waiters behind");
}

void CheckInfiniteWake() {
    int word = -1; // The SDK's contended mutex sentinel.
    auto waiter = std::async(std::launch::async, [&] {
        return Futex(&word, FutexWait, -1, nullptr, nullptr, 0);
    });
    WakeUntil(&word, 1, 1);
    Check(waiter.get() == 0, "infinite WAIT failed to wake");
}

void CheckPublishRace() {
    for (int i = 0; i < 500; ++i) {
        int word = 0;
        auto waiter = std::async(std::launch::async, [&] {
            const timespec timeout{0, 200000000};
            const int status = Futex(&word, FutexWait, 0, &timeout, nullptr, 0);
            return status == 0 || (status == -1 && errno == EAGAIN);
        });
        if (i & 1) std::this_thread::yield();
        __atomic_store_n(&word, 1, __ATOMIC_SEQ_CST);
        Futex(&word, FutexWake, 1, nullptr, nullptr, 0);
        Check(waiter.get(), "publication/wait registration race lost a wake");
    }
}

void CheckContendedLock() {
    int word = 0;
    int counter = 0;
    std::vector<std::future<bool>> workers;
    for (int i = 0; i < 8; ++i) {
        workers.emplace_back(std::async(std::launch::async, [&] {
            for (int repeat = 0; repeat < 10000; ++repeat) {
                while (__atomic_exchange_n(&word, 1, __ATOMIC_ACQUIRE)) {
                    const timespec timeout{0, 1000000};
                    if (Futex(&word, FutexWait, 1, &timeout, nullptr, 0) == -1 &&
                        errno != EAGAIN && errno != ETIMEDOUT) return false;
                }
                ++counter;
                __atomic_store_n(&word, 0, __ATOMIC_RELEASE);
                Futex(&word, FutexWake, 1, nullptr, nullptr, 0);
            }
            return true;
        }));
    }
    for (auto& worker : workers) Check(worker.get(), "contended WAIT failed");
    Check(counter == 80000, "contended lock lost protected writes");
}

void CheckThreadIds() {
    std::set<std::int32_t> ids{CurrentThreadId()};
    std::vector<std::future<std::int32_t>> threads;
    for (int i = 0; i < 64; ++i) {
        threads.emplace_back(std::async(std::launch::async, [] {
            const auto id = CurrentThreadId();
            Check(id > 0 && id == CurrentThreadId(), "thread ID changed or is reserved");
            return id;
        }));
    }
    for (auto& thread : threads) Check(ids.insert(thread.get()).second, "thread IDs collided");
}
} // namespace

int main() {
    try {
        CheckErrorsAndTimeout();
        CheckWakeCountAndIsolation();
        CheckInfiniteWake();
        CheckPublishRace();
        CheckContendedLock();
        CheckThreadIds();
        std::puts("SDL sync PASS: errors, real timeout/infinite wait/wake counts/address isolation, 500 publication races, 80000 contended increments, 65 unique thread IDs");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "SDL sync FAIL: %s\n", error.what());
        return 1;
    }
}
