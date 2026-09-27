#pragma once

#include <memory>
#include <string>
#include <vector>

#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include <SDL3/SDL_stdinc.h>
#else
extern "C" { extern char** environ; }
#endif

namespace study::platform {

// Take a fresh process-environment snapshot, preserving the legacy query's
// behavior after C-runtime changes. As with environ, callers must serialize
// concurrent setenv/unsetenv operations while this snapshot is populated.
inline bool ReadEnvironmentVariables(std::vector<std::string>& output) {
    output.clear();
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    std::unique_ptr<SDL_Environment, decltype(&SDL_DestroyEnvironment)> environment(
        SDL_CreateEnvironment(true), SDL_DestroyEnvironment);
    if (!environment) return false;
    std::unique_ptr<char*, decltype(&SDL_free)> variables(
        SDL_GetEnvironmentVariables(environment.get()), SDL_free);
    if (!variables) return false;
    char** entries = variables.get();
#else
    char** entries = ::environ;
#endif
    for (char** current = entries; current && *current; ++current) output.emplace_back(*current);
    return true;
}

} // namespace study::platform
