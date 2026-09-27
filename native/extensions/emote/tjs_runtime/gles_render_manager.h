#pragma once
#include "extensions/emote/tjs_runtime/texture_bridge.h"
#include <vector>
namespace studysteady::motion {
class GlesRenderManager final : public TextureBridgeManager {
public:
    GlesRenderManager();
    ~GlesRenderManager() override;
    iTVPRenderMethod *GetRenderMethod(const char *,uint32_t * = nullptr) override;
    void SetRenderTarget(iTVPTexture2D *) override;
    void BeginStencil(iTVPTexture2D *) override;
    void EndStencil() override;
    void OperateTriangles(iTVPRenderMethod *,int,iTVPTexture2D *,iTVPTexture2D *,const tTVPRect &,const tTVPPointD *,const tRenderTexQuadArray &) override;
    void clearTarget(iTVPTexture2D *target);
    // Top-to-bottom RGBA8 readback of a rendered target.
    std::vector<uint8_t> readTarget(iTVPTexture2D *target);
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
