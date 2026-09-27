#pragma once
#include "tjs.h"
#include "RenderManager.h"
#include <memory>
void TVPReverseRGB(tjs_uint32 *destination,const tjs_uint32 *source,tjs_int length);
namespace studysteady::motion {
struct TextureStats {uint64_t bytes=0;unsigned textures=0,draws=0;};
class TextureBridgeManager : public iTVPRenderManager {
public:
    explicit TextureBridgeManager(bool gles);
    ~TextureBridgeManager() override;
    iTVPTexture2D *CreateTexture2D(const void *,int,unsigned,unsigned,TVPTextureFormat::e,int) override;
    iTVPTexture2D *CreateTexture2D(tTVPBitmap *) override;
    iTVPTexture2D *CreateTexture2D(TJS::tTJSBinaryStream *) override;
    iTVPTexture2D *CreateTexture2D(unsigned,unsigned,iTVPTexture2D *) override;
    iTVPRenderMethod *GetRenderMethod(const char *,uint32_t * = nullptr) override;
    const char *GetName() override;
    bool IsSoftware() override {return !gles_;}
    bool GetRenderStat(unsigned &,uint64_t &) override;
    bool GetTextureStat(iTVPTexture2D *,uint64_t &) override;
    void BeginStencil(iTVPTexture2D *) override;
    void EndStencil() override;
    void SetRenderTarget(iTVPTexture2D *) override;
    int EnumParameterID(const char *) override;
    void SetParameterUInt(int,unsigned) override;
    void SetParameterInt(int,int) override;
    void SetParameterPtr(int,const void *) override;
    void SetParameterFloat(int,float) override;
    void OperateRect(iTVPRenderMethod *,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tRenderTexRectArray &) override;
    void OperateTriangles(iTVPRenderMethod *,int,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tTVPPointD *,const tRenderTexQuadArray &) override;
    void OperatePerspective(iTVPRenderMethod *,int,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tTVPPointD *,const tRenderTexQuadArray &) override;
protected:
    void recordDraw(){++stats_->draws;}
private:
    bool gles_;
    std::shared_ptr<TextureStats> stats_;
};
void useRenderManager(iTVPRenderManager *manager);
unsigned textureGlesName(iTVPTexture2D *texture);
void markTextureGpuWritten(iTVPTexture2D *texture);
}
