#pragma once
#include <memory>
#include <string>

namespace entis::launcher {
class PsbKeyResolver;
// Published before the game's runtime starts. Each motion owner holds a shared
// reference so teardown cannot invalidate an in-flight archive callback.
void SetGamePsbKeyResolver(std::shared_ptr<PsbKeyResolver> resolver);
std::shared_ptr<PsbKeyResolver> GetGamePsbKeyResolver();
void ReportGamePsbKeyError(const std::string& message);
std::string GamePsbKeyError();
bool HasGamePsbKeyError();
}
