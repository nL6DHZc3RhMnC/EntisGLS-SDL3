#include "extensions/emote/psb/psb_key_runtime.h"
#include "extensions/emote/psb/psb_key_resolver.h"
#include <mutex>
#include <atomic>

namespace entis::launcher {
namespace {
std::mutex stateMutex;
std::shared_ptr<PsbKeyResolver> activeResolver;
std::string firstError;
std::atomic<bool> failed{false};
}
void SetGamePsbKeyResolver(std::shared_ptr<PsbKeyResolver> resolver) {
    std::lock_guard<std::mutex> lock(stateMutex);
    activeResolver = std::move(resolver);
    firstError.clear();
    failed.store(false, std::memory_order_release);
}
std::shared_ptr<PsbKeyResolver> GetGamePsbKeyResolver() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return activeResolver;
}
void ReportGamePsbKeyError(const std::string& message) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (firstError.empty()) firstError = message;
    failed.store(true, std::memory_order_release);
}
bool HasGamePsbKeyError() { return failed.load(std::memory_order_acquire); }
std::string GamePsbKeyError() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return firstError;
}
}
