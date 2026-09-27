#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>

static void require(bool value, const char* stage) {
    if (!value) throw std::runtime_error(stage);
}

int main(int, char**) {
    SDL_Window* window = nullptr;
    SDL_GLContext first = nullptr, second = nullptr;
    try {
        require(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        window = SDL_CreateWindow("StudySteady SDL GL validation", 128, 128,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        require(window != nullptr, SDL_GetError());
        first = SDL_GL_CreateContext(window);
        require(first != nullptr, SDL_GetError());
        std::printf("GL: %s\nGLSL: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        using GenFramebuffers = void (*)(GLsizei, GLuint*);
        using BindFramebuffer = void (*)(GLenum, GLuint);
        using AttachTexture = void (*)(GLenum, GLenum, GLenum, GLuint, GLint);
        using CheckFramebuffer = GLenum (*)(GLenum);
        auto gen = reinterpret_cast<GenFramebuffers>(SDL_GL_GetProcAddress("glGenFramebuffers"));
        auto bind = reinterpret_cast<BindFramebuffer>(SDL_GL_GetProcAddress("glBindFramebuffer"));
        auto attach = reinterpret_cast<AttachTexture>(SDL_GL_GetProcAddress("glFramebufferTexture2D"));
        auto complete = reinterpret_cast<CheckFramebuffer>(SDL_GL_GetProcAddress("glCheckFramebufferStatus"));
        require(gen && bind && attach && complete, "FBO entry points unavailable");
        const unsigned char original[] = {19, 47, 89, 255, 31, 59, 97, 255, 41, 67, 103, 255, 53, 71, 109, 255};
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, original);
        glFinish();
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        second = SDL_GL_CreateContext(window);
        require(second != nullptr, SDL_GetError());
        GLuint framebuffer = 0;
        gen(1, &framebuffer);
        bind(GL_FRAMEBUFFER, framebuffer);
        attach(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        require(complete(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "shared texture framebuffer incomplete");
        unsigned char readback[sizeof(original)] = {};
        glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, readback);
        require(glGetError() == GL_NO_ERROR && !std::memcmp(original, readback, sizeof(original)),
                "shared texture readback mismatch");
        std::puts("PASS: SDL contexts share texture data with exact FBO readback");
        bind(GL_FRAMEBUFFER, 0);
        require(SDL_GL_MakeCurrent(window, first), SDL_GetError());
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        require(SDL_GL_SwapWindow(window), SDL_GetError());
        std::puts("PASS: original SDL context resumes rendering and presents");
        SDL_GL_DestroyContext(second); second = nullptr;
        SDL_GL_DestroyContext(first); first = nullptr;
        SDL_DestroyWindow(window); window = nullptr;
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s; SDL: %s\n", error.what(), SDL_GetError());
        if (second) SDL_GL_DestroyContext(second);
        if (first) SDL_GL_DestroyContext(first);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
}
