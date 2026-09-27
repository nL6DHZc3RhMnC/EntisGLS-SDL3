#include "gles_render_manager.h"
#include "platform/gl.h"
#include <cstring>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <stdexcept>
#include <string>

namespace studysteady::motion {
namespace {
void check(const char *operation){const auto e=glGetError();if(e)throw std::runtime_error(std::string(operation)+": GLES error "+std::to_string(e));}
GLuint shader(GLenum kind,const char *source) {
    const auto* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!version) throw std::runtime_error("Shader compilation requires a current GL context");
    const bool embedded = std::strstr(version, "OpenGL ES") != nullptr;
    const std::string adapted = std::string(embedded ? "#version 100\n" : "#version 120\n")
        + (embedded && kind == GL_FRAGMENT_SHADER ? "precision mediump float;\n" : "") + source;
    source = adapted.c_str();
    const auto value=glCreateShader(kind);glShaderSource(value,1,&source,nullptr);glCompileShader(value);
    GLint ok;glGetShaderiv(value,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[2048]={};glGetShaderInfoLog(value,sizeof(log),nullptr,log);glDeleteShader(value);throw std::runtime_error(log);}
    return value;
}
GLuint makeProgram() {
    const char *vertex=R"(attribute vec2 position;attribute vec2 uv;uniform vec2 resolution;varying vec2 texcoord;
void main(){gl_Position=vec4(position.x/resolution.x*2.0-1.0,1.0-position.y/resolution.y*2.0,0.0,1.0);texcoord=uv;})";
    // Exact color/alpha/multiply expressions and fixed blend factors from
    // imported RenderManager_ogl_reference.cpp (AlphaBlend_color[_a] and
    // PsMulBlend_color[_AlphaTest]). Source coordinates/platform setup are new.
    const char *fragment=R"(uniform sampler2D sourceTexture;uniform vec4 color;
uniform float alpha_threshold;uniform int multiplyMode;varying vec2 texcoord;
void main(){vec4 s=texture2D(sourceTexture,texcoord);s*=color;
if(s.a<alpha_threshold)discard;if(multiplyMode!=0){s.rgb*=s.a;s.rgb+=1.0-s.a;}gl_FragColor=s;})";
    GLuint v=shader(GL_VERTEX_SHADER,vertex),f=0,program=0;
    try {f=shader(GL_FRAGMENT_SHADER,fragment);program=glCreateProgram();glAttachShader(program,v);glAttachShader(program,f);
        glBindAttribLocation(program,0,"position");glBindAttribLocation(program,1,"uv");glLinkProgram(program);
        GLint ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok){char log[2048]={};glGetProgramInfoLog(program,sizeof(log),nullptr,log);throw std::runtime_error(log);}
    }catch(...){glDeleteShader(v);if(f)glDeleteShader(f);if(program)glDeleteProgram(program);throw;}
    glDeleteShader(v);glDeleteShader(f);return program;
}
class Method final : public iTVPRenderMethod {
public:
    enum Kind {AlphaKeep,AlphaAdd,Multiply};
    Kind kind;bool alphaTest;uint32_t color=0xffffffffu;int threshold=0;
    explicit Method(const std::string &name):kind(AlphaAdd),alphaTest(false) {
        SetName(name);auto base=name;const std::string suffix="_AlphaTest";
        if(base.size()>suffix.size() && base.compare(base.size()-suffix.size(),suffix.size(),suffix)==0){alphaTest=true;base.resize(base.size()-suffix.size());}
        if(base=="AlphaBlend_color_a")kind=AlphaAdd;
        else if(base=="AlphaBlend_color")kind=AlphaKeep;
        else if(base=="PsMulBlend_color")kind=Multiply;
        else throw std::runtime_error("GLES blend method not yet implemented: "+name);
    }
    ~Method() override=default;
    int EnumParameterID(const char *name) override {
        if(std::string(name)=="color")return 0;if(std::string(name)=="alpha_threshold")return 1;
        throw std::runtime_error("unknown motion shader parameter");
    }
    void SetParameterColor4B(int id,unsigned value) override{if(id!=0)throw std::runtime_error("invalid shader color id");color=value;}
    void SetParameterOpa(int id,int value) override{if(id!=1)throw std::runtime_error("invalid shader alpha id");threshold=std::clamp(value,0,255);}
    void SetParameterUInt(int,unsigned) override{throw std::runtime_error("unsupported shader uint parameter");}
    void SetParameterInt(int,int) override{throw std::runtime_error("unsupported shader int parameter");}
    void SetParameterPtr(int,const void *) override{throw std::runtime_error("unsupported shader pointer parameter");}
    void SetParameterFloat(int,float) override{throw std::runtime_error("unsupported shader float parameter");}
    void SetParameterFloatArray(int,float *,int) override{throw std::runtime_error("unsupported shader array parameter");}
    iTVPRenderMethod *SetBlendFuncSeparate(int,int,int,int,int) override{throw std::runtime_error("unsupported runtime shader blend override");}
    bool IsBlendTarget() override{return false;}
};
}
struct GlesRenderManager::State {
    GLuint program=0,framebuffer=0,stencil=0,vertexBuffer=0;
    int width=0,height=0;
    std::unordered_map<std::string,std::unique_ptr<Method>> methods;
    ~State(){if(vertexBuffer)glDeleteBuffers(1,&vertexBuffer);if(stencil)glDeleteRenderbuffers(1,&stencil);if(framebuffer)glDeleteFramebuffers(1,&framebuffer);if(program)glDeleteProgram(program);}
};
GlesRenderManager::GlesRenderManager():TextureBridgeManager(true),state_(std::make_unique<State>()) {
    if(!glGetString(GL_VERSION))throw std::runtime_error("Motion renderer requires a current GL context");
    state_->program=makeProgram();glGenFramebuffers(1,&state_->framebuffer);glGenRenderbuffers(1,&state_->stencil);glGenBuffers(1,&state_->vertexBuffer);check("create GLES manager");
}
GlesRenderManager::~GlesRenderManager()=default;
iTVPRenderMethod *GlesRenderManager::GetRenderMethod(const char *name,uint32_t *) {
    auto &m=state_->methods[name];if(!m)m=std::make_unique<Method>(name);return m.get();
}
void GlesRenderManager::SetRenderTarget(iTVPTexture2D *target) {
    if(!target)throw std::runtime_error("null GLES render target");
    glBindFramebuffer(GL_FRAMEBUFFER,state_->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textureGlesName(target),0);
    const int w=target->GetWidth(),h=target->GetHeight();
    if(w!=state_->width||h!=state_->height){glBindRenderbuffer(GL_RENDERBUFFER,state_->stencil);glRenderbufferStorage(GL_RENDERBUFFER,GL_STENCIL_INDEX8,w,h);state_->width=w;state_->height=h;}
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_RENDERBUFFER,state_->stencil);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("GLES color/stencil target incomplete");
    glViewport(0,0,w,h);check("set GLES target");
}
void GlesRenderManager::BeginStencil(iTVPTexture2D *target){SetRenderTarget(target);}
void GlesRenderManager::EndStencil(){glDisable(GL_STENCIL_TEST);glStencilMask(255);check("end stencil");}
void GlesRenderManager::clearTarget(iTVPTexture2D *target){SetRenderTarget(target);markTextureGpuWritten(target);glDisable(GL_SCISSOR_TEST);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glClearColor(0,0,0,0);glClear(GL_COLOR_BUFFER_BIT);check("clear target");}
void GlesRenderManager::OperateTriangles(iTVPRenderMethod *method,int triangles,iTVPTexture2D *target,iTVPTexture2D *,const tTVPRect &clip,const tTVPPointD *positions,const tRenderTexQuadArray &textures) {
    auto *m=dynamic_cast<Method *>(method);
    if(!m||triangles<1||!positions||textures.size()!=1||!textures[0].first||!textures[0].second)
        throw std::runtime_error("invalid GLES triangle submission");
    auto *source=textures[0].first;if(source==target)throw std::runtime_error("GLES texture feedback is unsupported");
    SetRenderTarget(target);glUseProgram(state_->program);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
    if(m->kind==Method::Multiply)glBlendFuncSeparate(GL_ZERO,GL_SRC_COLOR,GL_ZERO,GL_ONE);
    else glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,m->kind==Method::AlphaAdd?GL_ONE:GL_ZERO,m->kind==Method::AlphaAdd?GL_ONE_MINUS_SRC_ALPHA:GL_ONE);
    const int l=std::clamp(clip.left,0,state_->width),r=std::clamp(clip.right,0,state_->width);
    const int t=std::clamp(clip.top,0,state_->height),b=std::clamp(clip.bottom,0,state_->height);
    if(l>=r||t>=b)return;
    glEnable(GL_SCISSOR_TEST);glScissor(l,state_->height-b,r-l,b-t);
    glUniform2f(glGetUniformLocation(state_->program,"resolution"),state_->width,state_->height);
    glUniform4f(glGetUniformLocation(state_->program,"color"),((m->color>>16)&255)/255.0f,((m->color>>8)&255)/255.0f,(m->color&255)/255.0f,(m->color>>24)/255.0f);
    glUniform1f(glGetUniformLocation(state_->program,"alpha_threshold"),m->alphaTest?m->threshold/255.0f:0.0f);
    glUniform1i(glGetUniformLocation(state_->program,"multiplyMode"),m->kind==Method::Multiply);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,textureGlesName(source));glUniform1i(glGetUniformLocation(state_->program,"sourceTexture"),0);
    std::vector<std::array<float,4>> vertices(static_cast<size_t>(triangles)*3);
    for(size_t i=0;i<vertices.size();++i)vertices[i]={static_cast<float>(positions[i].x),static_cast<float>(positions[i].y),static_cast<float>(textures[0].second[i].x/source->GetWidth()),static_cast<float>(textures[0].second[i].y/source->GetHeight())};
    // Shared parents can be ES3. Use a real buffer instead of ES2 client arrays.
    glBindBuffer(GL_ARRAY_BUFFER,state_->vertexBuffer);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(vertices[0]),vertices.data(),GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(vertices[0]),nullptr);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(vertices[0]),reinterpret_cast<const void *>(2*sizeof(float)));
    markTextureGpuWritten(target);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));check("draw motion triangles");recordDraw();
}
std::vector<uint8_t> GlesRenderManager::readTarget(iTVPTexture2D *target) {
    SetRenderTarget(target);std::vector<uint8_t> pixels(static_cast<size_t>(state_->width)*state_->height*4),out(pixels.size());
    GLint alignment;glGetIntegerv(GL_PACK_ALIGNMENT,&alignment);glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(0,0,state_->width,state_->height,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());glPixelStorei(GL_PACK_ALIGNMENT,alignment);check("read motion target");
    const size_t pitch=state_->width*4;for(int y=0;y<state_->height;++y)std::copy_n(pixels.data()+size_t(state_->height-1-y)*pitch,pitch,out.data()+size_t(y)*pitch);
    return out;
}
}
