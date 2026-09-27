#include "extensions/emote/tjs_runtime/texture_bridge.h"
#include "extensions/emote/tjs_runtime/gl_capabilities.h"
#include <vector>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
#include "platform/gl.h"
#endif
namespace {
iTVPRenderManager *currentManager=nullptr;
[[noreturn]] void unsupported(const char *operation) {
    throw std::runtime_error(std::string("motion render adapter does not yet implement ")+operation);
}
class Texture final : public iTVPTexture2D {
    std::vector<uint8_t> pixels_;
    std::shared_ptr<studysteady::motion::TextureStats> stats_;
    bool gles_,static_,gpuWritten_=false;
    unsigned texture_=0;
    void requireCpuPixels() const {if(gpuWritten_)unsupported("CPU access to a GPU-written texture; use explicit target readback");}
    void checkRect(const tTVPRect &r) const {
        if(r.left<0||r.top<0||r.right<r.left||r.bottom<r.top||r.right>Width||r.bottom>Height)
            throw std::runtime_error("motion texture rectangle out of bounds");
    }
    void upload() {
        if(!gles_)return;
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(!glGetString(GL_VERSION))throw std::runtime_error("motion texture upload requires current GLES context");
        GLint previous,alignment;glGetIntegerv(GL_TEXTURE_BINDING_2D,&previous);glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
        if(!texture_)glGenTextures(1,&texture_);
        glBindTexture(GL_TEXTURE_2D,texture_);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        // Original TVPTextureFormat::RGBA -> GL_RGBA uploads bytes directly.
        // PlayerResource has already applied its Win-PSB channel conversion.
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Width,Height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels_.data());
        const auto error=glGetError();glBindTexture(GL_TEXTURE_2D,previous);glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);
        if(error!=GL_NO_ERROR)throw std::runtime_error("motion texture GLES upload failed: "+std::to_string(error));
#else
        throw std::runtime_error("GL textures are not enabled in this target");
#endif
    }
public:
    Texture(const void *pixels,int pitch,unsigned w,unsigned h,bool gles,bool isStatic,
            std::shared_ptr<studysteady::motion::TextureStats> stats):iTVPTexture2D(w,h),stats_(std::move(stats)),gles_(gles),static_(isStatic) {
        if(!w||!h||w>16384||h>16384||uint64_t(w)*h*4>256*1024*1024)
            throw std::runtime_error("motion texture dimensions out of range");
        if(pixels && pitch<int(w*4))throw std::runtime_error("motion texture pitch too short");
        pixels_.resize(size_t(w)*h*4);
        if(pixels)for(unsigned y=0;y<h;++y)std::memcpy(pixels_.data()+size_t(y)*w*4,static_cast<const uint8_t *>(pixels)+size_t(y)*pitch,w*4);
        try {upload();}catch(...){
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
            if(texture_)glDeleteTextures(1,&texture_);
#endif
            throw;
        }
        stats_->bytes+=pixels_.size();++stats_->textures;
    }
    ~Texture() override {
#if defined(STUDYSTEADY_MOTION_GL_ENABLED)
        if(texture_)glDeleteTextures(1,&texture_);
#endif
        stats_->bytes-=pixels_.size();--stats_->textures;
    }
    TVPTextureFormat::e GetFormat() const override{return TVPTextureFormat::RGBA;}
    const void *GetScanLineForRead(tjs_uint y) override {requireCpuPixels();if(y>=unsigned(Height))throw std::runtime_error("texture row out of range");return pixels_.data()+size_t(y)*Width*4;}
    void *GetScanLineForWrite(tjs_uint) override {unsupported("direct writable texture rows; use Update");}
    tjs_int GetPitch() const override{return Width*4;}
    void SetSize(unsigned,unsigned) override{unsupported("texture resize");}
    void Update(const void *p,TVPTextureFormat::e f,int pitch,const tTVPRect &r) override {
        requireCpuPixels();
        checkRect(r);if(f!=TVPTextureFormat::RGBA||!p||pitch<(r.right-r.left)*4)throw std::runtime_error("invalid texture update");
        for(int y=r.top;y<r.bottom;++y)std::memcpy(pixels_.data()+(size_t(y)*Width+r.left)*4,static_cast<const uint8_t *>(p)+size_t(y-r.top)*pitch,size_t(r.right-r.left)*4);
        upload();
    }
    uint32_t GetPoint(int x,int y) override {requireCpuPixels();checkRect({x,y,x+1,y+1});uint32_t out;std::memcpy(&out,pixels_.data()+(size_t(y)*Width+x)*4,4);return out;}
    void SetPoint(int x,int y,uint32_t c) override {Update(&c,TVPTextureFormat::RGBA,4,{x,y,x+1,y+1});}
    bool IsStatic() override{return static_;}
    bool IsOpaque() override {requireCpuPixels();for(size_t i=3;i<pixels_.size();i+=4)if(pixels_[i]!=255)return false;return true;}
    cocos2d::Texture2D *GetAdapterTexture(cocos2d::Texture2D *) override {unsupported("Cocos texture conversion");}
    unsigned glName() const {if(!gles_||!texture_)throw std::runtime_error("texture has no GLES allocation");return texture_;}
    void markGpuWritten(){(void)glName();gpuWritten_=true;}
};
}
void iTVPTexture2D::Release(){if(--RefCount==0)delete this;}
iTVPRenderMethod *iTVPRenderManager::GetRenderMethod(const char *,uint32_t *) {unsupported("base render method lookup");}
iTVPRenderManager *TVPGetRenderManager(){if(!currentManager)throw std::runtime_error("motion render manager not initialized");return currentManager;}
iTVPRenderManager *TVPGetRenderManager(const ttstr &name){if(name!=TJS_W("opengl"))unsupported("named render manager");return TVPGetRenderManager();}
bool TVPIsSoftwareRenderManager(){return TVPGetRenderManager()->IsSoftware();}
void TVPReverseRGB(tjs_uint32 *d,const tjs_uint32 *s,tjs_int n){for(tjs_int i=0;i<n;++i){const auto v=s[i];d[i]=(v&0xff00ff00u)|((v&255u)<<16)|((v>>16)&255u);}}
namespace studysteady::motion {
TextureBridgeManager::TextureBridgeManager(bool gles):gles_(gles),stats_(std::make_shared<TextureStats>()){}
TextureBridgeManager::~TextureBridgeManager(){if(currentManager==this)currentManager=nullptr;}
iTVPTexture2D *TextureBridgeManager::CreateTexture2D(const void *p,int pitch,unsigned w,unsigned h,TVPTextureFormat::e format,int flags){if(format!=TVPTextureFormat::RGBA)unsupported("non-RGBA textures");return new Texture(p,pitch,w,h,gles_,bool(flags&RENDER_CREATE_TEXTURE_FLAG_STATIC),stats_);}
iTVPTexture2D *TextureBridgeManager::CreateTexture2D(tTVPBitmap *){unsupported("Bitmap texture import");}
iTVPTexture2D *TextureBridgeManager::CreateTexture2D(TJS::tTJSBinaryStream *){unsupported("compressed texture streams");}
iTVPTexture2D *TextureBridgeManager::CreateTexture2D(unsigned w,unsigned h,iTVPTexture2D *src){
    if(!src||src->GetFormat()!=TVPTextureFormat::RGBA)unsupported("texture copy format");
    std::vector<uint32_t> pixels(size_t(w)*h);
    for(unsigned y=0;y<std::min(h,src->GetHeight());++y)std::memcpy(pixels.data()+size_t(y)*w,src->GetScanLineForRead(y),std::min(w,src->GetWidth())*4);
    return CreateTexture2D(pixels.data(),w*4,w,h,TVPTextureFormat::RGBA,0);
}
iTVPRenderMethod *TextureBridgeManager::GetRenderMethod(const char *,uint32_t *){unsupported("shader render methods");}
const char *TextureBridgeManager::GetName(){return gles_?"StudySteady GLES texture adapter":"StudySteady CPU texture adapter";}
bool TextureBridgeManager::GetRenderStat(unsigned &draws,uint64_t &bytes){draws=stats_->draws;bytes=stats_->bytes;return true;}
bool TextureBridgeManager::GetTextureStat(iTVPTexture2D *texture,uint64_t &bytes){if(!dynamic_cast<Texture *>(texture))return false;bytes=uint64_t(texture->GetWidth())*texture->GetHeight()*4;return true;}
void TextureBridgeManager::BeginStencil(iTVPTexture2D *){unsupported("stencil begin");}
void TextureBridgeManager::EndStencil(){unsupported("stencil end");}
void TextureBridgeManager::SetRenderTarget(iTVPTexture2D *){unsupported("render target binding");}
int TextureBridgeManager::EnumParameterID(const char *){unsupported("render parameter lookup");}
void TextureBridgeManager::SetParameterUInt(int,unsigned){unsupported("uint render parameter");}
void TextureBridgeManager::SetParameterInt(int,int){unsupported("int render parameter");}
void TextureBridgeManager::SetParameterPtr(int,const void *){unsupported("pointer render parameter");}
void TextureBridgeManager::SetParameterFloat(int,float){unsupported("float render parameter");}
void TextureBridgeManager::OperateRect(iTVPRenderMethod *,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tRenderTexRectArray &){unsupported("rect draw");}
void TextureBridgeManager::OperateTriangles(iTVPRenderMethod *,int,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tTVPPointD *,const tRenderTexQuadArray &){unsupported("triangle draw");}
void TextureBridgeManager::OperatePerspective(iTVPRenderMethod *,int,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tTVPPointD *,const tRenderTexQuadArray &){unsupported("perspective draw");}
void useRenderManager(iTVPRenderManager *manager){currentManager=manager;}
unsigned textureGlesName(iTVPTexture2D *texture){auto *t=dynamic_cast<Texture *>(texture);if(!t)throw std::runtime_error("not a motion bridge texture");return t->glName();}
void markTextureGpuWritten(iTVPTexture2D *texture){auto *t=dynamic_cast<Texture *>(texture);if(!t)throw std::runtime_error("not a motion bridge texture");t->markGpuWritten();}
}
