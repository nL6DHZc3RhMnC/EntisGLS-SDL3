#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_super_shading.h"
#include "runtime/cotopha_port/legacy_super_shading_math.h"
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include "platform/log.h"
#include <cstring>

struct LegacySuperShadingState::Impl {
    SSystem::SSmartReference<SakuraGL::SGLSprite> owner;
    uint32_t kind,flags,degree=UINT32_MAX;
    SakuraGL::SGLImage nativeFrame,rgbaFrame,normalized,result;
    LegacySuperShading::Raster raster;
    std::vector<uint8_t> output;
    Impl(SakuraGL::SGLSprite& sprite,const std::array<uint32_t,33>& words):owner(&sprite),kind(words[0]),flags(words[1]){}
};
LegacySuperShadingState::LegacySuperShadingState(SakuraGL::SGLSprite& owner,const std::array<uint32_t,33>& words,uint32_t degree)
    :impl_(new Impl(owner,words)){SetDegree(degree);}
LegacySuperShadingState::~LegacySuperShadingState()=default;
uint32_t LegacySuperShadingState::GetDegree()const{return impl_->degree;}
void LegacySuperShadingState::SetDegree(uint32_t degree) {
    if(impl_->degree==degree)return;impl_->degree=degree;
    if(auto* owner=impl_->owner.GetReference())owner->NotifyUpdate();
}
uint32_t LegacySuperShadingState::GetDrawTransparency(uint32_t normal)const {
    return LegacySuperShading::DrawTransparency(impl_->kind,impl_->flags,impl_->degree,normal);
}
SakuraGL::SGLImageObject* LegacySuperShadingState::Filter(SakuraGL::SGLImageObject* source) {
    using namespace SakuraGL;
    if(!source)return nullptr;
    auto& state=*impl_;SGLImageInfo info{};const char* stage="source-info";
    auto fail=[&](int error)->SGLImageObject* {
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
            "SuperSprite shading failed stage=%s code=%d kind=%u degree=%u source=%ux%u format=%08x depth=%u",
            stage,error,state.kind,state.degree,info.width,info.height,info.format,info.depth);return nullptr;
    };
    if(state.kind!=10&&state.kind!=11)return fail(eslErrNotSupported);
    auto error=source->GetImageInfo(info);if(error)return fail(error);
    const auto color=info.format&formatImageTypeMask;
    if(!info.width||!info.height||uint64_t(info.width)*info.height>33554432)return fail(sglErrInvalidParam);
    if((color!=formatImageRGB&&color!=formatImageBGR&&color!=formatImageGray)||
       (info.format&(formatImageFlagSideBySide|formatImageFlagPalette|formatImageFlagClipping))||
       !info.depth||info.depth>32||(info.depth%8))return fail(eslErrNotSupported);
    auto ensure=[&](SGLImage& image,uint32_t format,uint32_t depth) {
        SGLImageInfo current{};image.GetImageInfo(current);
        if(current.width==info.width&&current.height==info.height&&current.format==format&&current.depth==depth)return sglErrSuccess;
        return image.CreateImage(info.width,info.height,format,depth,SGLImageObject::bufferOnMemory);
    };
    const auto rgbaFormat=formatImageRGB|(info.format&(formatImageFlagAlpha|formatImageFlagNoProductOfAlpha));
    // GLS3 EGL stores alpha-bearing colors premultiplied. Blur those bytes,
    // including alpha, then add light without multiplying RGB by alpha again.
    const auto outputFormat=formatImageRGB|(info.format&formatImageFlagAlpha);
    stage="allocate-native";if((error=ensure(state.nativeFrame,info.format,info.depth)))return fail(error);
    stage="allocate-rgb";if((error=ensure(state.rgbaFrame,rgbaFormat,32)))return fail(error);
    stage="allocate-normalized";if((error=ensure(state.normalized,outputFormat,32)))return fail(error);
    stage="allocate-result";if((error=ensure(state.result,outputFormat,32)))return fail(error);
    struct Lock {
        SGLImageObject& image;int flags;SGLImageBuffer buffer;uint8_t* pixels;
        Lock(SGLImageObject& value,int access):image(value),flags(access),pixels(image.LockBuffer(buffer,flags)){buffer.ptrBuffer=pixels;}
        ~Lock(){if(pixels)image.UnlockBuffer(flags);}
        SGLError Unlock(){if(!pixels)return sglErrFailed;pixels=nullptr;return image.UnlockBuffer(flags);}
    };
    {
        Lock from(state.nativeFrame,SGLImageObject::lockRead|SGLImageObject::lockWrite);
        Lock intermediate(state.rgbaFrame,SGLImageObject::lockRead|SGLImageObject::lockWrite);
        Lock normalized(state.normalized,SGLImageObject::lockRead|SGLImageObject::lockWrite);
        stage="lock-source";if(!from.pixels||!intermediate.pixels||!normalized.pixels)return fail(sglErrFailed);
        stage="read-current";if((error=source->ReadFrameBuffer(from.buffer,from.pixels,source->GetSelectedFrame())))return fail(error);
        stage="convert-channels";if((error=sglConvertImageBuffer(intermediate.buffer,from.buffer)))return fail(error);
        stage="convert-alpha";if((error=sglConvertImageBuffer(normalized.buffer,intermediate.buffer)))return fail(error);
        stage="cache-source";if(!state.raster.SetSource(normalized.pixels,info.width,info.height,normalized.buffer.pitchLine))return fail(sglErrInvalidParam);
        stage="unlock-source";if((error=from.Unlock()))return fail(error);
        if((error=intermediate.Unlock()))return fail(error);if((error=normalized.Unlock()))return fail(error);
    }
    stage="loop421";if(!state.raster.Render(state.kind,state.degree,state.output))return fail(sglErrFailed);
    {
        Lock result(state.result,SGLImageObject::lockWrite);stage="lock-result";if(!result.pixels)return fail(sglErrFailed);
        for(uint32_t y=0;y<info.height;++y)
            std::memcpy(result.pixels+ptrdiff_t(y)*result.buffer.pitchLine,state.output.data()+size_t(y)*info.width*4,size_t(info.width)*4);
        stage="unlock-result";if((error=result.Unlock()))return fail(error);
    }
    return &state.result;
}

bool CheckLegacySuperShading() {
    using namespace SakuraGL;
    struct Guard{Guard(){SSystem::Lock();}~Guard(){SSystem::Unlock();}} guard;
    auto check=[](bool value,const char* detail) {
        study::platform::LogPrint(value?study::platform::LogPriority::Info:study::platform::LogPriority::Error,"StudySteady",
            "SuperSprite shading probe %s: %s",value?"PASS":"FAIL",detail);return value;
    };
    auto pixel=[](SGLImageObject* image,unsigned x,unsigned y,uint32_t& value) {
        if(!image)return false;SGLImageBuffer info;
        auto* data=image->LockBuffer(info,SGLImageObject::lockRead);
        if(!data)return false;
        const bool valid=info.depth==32&&info.pitchPixel==4&&x<info.width&&y<info.height;
        if(valid)std::memcpy(&value,data+ptrdiff_t(y)*info.pitchLine+x*4,4);
        return !image->UnlockBuffer(SGLImageObject::lockRead)&&valid;
    };
    SGLSprite owner;std::array<uint32_t,33> words{};words[0]=10;words[1]=1;
    LegacySuperShadingState shade(owner,words,0);
    SGLImage source;uint32_t value=0;
    if(!check(!source.CreateImage(2,2,formatImageARGB,32,SGLImageObject::bufferOnMemory)&&
        !source.FillImage(SGLPalette(0xff4d4d4d)),"real premultiplied ARGB32 source"))return false;
    if(!check(pixel(shade.Filter(&source),0,0,value)&&value==0xff4d4d4d&&shade.impl_->raster.Passes()==16,
        "zero degree exact source and first 16 blur passes"))return false;
    shade.SetDegree(1);
    if(!check(pixel(shade.Filter(&source),0,0,value)&&value==0xfe4c4c4c&&shade.impl_->raster.Passes()==16,
        "fractional interpolation truncates each product including alpha; cached levels reused"))return false;
    shade.SetDegree(128);
    if(!check(pixel(shade.Filter(&source),0,0,value)&&value==0xff4d4d4d&&shade.GetDrawTransparency(17)==64&&
        shade.impl_->raster.Passes()==48,"half-degree selects level2 and squared transition transparency"))return false;
    const auto revision=shade.impl_->raster.Revision();
    if(!check(!source.FillImage(SGLPalette(0xff314151))&&pixel(shade.Filter(&source),0,0,value)&&value==0xff314151&&
        shade.impl_->raster.Revision()==revision+1,"actual source pixel changes invalidate cached levels"))return false;
    words[0]=11;LegacySuperShadingState light(owner,words,128);
    if(!check(pixel(light.Filter(&source),0,0,value)&&value==0xff708090&&light.GetDrawTransparency(17)==17,
        "ShadingLight saturating RGB addition keeps ordinary transparency"))return false;
    light.SetDegree(256);
    if(!check(pixel(light.Filter(&source),0,0,value)&&value==0xffffffff,
        "completed ShadingLight reaches white"))return false;
    if(!check(pixel(&source,0,0,value)&&value==0xff314151,"blur and light never modify source pixels"))return false;

    // A transparent straight-alpha frame must be converted before smoothing;
    // premultiplying a result for a second time would make these bytes darker.
    SGLImage abgr;
    if(!check(!abgr.CreateImage(1,1,formatImageABGR|formatImageFlagNoProductOfAlpha,32,SGLImageObject::bufferOnMemory)&&
        !abgr.FillImage(SGLPalette(0x80204060)),"straight ABGR32 input"))return false;
    shade.SetDegree(0);
    if(!check(pixel(shade.Filter(&abgr),0,0,value)&&value==0x80302010,
        "ABGR channel order and straight alpha convert once to premultiplied ARGB"))return false;
    light.SetDegree(128);
    if(!check(pixel(light.Filter(&abgr),0,0,value)&&value==0x806f5f4f,
        "light keeps partial alpha and adds color after premultiplication"))return false;

    class ReadOnlyImage final:public SGLImage {
    public:
        size_t reads=0,lastFrame=SIZE_MAX;
        uint8_t* LockBuffer(SGLImageInfo&,int,const SGLImageRect*) override {return nullptr;}
        SGLError ReadFrameBuffer(const SGLImageInfo& info,uint8_t* pixels,size_t frame,int side) override {
            ++reads;lastFrame=frame;return SGLImage::ReadFrameBuffer(info,pixels,frame,side);
        }
    } readOnly;
    if(!check(!readOnly.CreateImage(2,2,formatImageBGR,24,SGLImageObject::bufferOnMemory,2,100),
        "readback-only two-frame BGR24 source"))return false;
    readOnly.SelectFrame(0);std::unique_ptr<SGLImageObject> first(readOnly.NewReference(nullptr,0));
    readOnly.SelectFrame(1);std::unique_ptr<SGLImageObject> selected(readOnly.NewReference(nullptr,1));
    if(!check(first&&selected&&!first->FillImage(SGLPalette(0xffc03010))&&
        !selected->FillImage(SGLPalette(0xff302010)),"distinct real animation frames"))return false;
    auto* current=shade.Filter(&readOnly);SGLImageInfo info{};if(current)current->GetImageInfo(info);
    if(!check(pixel(current,0,0,value)&&(value&0xffffff)==0x102030&&!(info.format&formatImageFlagAlpha)&&
        readOnly.reads==1&&readOnly.lastFrame==1&&readOnly.GetSelectedFrame()==1,
        "only selected frame is read; RGB24 never acquires artificial alpha"))return false;

    // Test the compiled ARM64/NEON path against analytic impulse values.
    for(uint32_t width:{5u,6u}) {
        SGLImage impulse;
        if(impulse.CreateImage(width,3,formatImageARGB,32,SGLImageObject::bufferOnMemory))return false;
        impulse.FillImage(SGLPalette(0));impulse.SetPixelRGBA(2,1,SGLPalette(0xffffffff));
        SGLImageBuffer buffer;auto* data=impulse.LockBuffer(buffer,SGLImageObject::lockRead);
        if(!data)return false;
        std::vector<uint8_t> input(size_t(width)*3*4),out(input.size()),scratch;
        for(uint32_t y=0;y<3;++y)std::memcpy(input.data()+size_t(y)*width*4,data+ptrdiff_t(y)*buffer.pitchLine,width*4);
        if(impulse.UnlockBuffer(SGLImageObject::lockRead))return false;
        if(!LegacySuperShading::Loop421(out.data(),input.data(),width,3,scratch))return false;
        const uint8_t expected=width==5?63:64;
        if(!check(out[(width+2)*4]==expected&&out[(width+2)*4+3]==expected,
            width==5?"odd-width scalar rounding includes alpha":"even-width NEON matches SSE pavgb rounding"))return false;
    }
    return true;
}
