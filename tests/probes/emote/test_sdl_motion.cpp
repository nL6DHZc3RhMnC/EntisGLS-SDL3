#include "extensions/emote/tjs_runtime/motion_apk_runtime.h"
#include "../../fixtures/psb_key_argument.h"
#include "platform/gl.h"
#include "platform/sdl/main_thread.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <array>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
void Check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

struct HostState {
    SDL_Window* window;
    SDL_GLContext context;
    GLuint texture;
    void CheckIntact() const {
        Check(SDL_GL_GetCurrentWindow() == window && SDL_GL_GetCurrentContext() == context,
              "motion call did not restore the host context");
        GLint viewport[4], bound = 0;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound);
        Check(viewport[0] == 3 && viewport[1] == 5 && viewport[2] == 31 && viewport[3] == 47,
              "motion call modified the host viewport");
        Check(bound == GLint(texture), "motion call modified the host texture binding");
        Check(glGetError() == GL_NO_ERROR, "host context has a GL error");
    }
};
}

int main(int argc, char** argv) {
    if (argc != 3) { std::cerr << "Usage: test_sdl_motion original.psb header_seed\n"; return 2; }
    StudyMotionRuntime* runtime = nullptr;
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    try {
        const auto psbKey=ParsePsbKeyArgument(argv[2]);
        Check(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
        Check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2), SDL_GetError());
        Check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1), SDL_GetError());
        window = SDL_CreateWindow("StudySteady E-mote GL validation", 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        Check(window != nullptr, SDL_GetError());
        context = SDL_GL_CreateContext(window);
        Check(context != nullptr, SDL_GetError());
        std::cout << "GL=" << glGetString(GL_VERSION) << '\n';

        auto dispatched = std::async(std::launch::async, [] {
            const int result = study::platform::sdl::RunOnMainThreadSync([] {
                Check(SDL_IsMainThread(), "dispatch ran on a worker"); return 42;
            });
            bool caught = false;
            try { study::platform::sdl::RunOnMainThreadSync([] { throw std::runtime_error("dispatch exception"); }); }
            catch (const std::runtime_error&) { caught = true; }
            Check(result == 42 && caught, "dispatch did not preserve return value/exception");
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (dispatched.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
            Check(std::chrono::steady_clock::now() < deadline, "main-thread dispatch timed out");
            SDL_PumpEvents(); SDL_Delay(1);
        }
        dispatched.get();

        GLuint hostTexture = 0;
        glGenTextures(1, &hostTexture); glBindTexture(GL_TEXTURE_2D, hostTexture);
        glViewport(3, 5, 31, 47);
        const HostState host{window, context, hostTexture};
        char error[1024] = {};
        runtime = study_motion_create(error, sizeof(error)); Check(runtime != nullptr, error);
        auto Require = [&](int result) {
            host.CheckIntact();
            if (!result) throw std::runtime_error(study_motion_last_error(runtime));
        };
        Require(study_motion_initialize_gles(runtime, reinterpret_cast<std::uintptr_t>(window), reinterpret_cast<std::uintptr_t>(context)));
        StudyMotionStats stats{}; Require(study_motion_stats(runtime, &stats));
        Check((stats.capabilities & STUDY_MOTION_GLES_RENDERER) != 0, "renderer capability missing");
        const auto project = study_motion_load_project(runtime, argv[1], psbKey); Require(project != 0);
        const auto actor = study_motion_create_player(runtime, project); Require(actor != 0);
        double left, top, right, bottom;
        Require(study_motion_player_bounds(runtime, actor, &left, &top, &right, &bottom));
        Check(right > left && bottom > top, "actor bounds empty");
        const double scale = std::min(480 / (right - left), 480 / (bottom - top));
        const double matrix[] = {scale, 0, 0, scale, 256 - (left + right) * scale / 2, 256 - (top + bottom) * scale / 2};
        StudyMotionFrame frame{};
        Require(study_motion_render_player(runtime, actor, 512, 512, matrix, &frame));
        std::vector<std::uint8_t> pixels(512 * 512 * 4), repeated(pixels.size()), shared(pixels.size());
        Require(study_motion_read_pixels(runtime, actor, pixels.data(), pixels.size()));
        Require(study_motion_render_player(runtime, actor, 512, 512, matrix, &frame));
        Require(study_motion_read_pixels(runtime, actor, repeated.data(), repeated.size()));
        Check(pixels == repeated, "zero-time actor render changed pixels");
        std::size_t alpha = 0;
        for (std::size_t i = 3; i < pixels.size(); i += 4) alpha += pixels[i] != 0;
        Check(alpha > 0, "real actor rendered no visible pixels");
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame.texture, 0);
        Check(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "motion texture not shared with host");
        glReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, shared.data());
        for (std::size_t y = 0; y < 512; ++y)
            Check(std::memcmp(shared.data() + y * 2048, pixels.data() + (511 - y) * 2048, 2048) == 0,
                  "shared host texture pixels differ from motion readback");
        glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &framebuffer);
        Check(!study_motion_render_player(runtime, std::numeric_limits<std::uint64_t>::max(), 512, 512, matrix, &frame),
              "invalid actor unexpectedly succeeded");
        host.CheckIntact();
        Require(study_motion_destroy_player(runtime, actor));
        Require(study_motion_unload_project(runtime, project));
        Check(study_motion_destroy(runtime, error, sizeof(error)), error); runtime = nullptr;
        host.CheckIntact(); glDeleteTextures(1, &hostTexture);
        SDL_GL_DestroyContext(context); context = nullptr;
        SDL_DestroyWindow(window); window = nullptr; SDL_Quit();
        std::cout << "SDL motion PASS: real PSB actor alpha=" << alpha
                  << ", repeated draw, shared texture pixel equality, context/state restoration including failure/destruction, main-thread return/exception dispatch\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "SDL motion FAIL: " << error.what() << '\n';
        if (runtime) { char detail[1024] = {}; study_motion_destroy(runtime, detail, sizeof(detail)); }
        if (context) SDL_GL_DestroyContext(context);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit(); return 1;
    }
}
