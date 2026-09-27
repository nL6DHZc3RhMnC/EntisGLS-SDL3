#pragma once
#include <mutex>
#include <unordered_map>

// Independent of the SDK UI mutex. A frozen draw must return even if the
// script owns that mutex while waiting for synchronous rendering work.
template <class Window> class LegacyPaintGate {
    std::mutex mutex_;
    std::unordered_map<const Window*, unsigned> depths_;
public:
    void Freeze(const Window* window) {
        std::lock_guard<std::mutex> guard(mutex_);
        ++depths_[window];
    }
    bool Unfreeze(const Window* window) {
        std::lock_guard<std::mutex> guard(mutex_);
        const auto found = depths_.find(window);
        if (found == depths_.end()) return false;
        if (--found->second == 0) depths_.erase(found);
        return true;
    }
    void Reset(const Window* window) {
        std::lock_guard<std::mutex> guard(mutex_);
        depths_.erase(window);
    }
    bool IsFrozen(const Window* window) {
        std::lock_guard<std::mutex> guard(mutex_);
        return depths_.find(window) != depths_.end();
    }
    template <class DrawCallback> void Draw(Window* window, DrawCallback&& callback) {
        // IsFrozen has released mutex_ before the callback can enter the SDK.
        if (window && !IsFrozen(window)) callback(window);
    }
};
