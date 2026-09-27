// Manual display smoke test; deliberately not registered as a headless CTest.
#include "../presentation.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <cstdio>
#include <thread>

using namespace study::platform::sdl;

int main() {
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::fprintf(stderr, "%s\n", SDL_GetError()); return 1; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    auto* window = SDL_CreateWindow("StudySteady presentation smoke", 320, 180,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    const auto context = window ? SDL_GL_CreateContext(window) : nullptr;
    if (!context || !ConfigureImmediatePresentation()) {
        std::fprintf(stderr, "Create immediate GL window: %s\n", SDL_GetError()); return 1;
    }
    auto clearColor = reinterpret_cast<void (*)(float,float,float,float)>(SDL_GL_GetProcAddress("glClearColor"));
    auto clear = reinterpret_cast<void (*)(unsigned)>(SDL_GL_GetProcAddress("glClear"));
    auto getString = reinterpret_cast<const unsigned char* (*)(unsigned)>(SDL_GL_GetProcAddress("glGetString"));
    if (!clearColor || !clear || !getString) return 1;
    std::printf("renderer=%s version=%s swap_interval=0\n", getString(0x1f01), getString(0x1f02));
    std::atomic<int> serviced{0};
    unsigned presented = 0, skipped = 0;
    Uint64 longestSwap = 0;
    bool ok = true;
    WindowFramePacer pacer;
    const auto phase = [&](const char* name, bool visible) {
        if (visible) {
            const auto readyDeadline = SDL_GetTicks() + 1500;
            while (!IsWindowPresentable(window) && SDL_GetTicks() < readyDeadline) {
                SDL_Event event; while (SDL_PollEvent(&event)) {} SDL_Delay(1);
            }
        }
        const int previous = serviced.load();
        std::thread request([&] {
            if (!SDL_RunOnMainThread([](void* userdata) {
                static_cast<std::atomic<int>*>(userdata)->fetch_add(1);
            }, &serviced, true)) serviced.store(-1000);
        });
        const auto end = SDL_GetTicks() + 180;
        unsigned phasePresented = 0;
        do {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {}
            if (pacer.IsDue()) {
                if (IsWindowPresentable(window)) {
                    clearColor(.15f, .4f, .6f, 1.f); clear(0x00004000); // GL_COLOR_BUFFER_BIT
                    const auto before = SDL_GetTicksNS();
                    ok &= SDL_GL_SwapWindow(window);
                    const auto elapsed = SDL_GetTicksNS() - before;
                    if (elapsed > longestSwap) longestSwap = elapsed;
                    ++presented; ++phasePresented;
                } else ++skipped;
            }
            SDL_Delay(1);
        } while (SDL_GetTicks() < end || serviced.load() == previous);
        request.join();
        ok &= serviced.load() == previous + 1;
        ok &= visible ? phasePresented > 0 : phasePresented == 0;
        std::printf("%s: presented=%u flags=0x%llx callback=%d\n", name,
                    phasePresented, (unsigned long long)SDL_GetWindowFlags(window), serviced.load());
    };
    phase("initial hidden", false);
    for (int cycle = 0; cycle < 3; ++cycle) {
        ok &= SDL_ShowWindow(window); ok &= SDL_RaiseWindow(window);
        phase("shown", true);
        ok &= SDL_HideWindow(window);
        phase("hidden", false);
    }
    ok &= SDL_ShowWindow(window); ok &= SDL_RaiseWindow(window);
    phase("shown before minimize", true);
    ok &= SDL_MinimizeWindow(window);
    // Window-manager transitions are asynchronous; only start assertions after
    // the minimized flag is observed, with a bounded wait for the real event.
    const auto minimizeDeadline = SDL_GetTicks() + 1500;
    while (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) && SDL_GetTicks() < minimizeDeadline) {
        SDL_Event event; while (SDL_PollEvent(&event)) {} SDL_Delay(1);
    }
    ok &= bool(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED);
    phase("minimized", false);
    ok &= SDL_RestoreWindow(window); ok &= SDL_RaiseWindow(window);
    phase("restored", true);
    std::printf("result=%s presented=%u skipped=%u max_swap_ms=%.3f\n",
                ok ? "PASS" : "FAIL", presented, skipped, longestSwap / 1000000.0);
    SDL_GL_DestroyContext(context); SDL_DestroyWindow(window); SDL_Quit();
    return ok ? 0 : 1;
}
