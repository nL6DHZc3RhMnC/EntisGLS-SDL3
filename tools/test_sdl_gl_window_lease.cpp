#include "platform/gl.h"
#include "platform/sdl/gl_window_lease.h"
#include "motion_bridge/tjs_runtime/runtime_gl.h"
#include <SDL3/SDL_init.h>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
void Check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
}

int main() {
    try {
        Check(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        auto* window = SDL_CreateWindow("GL lease validation", 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        Check(window, SDL_GetError());
        const auto context = SDL_GL_CreateContext(window);
        Check(context, SDL_GetError());
        const auto windowId = SDL_GetWindowID(window);
        auto lease = study::platform::sdl::RegisterGlWindow(window, context);
        std::weak_ptr<study::platform::sdl::GlWindowOwner> weak = lease;
        auto child = std::make_unique<studysteady::motion::RuntimeGl>(
            reinterpret_cast<std::uintptr_t>(window), reinterpret_cast<std::uintptr_t>(context));
        Check(SDL_GL_GetCurrentContext() == context, "child creation did not restore parent");

        SDL_HideWindow(window);
        study::platform::sdl::UnregisterGlWindow(window);
        lease.reset(); // Equivalent to removing the SDK window state.
        Check(!weak.expired() && SDL_GetWindowFromID(windowId) == window, "SDK close destroyed a leased window");
        bool rejected = false;
        try { (void)study::platform::sdl::AcquireGlWindowLease(window, context); }
        catch (const std::runtime_error&) { rejected = true; }
        Check(rejected, "closing window accepted a new renderer");
        {
            studysteady::motion::CurrentGlContextScope restore;
            child->makeCurrent();
            Check(glGetString(GL_VERSION), "dependent GL context died on SDK close");
            child.reset(); // Last renderer ends after SDK window removal.
            Check(!weak.expired(), "saved context scope lost its lifetime protection");
            restore.restore();
            Check(SDL_GL_GetCurrentContext() == nullptr, "restored a logically closed parent context");
        }
        Check(weak.expired(), "physical window lease leaked after the last renderer");
        Check(SDL_GetWindowFromID(windowId) == nullptr, "physical SDL window was not destroyed");
        SDL_Quit();
        std::cout << "SDL GL lease PASS: SDK close retains dependent context, rejects new owners, restores no closed context, and final release destroys physical window\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SDL GL lease FAIL: " << error.what() << '\n'; return 1;
    }
}
