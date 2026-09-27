#pragma once
#include <EGL/egl.h>
#include <cstdint>
#include <stdexcept>
namespace studysteady::motion {
class RuntimeEgl {
    EGLDisplay display_=EGL_NO_DISPLAY;
    EGLSurface surface_=EGL_NO_SURFACE;
    EGLContext context_=EGL_NO_CONTEXT;
    bool ownDisplay_=false;
    void close() noexcept {
        if(display_==EGL_NO_DISPLAY)return;
        if(context_!=EGL_NO_CONTEXT)eglMakeCurrent(display_,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        if(context_!=EGL_NO_CONTEXT)eglDestroyContext(display_,context_);
        if(surface_!=EGL_NO_SURFACE)eglDestroySurface(display_,surface_);
        if(ownDisplay_)eglTerminate(display_);
        display_=EGL_NO_DISPLAY;context_=EGL_NO_CONTEXT;surface_=EGL_NO_SURFACE;
    }
public:
    RuntimeEgl(uintptr_t display,uintptr_t shared) {
        if(bool(display)!=bool(shared))throw std::runtime_error("supply both parent EGL handles or neither");
        try {
            const auto parent=reinterpret_cast<EGLContext>(shared);
            display_=display?reinterpret_cast<EGLDisplay>(display):eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if(display_==EGL_NO_DISPLAY)throw std::runtime_error("motion EGL display unavailable");
            ownDisplay_=!display;
            if(ownDisplay_&&!eglInitialize(display_,nullptr,nullptr))throw std::runtime_error("motion EGL initialize failed");
            if(!eglBindAPI(EGL_OPENGL_ES_API))throw std::runtime_error("motion EGL API binding failed");
            EGLint version=2;
            if(shared&&!eglQueryContext(display_,parent,EGL_CONTEXT_CLIENT_VERSION,&version))throw std::runtime_error("motion EGL parent query failed");
            const EGLint attrs[]={EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};
            EGLConfig config;EGLint count;
            if(!eglChooseConfig(display_,attrs,&config,1,&count)||count!=1)throw std::runtime_error("motion EGL config unavailable");
            const EGLint surfaceAttrs[]={EGL_WIDTH,1,EGL_HEIGHT,1,EGL_NONE};
            surface_=eglCreatePbufferSurface(display_,config,surfaceAttrs);
            const EGLint contextAttrs[]={EGL_CONTEXT_CLIENT_VERSION,version,EGL_NONE};
            context_=eglCreateContext(display_,config,shared?parent:EGL_NO_CONTEXT,contextAttrs);
            if(surface_==EGL_NO_SURFACE||context_==EGL_NO_CONTEXT)throw std::runtime_error("motion shared EGL context creation failed");
            makeCurrent();
        }catch(...){close();throw;}
    }
    ~RuntimeEgl(){close();}
    void makeCurrent(){if(!eglMakeCurrent(display_,surface_,surface_,context_))throw std::runtime_error("motion EGL context cannot be made current");}
};
}
