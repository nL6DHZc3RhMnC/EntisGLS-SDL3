// RasterScroll algorithm adapted from Entis GLS3 glssupsprite.cpp.
// Original: Copyright (c) 2004-2007 Leshade Entis, Entis-soft. All rights reserved.
#include "legacy_compat/gls.h"
#include "legacy_super_raster.h"
#include "platform/log.h"
#include <limits>
#include <memory>

LegacySuperRaster::LegacySuperRaster(SakuraGL::SGLSprite& owner,
    const std::array<uint32_t,33>& words,uint32_t degree):owner_(&owner),math_(words,degree) {}
ESLError LegacySuperRaster::Validate(const std::array<uint32_t,33>& words) {
    if(words[0]!=6 || !words[5] || int32_t(words[4])<0) return eslErrInvalidParam;
    return eslErrSuccess;
}
uint32_t LegacySuperRaster::GetDegree() const { return math_.Degree(); }
void LegacySuperRaster::SetDegree(uint32_t degree) {
    if(degree==GetDegree())return;
    math_.SetDegree(degree);
    if(auto* owner=owner_.GetReference())owner->NotifyUpdate();
}
void LegacySuperRaster::Advance(uint32_t milliseconds,bool enabled) {
    if(math_.Advance(milliseconds,enabled))
        if(auto* owner=owner_.GetReference())owner->NotifyUpdate();
}
bool LegacySuperRaster::GetRectangle(SakuraGL::SGLRect& rectangle,int32_t x,int32_t y,
                                    uint32_t width,uint32_t height) const {
    const int64_t left=int64_t(x)-math_.Width(), right=int64_t(x)+width-1+math_.Width();
    const int64_t bottom=int64_t(y)+height-1;
    if(!width||!height||left<INT32_MIN||right>INT32_MAX||bottom>INT32_MAX||right-left+1>INT32_MAX)return false;
    rectangle=SakuraGL::SGLRect(int32_t(left),y,int32_t(right),int32_t(bottom));return true;
}
SakuraGL::SGLError LegacySuperRaster::Draw(SakuraGL::SGLPaintContextInterface& render,
    SakuraGL::SGLImageObject* source,int32_t x,int32_t y) {
    using namespace SakuraGL;
    if(!source)return sglErrInvalidParam;
    SGLImageInfo info{};auto error=source->GetImageInfo(info);if(error)return error;
    const auto color=info.format&formatImageTypeMask;
    if(!info.width||!info.height||uint64_t(info.width)*info.height>0x4000000u||
       int64_t(y)+info.height>INT32_MAX)return sglErrInvalidParam;
    if((color!=formatImageRGB&&color!=formatImageBGR&&color!=formatImageGray)||
       (info.format&(formatImageFlagSideBySide|formatImageFlagPalette|formatImageFlagClipping|formatImageFlagS3TC))||
       !info.depth||info.depth>32||info.depth%8)return sglErrInvalidParam;
    SGLImageInfo cached{};frame_.GetImageInfo(cached);
    if(cached.width!=info.width||cached.height!=info.height||cached.format!=info.format||cached.depth!=info.depth)
        if((error=frame_.CreateImage(info.width,info.height,info.format,info.depth,SGLImageObject::bufferOnMemory)))return error;
    // ReadFrameBuffer preserves the selected frame and permits a GPU/readback-
    // only source. Keep its native channels and alpha mode for the SDK blender.
    SGLImageInfo target{};auto* pixels=frame_.LockBuffer(target,SGLImageObject::lockWrite);
    if(!pixels)return sglErrFailed;
    error=source->ReadFrameBuffer(target,pixels,source->GetSelectedFrame());
    const auto unlocked=frame_.UnlockBuffer(SGLImageObject::lockWrite);
    if(error)return error;if(unlocked)return unlocked;
    SGLPaintParam parameter;parameter.nTransparency=math_.Transparency();
    SGLImageRect clip{0,0,int32_t(info.width),1};
    for(uint32_t row=0;row<info.height;++row) {
        int32_t offset=0;
        if(!math_.Offset(row,offset)||int64_t(x)+offset<INT32_MIN||int64_t(x)+offset>INT32_MAX)
            return sglErrInvalidParam;
        parameter.ptPaint.x=x+offset;parameter.ptPaint.y=y+int32_t(row);clip.y=int32_t(row);
        // Src clip has its own local origin in GLS4. Including row in ptPaint
        // therefore matches GLS3's ptBasePos.y = destination.y + source row.
        if((error=render.DrawImage(parameter,&frame_,&clip)))return error;
    }
    return sglErrSuccess;
}

bool CheckLegacySuperRaster() {
    using namespace SakuraGL;
    struct Lock {Lock(){SSystem::Lock();}~Lock(){SSystem::Unlock();}} lock;
    auto check=[](bool result,const char* detail){if(!result)study::platform::LogPrint(study::platform::LogPriority::Error,
        "StudySteady","RasterScroll probe FAIL: %s",detail);return result;};
    std::array<uint32_t,33> words{};words[0]=6;words[2]=33;words[3]=7;words[4]=4;words[5]=2;
    SGLSprite owner;LegacySuperRaster effect(owner,words,128);
    if(!check(!LegacySuperRaster::Validate(words),"real RasterScroll parameter validation"))return false;
    effect.Advance(32,true);if(!check(effect.GetDegree()==128&&effect.IntervalCounter()==32,"hold before interval"))return false;
    effect.Advance(1,true);if(!check(effect.GetDegree()==135&&effect.IntervalCounter()==0,"exact interval tick"))return false;
    effect.SetDegree(766);effect.Advance(100,true);
    if(!check(effect.GetDegree()==531&&effect.IntervalCounter()==1,"multiple ticks and original 768 to 512 wrap"))return false;
    effect.Advance(31,false);if(!check(effect.GetDegree()==531,"disabled holds before tick"))return false;
    effect.Advance(1,false);if(!check(effect.GetDegree()==0,"disabled resets on next interval tick"))return false;
    effect.SetDegree(128);
    SGLRect bounds;if(!check(effect.GetRectangle(bounds,-1,2,4,4)&&bounds.left==-5&&bounds.top==2&&bounds.right==6&&bounds.bottom==5,
        "bounds include full configured amplitude independently of degree"))return false;
    class ReadbackImage final:public SGLImage {
    public:
        size_t reads=0,lastFrame=SIZE_MAX;
        uint8_t* LockBuffer(SGLImageInfo&,int,const SGLImageRect*) override {return nullptr;}
        SGLError ReadFrameBuffer(const SGLImageInfo& info,uint8_t* pixels,size_t frame,int side) override {
            ++reads;lastFrame=frame;return SGLImage::ReadFrameBuffer(info,pixels,frame,side);
        }
    } source;
    SGLImage target;constexpr uint32_t background=0xff030507;
    if(!check(!source.CreateImage(4,4,formatImageARGB|formatImageFlagNoProductOfAlpha,32,SGLImageObject::bufferOnMemory,2,100)&&
              !target.CreateImage(8,6,formatImageARGB|formatImageFlagNoProductOfAlpha,32),"real image buffers"))return false;
    source.SelectFrame(0);std::unique_ptr<SGLImageObject> first(source.NewReference(nullptr,0));
    source.SelectFrame(1);std::unique_ptr<SGLImageObject> selected(source.NewReference(nullptr,1));
    if(!check(first&&selected&&!first->FillImage(SGLPalette(0xffabcdef)),"distinct first frame"))return false;
    for(int row=0;row<4;++row)for(int column=0;column<4;++column)
        if(!check(!selected->SetPixelRGBA(column,row,SGLPalette(0xff102030u+uint32_t(row)*0x10101u+uint32_t(column)*0x080400u)),"selected frame fixture pixels"))return false;
    target.FillImage(SGLPalette(background));SGLPaintContext paint;
    if(!check(!paint.AttachTargetImage(&target,nullptr),"actual SDK software paint context"))return false;
    auto finish=[&](){paint.DetachTargetImage();};
    if(!check(!effect.Draw(paint,&source,-1,1),"clipped per-row draw")){finish();return false;}
    finish();
    // degree 128, width 4, mesh 2, frequency 0: offsets [0,+2,0,-2].
    const int starts[4]={-1,1,-1,-3};
    for(int row=0;row<6;++row)for(int column=0;column<8;++column){
        uint32_t expected=background;
        if(row>=1&&row<=4){const int sx=column-starts[row-1];if(sx>=0&&sx<4)
            expected=0xff102030u+uint32_t(row-1)*0x10101u+uint32_t(sx)*0x080400u;}
        SGLPalette actual{};if(!check(!target.GetPixelRGBA(actual,column,row)&&actual.ui32==expected,
            "golden raster rows, left clipping and untouched outside pixels"))return false;
    }
    if(!check(source.GetSelectedFrame()==1&&source.reads==1&&source.lastFrame==1,"readback-only source reads exactly the selected frame"))return false;
    const int shifts[4]={0,2,0,-2};
    for(const auto placement:{SGLPoint(6,-1),SGLPoint(1,4),SGLPoint(1,1)}){
        target.FillImage(SGLPalette(background));SGLImageRect viewport{2,2,3,2};
        const bool clipped=placement.y==1;
        if(!check(!paint.AttachTargetImage(&target,nullptr,clipped?&viewport:nullptr),"clipped target context"))return false;
        const auto drawError=effect.Draw(paint,&source,placement.x,placement.y);paint.DetachTargetImage();
        if(!check(!drawError,"edge and viewport clipped draw"))return false;
        for(int row=0;row<6;++row)for(int column=0;column<8;++column){
            uint32_t expectedPixel=background;const int sy=row-placement.y;
            if(sy>=0&&sy<4&&(!clipped||(column>=2&&column<5&&row>=2&&row<4))){
                const int sx=column-placement.x-shifts[sy];if(sx>=0&&sx<4)
                    expectedPixel=0xff102030u+uint32_t(sy)*0x10101u+uint32_t(sx)*0x080400u;
            }
            SGLPalette actual{};
            if(!check(!target.GetPixelRGBA(actual,column,row)&&actual.ui32==expectedPixel,"right/top/bottom edges and explicit viewport retain background"))return false;
        }
    }
    SGLPalette untouched{};selected->GetPixelRGBA(untouched,0,0);
    if(!check(untouched.ui32==0xff102030,"source pixels remain unchanged"))return false;
    // Reference is a real SDK alpha draw of one row, independently placed;
    // assert exact compositing against it instead of assuming ARGB arithmetic.
    words[1]=1;LegacySuperRaster transition(owner,words,128);
    SGLImage expected;
    if(!check(!expected.CreateImage(8,6,formatImageARGB|formatImageFlagNoProductOfAlpha,32),"transition reference buffer"))return false;
    selected->SetPixelRGBA(1,0,SGLPalette(0x70102030));target.FillImage(SGLPalette(background));expected.FillImage(SGLPalette(background));
    paint.AttachTargetImage(&target,nullptr);const auto actualError=transition.Draw(paint,&source,-1,1);paint.DetachTargetImage();
    SGLPaintContext reference;reference.AttachTargetImage(&expected,nullptr);SGLPaintParam pp;pp.nTransparency=64;
    for(int row=0;row<4;++row){pp.ptPaint.x=starts[row];pp.ptPaint.y=row+1;SGLImageRect clip{0,row,4,1};
        if(!check(!reference.DrawImage(pp,selected.get(),&clip),"reference alpha row")){reference.DetachTargetImage();return false;}}
    reference.DetachTargetImage();if(!check(!actualError,"transition alpha rendering"))return false;
    for(int row=0;row<6;++row)for(int column=0;column<8;++column){SGLPalette a{},b{};
        target.GetPixelRGBA(a,column,row);expected.GetPixelRGBA(b,column,row);
        if(!check(a.ui32==b.ui32,"quadratic transition transparency and partial source alpha"))return false;}
    transition.SetDegree(256);target.FillImage(SGLPalette(background));paint.AttachTargetImage(&target,nullptr);
    const auto endError=transition.Draw(paint,&source,1,1);paint.DetachTargetImage();
    if(!check(!endError,"fully transparent transition endpoint"))return false;
    for(int row=0;row<6;++row)for(int column=0;column<8;++column){SGLPalette a{};target.GetPixelRGBA(a,column,row);
        if(!check(a.ui32==background,"degree 256 does not overwrite destination"))return false;}
    words[5]=0;if(!check(LegacySuperRaster::Validate(words)!=eslErrSuccess,"zero wavelength rejects explicitly"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","RasterScroll probe PASS: original interval/wrap/flags, real selected-frame pixels, source preserved, expanded bounds, clipped sine rows, transition alpha and invalid parameters");
    return true;
}
