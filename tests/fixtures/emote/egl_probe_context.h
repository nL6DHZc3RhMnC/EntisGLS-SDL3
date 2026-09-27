#pragma once
#include <EGL/egl.h>
#include <stdexcept>
struct MotionProbeEgl {
    EGLDisplay display=EGL_NO_DISPLAY;EGLSurface surface=EGL_NO_SURFACE;EGLContext context=EGL_NO_CONTEXT;
    explicit MotionProbeEgl(int clientVersion=2) {
        display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(display==EGL_NO_DISPLAY||!eglInitialize(display,nullptr,nullptr))throw std::runtime_error("EGL initialization failed");
        const EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
        EGLConfig config;EGLint count;
        if(!eglChooseConfig(display,attrs,&config,1,&count)||count!=1)throw std::runtime_error("EGL config failed");
        const EGLint pbuffer[]={EGL_WIDTH,1,EGL_HEIGHT,1,EGL_NONE};surface=eglCreatePbufferSurface(display,config,pbuffer);
        const EGLint version[]={EGL_CONTEXT_CLIENT_VERSION,clientVersion,EGL_NONE};context=eglCreateContext(display,config,EGL_NO_CONTEXT,version);
        if(surface==EGL_NO_SURFACE||context==EGL_NO_CONTEXT||!eglMakeCurrent(display,surface,surface,context))throw std::runtime_error("EGL current context failed");
    }
    ~MotionProbeEgl(){if(display!=EGL_NO_DISPLAY){eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);eglTerminate(display);}}
};
