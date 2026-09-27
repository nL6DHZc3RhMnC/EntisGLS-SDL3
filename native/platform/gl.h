#pragma once

#if defined(STUDYSTEADY_PLATFORM_SDL3)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>
#include <stdexcept>
#include <string>

namespace study::platform::gl {
// Resolve the common GL 2.1 / GLES 2 drawing subset in the current SDL context.
// On desktop GL 2.1 the framebuffer API can use its equivalent EXT entry points.
struct Functions {
    SDL_GLContext context = nullptr;
    decltype(&::glActiveTexture) ActiveTexture = nullptr;
    decltype(&::glAttachShader) AttachShader = nullptr;
    decltype(&::glBindAttribLocation) BindAttribLocation = nullptr;
    decltype(&::glBindBuffer) BindBuffer = nullptr;
    decltype(&::glBindFramebuffer) BindFramebuffer = nullptr;
    decltype(&::glBindRenderbuffer) BindRenderbuffer = nullptr;
    decltype(&::glBindTexture) BindTexture = nullptr;
    decltype(&::glBlendEquationSeparate) BlendEquationSeparate = nullptr;
    decltype(&::glBlendFuncSeparate) BlendFuncSeparate = nullptr;
    decltype(&::glBufferData) BufferData = nullptr;
    decltype(&::glCheckFramebufferStatus) CheckFramebufferStatus = nullptr;
    decltype(&::glClear) Clear = nullptr;
    decltype(&::glClearColor) ClearColor = nullptr;
    decltype(&::glClearStencil) ClearStencil = nullptr;
    decltype(&::glColorMask) ColorMask = nullptr;
    decltype(&::glCompileShader) CompileShader = nullptr;
    decltype(&::glCreateProgram) CreateProgram = nullptr;
    decltype(&::glCreateShader) CreateShader = nullptr;
    decltype(&::glDeleteBuffers) DeleteBuffers = nullptr;
    decltype(&::glDeleteFramebuffers) DeleteFramebuffers = nullptr;
    decltype(&::glDeleteProgram) DeleteProgram = nullptr;
    decltype(&::glDeleteRenderbuffers) DeleteRenderbuffers = nullptr;
    decltype(&::glDeleteShader) DeleteShader = nullptr;
    decltype(&::glDeleteTextures) DeleteTextures = nullptr;
    decltype(&::glDepthMask) DepthMask = nullptr;
    decltype(&::glDisable) Disable = nullptr;
    decltype(&::glDrawArrays) DrawArrays = nullptr;
    decltype(&::glEnable) Enable = nullptr;
    decltype(&::glEnableVertexAttribArray) EnableVertexAttribArray = nullptr;
    decltype(&::glFinish) Finish = nullptr;
    decltype(&::glFramebufferRenderbuffer) FramebufferRenderbuffer = nullptr;
    decltype(&::glFramebufferTexture2D) FramebufferTexture2D = nullptr;
    decltype(&::glGenBuffers) GenBuffers = nullptr;
    decltype(&::glGenFramebuffers) GenFramebuffers = nullptr;
    decltype(&::glGenRenderbuffers) GenRenderbuffers = nullptr;
    decltype(&::glGenTextures) GenTextures = nullptr;
    decltype(&::glGetError) GetError = nullptr;
    decltype(&::glGetIntegerv) GetIntegerv = nullptr;
    decltype(&::glGetProgramInfoLog) GetProgramInfoLog = nullptr;
    decltype(&::glGetProgramiv) GetProgramiv = nullptr;
    decltype(&::glGetShaderInfoLog) GetShaderInfoLog = nullptr;
    decltype(&::glGetShaderiv) GetShaderiv = nullptr;
    decltype(&::glGetString) GetString = nullptr;
    decltype(&::glGetUniformLocation) GetUniformLocation = nullptr;
    decltype(&::glLinkProgram) LinkProgram = nullptr;
    decltype(&::glPixelStorei) PixelStorei = nullptr;
    decltype(&::glReadPixels) ReadPixels = nullptr;
    decltype(&::glRenderbufferStorage) RenderbufferStorage = nullptr;
    decltype(&::glScissor) Scissor = nullptr;
    decltype(&::glShaderSource) ShaderSource = nullptr;
    decltype(&::glStencilFunc) StencilFunc = nullptr;
    decltype(&::glStencilMask) StencilMask = nullptr;
    decltype(&::glStencilOp) StencilOp = nullptr;
    decltype(&::glTexImage2D) TexImage2D = nullptr;
    decltype(&::glTexParameteri) TexParameteri = nullptr;
    decltype(&::glUniform1f) Uniform1f = nullptr;
    decltype(&::glUniform1i) Uniform1i = nullptr;
    decltype(&::glUniform2f) Uniform2f = nullptr;
    decltype(&::glUniform4f) Uniform4f = nullptr;
    decltype(&::glUseProgram) UseProgram = nullptr;
    decltype(&::glVertexAttribPointer) VertexAttribPointer = nullptr;
    decltype(&::glViewport) Viewport = nullptr;
    template<class Fn> static Fn Load(const char* name, bool framebufferExtension = false) {
        auto address = SDL_GL_GetProcAddress(name);
        if (!address && framebufferExtension) address = SDL_GL_GetProcAddress((std::string(name) + "EXT").c_str());
        if (!address) throw std::runtime_error(std::string("Required GL entry unavailable: ") + name);
        return reinterpret_cast<Fn>(address);
    }
    void Initialize(SDL_GLContext current) {
        if (!current) throw std::runtime_error("GL operations require a current SDL context");
        ActiveTexture = Load<decltype(ActiveTexture)>("glActiveTexture", false);
        AttachShader = Load<decltype(AttachShader)>("glAttachShader", false);
        BindAttribLocation = Load<decltype(BindAttribLocation)>("glBindAttribLocation", false);
        BindBuffer = Load<decltype(BindBuffer)>("glBindBuffer", false);
        BindFramebuffer = Load<decltype(BindFramebuffer)>("glBindFramebuffer", true);
        BindRenderbuffer = Load<decltype(BindRenderbuffer)>("glBindRenderbuffer", true);
        BindTexture = Load<decltype(BindTexture)>("glBindTexture", false);
        BlendEquationSeparate = Load<decltype(BlendEquationSeparate)>("glBlendEquationSeparate", false);
        BlendFuncSeparate = Load<decltype(BlendFuncSeparate)>("glBlendFuncSeparate", false);
        BufferData = Load<decltype(BufferData)>("glBufferData", false);
        CheckFramebufferStatus = Load<decltype(CheckFramebufferStatus)>("glCheckFramebufferStatus", true);
        Clear = Load<decltype(Clear)>("glClear", false);
        ClearColor = Load<decltype(ClearColor)>("glClearColor", false);
        ClearStencil = Load<decltype(ClearStencil)>("glClearStencil", false);
        ColorMask = Load<decltype(ColorMask)>("glColorMask", false);
        CompileShader = Load<decltype(CompileShader)>("glCompileShader", false);
        CreateProgram = Load<decltype(CreateProgram)>("glCreateProgram", false);
        CreateShader = Load<decltype(CreateShader)>("glCreateShader", false);
        DeleteBuffers = Load<decltype(DeleteBuffers)>("glDeleteBuffers", false);
        DeleteFramebuffers = Load<decltype(DeleteFramebuffers)>("glDeleteFramebuffers", true);
        DeleteProgram = Load<decltype(DeleteProgram)>("glDeleteProgram", false);
        DeleteRenderbuffers = Load<decltype(DeleteRenderbuffers)>("glDeleteRenderbuffers", true);
        DeleteShader = Load<decltype(DeleteShader)>("glDeleteShader", false);
        DeleteTextures = Load<decltype(DeleteTextures)>("glDeleteTextures", false);
        DepthMask = Load<decltype(DepthMask)>("glDepthMask", false);
        Disable = Load<decltype(Disable)>("glDisable", false);
        DrawArrays = Load<decltype(DrawArrays)>("glDrawArrays", false);
        Enable = Load<decltype(Enable)>("glEnable", false);
        EnableVertexAttribArray = Load<decltype(EnableVertexAttribArray)>("glEnableVertexAttribArray", false);
        Finish = Load<decltype(Finish)>("glFinish", false);
        FramebufferRenderbuffer = Load<decltype(FramebufferRenderbuffer)>("glFramebufferRenderbuffer", true);
        FramebufferTexture2D = Load<decltype(FramebufferTexture2D)>("glFramebufferTexture2D", true);
        GenBuffers = Load<decltype(GenBuffers)>("glGenBuffers", false);
        GenFramebuffers = Load<decltype(GenFramebuffers)>("glGenFramebuffers", true);
        GenRenderbuffers = Load<decltype(GenRenderbuffers)>("glGenRenderbuffers", true);
        GenTextures = Load<decltype(GenTextures)>("glGenTextures", false);
        GetError = Load<decltype(GetError)>("glGetError", false);
        GetIntegerv = Load<decltype(GetIntegerv)>("glGetIntegerv", false);
        GetProgramInfoLog = Load<decltype(GetProgramInfoLog)>("glGetProgramInfoLog", false);
        GetProgramiv = Load<decltype(GetProgramiv)>("glGetProgramiv", false);
        GetShaderInfoLog = Load<decltype(GetShaderInfoLog)>("glGetShaderInfoLog", false);
        GetShaderiv = Load<decltype(GetShaderiv)>("glGetShaderiv", false);
        GetString = Load<decltype(GetString)>("glGetString", false);
        GetUniformLocation = Load<decltype(GetUniformLocation)>("glGetUniformLocation", false);
        LinkProgram = Load<decltype(LinkProgram)>("glLinkProgram", false);
        PixelStorei = Load<decltype(PixelStorei)>("glPixelStorei", false);
        ReadPixels = Load<decltype(ReadPixels)>("glReadPixels", false);
        RenderbufferStorage = Load<decltype(RenderbufferStorage)>("glRenderbufferStorage", true);
        Scissor = Load<decltype(Scissor)>("glScissor", false);
        ShaderSource = Load<decltype(ShaderSource)>("glShaderSource", false);
        StencilFunc = Load<decltype(StencilFunc)>("glStencilFunc", false);
        StencilMask = Load<decltype(StencilMask)>("glStencilMask", false);
        StencilOp = Load<decltype(StencilOp)>("glStencilOp", false);
        TexImage2D = Load<decltype(TexImage2D)>("glTexImage2D", false);
        TexParameteri = Load<decltype(TexParameteri)>("glTexParameteri", false);
        Uniform1f = Load<decltype(Uniform1f)>("glUniform1f", false);
        Uniform1i = Load<decltype(Uniform1i)>("glUniform1i", false);
        Uniform2f = Load<decltype(Uniform2f)>("glUniform2f", false);
        Uniform4f = Load<decltype(Uniform4f)>("glUniform4f", false);
        UseProgram = Load<decltype(UseProgram)>("glUseProgram", false);
        VertexAttribPointer = Load<decltype(VertexAttribPointer)>("glVertexAttribPointer", false);
        Viewport = Load<decltype(Viewport)>("glViewport", false);
        context = current;
    }
};
inline Functions& Get() {
    thread_local Functions functions;
    const auto current = SDL_GL_GetCurrentContext();
    if (!current || current != functions.context) functions.Initialize(current);
    return functions;
}
} // namespace study::platform::gl

#define glActiveTexture (::study::platform::gl::Get().ActiveTexture)
#define glAttachShader (::study::platform::gl::Get().AttachShader)
#define glBindAttribLocation (::study::platform::gl::Get().BindAttribLocation)
#define glBindBuffer (::study::platform::gl::Get().BindBuffer)
#define glBindFramebuffer (::study::platform::gl::Get().BindFramebuffer)
#define glBindRenderbuffer (::study::platform::gl::Get().BindRenderbuffer)
#define glBindTexture (::study::platform::gl::Get().BindTexture)
#define glBlendEquationSeparate (::study::platform::gl::Get().BlendEquationSeparate)
#define glBlendFuncSeparate (::study::platform::gl::Get().BlendFuncSeparate)
#define glBufferData (::study::platform::gl::Get().BufferData)
#define glCheckFramebufferStatus (::study::platform::gl::Get().CheckFramebufferStatus)
#define glClear (::study::platform::gl::Get().Clear)
#define glClearColor (::study::platform::gl::Get().ClearColor)
#define glClearStencil (::study::platform::gl::Get().ClearStencil)
#define glColorMask (::study::platform::gl::Get().ColorMask)
#define glCompileShader (::study::platform::gl::Get().CompileShader)
#define glCreateProgram (::study::platform::gl::Get().CreateProgram)
#define glCreateShader (::study::platform::gl::Get().CreateShader)
#define glDeleteBuffers (::study::platform::gl::Get().DeleteBuffers)
#define glDeleteFramebuffers (::study::platform::gl::Get().DeleteFramebuffers)
#define glDeleteProgram (::study::platform::gl::Get().DeleteProgram)
#define glDeleteRenderbuffers (::study::platform::gl::Get().DeleteRenderbuffers)
#define glDeleteShader (::study::platform::gl::Get().DeleteShader)
#define glDeleteTextures (::study::platform::gl::Get().DeleteTextures)
#define glDepthMask (::study::platform::gl::Get().DepthMask)
#define glDisable (::study::platform::gl::Get().Disable)
#define glDrawArrays (::study::platform::gl::Get().DrawArrays)
#define glEnable (::study::platform::gl::Get().Enable)
#define glEnableVertexAttribArray (::study::platform::gl::Get().EnableVertexAttribArray)
#define glFinish (::study::platform::gl::Get().Finish)
#define glFramebufferRenderbuffer (::study::platform::gl::Get().FramebufferRenderbuffer)
#define glFramebufferTexture2D (::study::platform::gl::Get().FramebufferTexture2D)
#define glGenBuffers (::study::platform::gl::Get().GenBuffers)
#define glGenFramebuffers (::study::platform::gl::Get().GenFramebuffers)
#define glGenRenderbuffers (::study::platform::gl::Get().GenRenderbuffers)
#define glGenTextures (::study::platform::gl::Get().GenTextures)
#define glGetError (::study::platform::gl::Get().GetError)
#define glGetIntegerv (::study::platform::gl::Get().GetIntegerv)
#define glGetProgramInfoLog (::study::platform::gl::Get().GetProgramInfoLog)
#define glGetProgramiv (::study::platform::gl::Get().GetProgramiv)
#define glGetShaderInfoLog (::study::platform::gl::Get().GetShaderInfoLog)
#define glGetShaderiv (::study::platform::gl::Get().GetShaderiv)
#define glGetString (::study::platform::gl::Get().GetString)
#define glGetUniformLocation (::study::platform::gl::Get().GetUniformLocation)
#define glLinkProgram (::study::platform::gl::Get().LinkProgram)
#define glPixelStorei (::study::platform::gl::Get().PixelStorei)
#define glReadPixels (::study::platform::gl::Get().ReadPixels)
#define glRenderbufferStorage (::study::platform::gl::Get().RenderbufferStorage)
#define glScissor (::study::platform::gl::Get().Scissor)
#define glShaderSource (::study::platform::gl::Get().ShaderSource)
#define glStencilFunc (::study::platform::gl::Get().StencilFunc)
#define glStencilMask (::study::platform::gl::Get().StencilMask)
#define glStencilOp (::study::platform::gl::Get().StencilOp)
#define glTexImage2D (::study::platform::gl::Get().TexImage2D)
#define glTexParameteri (::study::platform::gl::Get().TexParameteri)
#define glUniform1f (::study::platform::gl::Get().Uniform1f)
#define glUniform1i (::study::platform::gl::Get().Uniform1i)
#define glUniform2f (::study::platform::gl::Get().Uniform2f)
#define glUniform4f (::study::platform::gl::Get().Uniform4f)
#define glUseProgram (::study::platform::gl::Get().UseProgram)
#define glVertexAttribPointer (::study::platform::gl::Get().VertexAttribPointer)
#define glViewport (::study::platform::gl::Get().Viewport)
#else
#include <GLES2/gl2.h>
#endif
