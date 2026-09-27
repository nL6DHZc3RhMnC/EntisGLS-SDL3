#include "gles_atlas.h"
#include <EGL/egl.h>
#include <fstream>
#include <iostream>
#include <zlib.h>

namespace {
struct EglContext {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    EglContext() {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr)) throw std::runtime_error("EGL initialize failed");
        const EGLint attrs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
        EGLConfig config; EGLint count;
        if(!eglChooseConfig(display, attrs, &config, 1, &count) || count != 1) throw std::runtime_error("EGL config failed");
        const EGLint surfaceAttrs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
        surface = eglCreatePbufferSurface(display, config, surfaceAttrs);
        const EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttrs);
        if(surface == EGL_NO_SURFACE || context == EGL_NO_CONTEXT || !eglMakeCurrent(display, surface, surface, context))
            throw std::runtime_error("EGL context failed");
    }
    ~EglContext() {
        if(display != EGL_NO_DISPLAY) {
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if(context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
            if(surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
            eglTerminate(display);
        }
    }
};
}

int main(int argc, char **argv) {
    if(argc != 4) { std::cerr << "Usage: motion_gles_probe file.psb seed group\n"; return 2; }
    try {
        studysteady::motion::WinAtlas atlas(
            studysteady::motion::loadPsb(argv[1], static_cast<uint32_t>(std::stoull(argv[2]))).GetRoot(), argv[3]);
        EglContext context;
        std::cout << "renderer=" << glGetString(GL_RENDERER) << " version=" << glGetString(GL_VERSION) << '\n';
        const GLuint texture = studysteady::motion::uploadWinAtlasGles(atlas);
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Atlas FBO incomplete");
        std::vector<uint8_t> readback(atlas.rgbaBytes());
        glReadPixels(0, 0, atlas.width(), atlas.height(), GL_RGBA, GL_UNSIGNED_BYTE, readback.data());
        if(glGetError() != GL_NO_ERROR) throw std::runtime_error("Atlas GPU readback failed");
        const auto expected = crc32(0, atlas.rgbaPixels(), static_cast<uInt>(atlas.rgbaBytes()));
        const auto actual = crc32(0, readback.data(), static_cast<uInt>(readback.size()));
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &texture);
        if(expected != actual) throw std::runtime_error("Atlas CPU/GPU CRC mismatch");
        std::cout << "GPU upload/readback: PASS size=" << atlas.width() << 'x' << atlas.height()
                  << " bytes=" << atlas.rgbaBytes() << " crc32=" << std::hex << actual << '\n';
        return 0;
    } catch(const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}
