#pragma once

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_error.h>
#include <future>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace study::platform::sdl {

// The caller must release locks needed by the main thread before calling this.
// Keep the SDL event loop alive until all producers and their destructors stop.
// packaged_task catches C++ exceptions before returning through SDL's C callback.
template<class Function>
auto RunOnMainThreadSync(Function&& function) -> std::invoke_result_t<Function> {
    using Result = std::invoke_result_t<Function>;
    if (SDL_IsMainThread()) return std::forward<Function>(function)();
    std::packaged_task<Result()> task(std::forward<Function>(function));
    auto result = task.get_future();
    if (!SDL_RunOnMainThread([](void* pointer) {
            (*static_cast<std::packaged_task<Result()>*>(pointer))();
        }, &task, true)) {
        throw std::runtime_error(SDL_GetError());
    }
    return result.get();
}

} // namespace study::platform::sdl
