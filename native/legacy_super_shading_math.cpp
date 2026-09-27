#include "legacy_super_shading_math.h"
#include <algorithm>
#include <cstring>
#include <limits>
#if defined(__aarch64__)
#include <arm_neon.h>
#endif

namespace LegacySuperShading {
namespace {
bool Dimensions(uint32_t w,uint32_t h) {
    return w && h && uint64_t(w)*h<=33554432 && uint64_t(w)*h<=SIZE_MAX/4;
}
void Average(uint8_t* out,const uint8_t* left,const uint8_t* middle,const uint8_t* right,
             size_t bytes,bool rounded) {
    size_t i=0;
#if defined(__aarch64__)
    // Same two separate rounding operations as pavgb, not (L+2*C+R)/4.
    for(;i+16<=bytes;i+=16) {
        const auto a=vld1q_u8(left+i),b=vld1q_u8(middle+i),c=vld1q_u8(right+i);
        vst1q_u8(out+i,rounded?vrhaddq_u8(b,vrhaddq_u8(a,c)):vhaddq_u8(b,vhaddq_u8(a,c)));
    }
#endif
    const unsigned r=rounded?1:0;
    for(;i<bytes;++i)out[i]=uint8_t((middle[i]+((unsigned(left[i])+right[i]+r)>>1)+r)>>1);
}
}
bool Loop421(uint8_t* destination,const uint8_t* source,uint32_t width,uint32_t height,
             std::vector<uint8_t>& scratch) {
    if(!source||!destination||!Dimensions(width,height))return false;
    const size_t stride=size_t(width)*4,bytes=stride*height;
    if(width<2||height<2) {if(destination!=source)std::memcpy(destination,source,bytes);return true;}
    scratch.resize(bytes);
    const bool rounded=(width&1)==0;
    for(uint32_t y=0;y<height;++y) {
        const auto* row=source+size_t(y)*stride;auto* to=scratch.data()+size_t(y)*stride;
        std::memcpy(to,row,4);std::memcpy(to+stride-4,row+stride-4,4);
        Average(to+4,row,row+4,row+8,stride-8,rounded);
    }
    std::memcpy(destination,scratch.data(),stride);
    for(uint32_t y=1;y+1<height;++y) {
        const auto* row=scratch.data()+size_t(y)*stride;
        Average(destination+size_t(y)*stride,row-stride,row,row+stride,stride,rounded);
    }
    std::memcpy(destination+bytes-stride,scratch.data()+bytes-stride,stride);
    return true;
}
uint32_t SquareDegree(uint32_t degree) {
    // The public setter is unsigned, but GLS3 stores m_nBlendDegree as SDWORD.
    const int64_t signedDegree=int32_t(degree);
    return uint32_t(std::min(signedDegree*signedDegree/256,int64_t(256)));
}
uint32_t DrawTransparency(uint32_t kind,uint32_t flags,uint32_t degree,uint32_t normal) {
    return kind==10&&(flags&1)?SquareDegree(degree):normal;
}
bool Raster::SetSource(const uint8_t* pixels,uint32_t width,uint32_t height,ptrdiff_t stride) {
    if(!pixels||!Dimensions(width,height)||stride==PTRDIFF_MIN||
       (stride<0?-stride:stride)<ptrdiff_t(width)*4)return false;
    const size_t row=size_t(width)*4,bytes=row*height;
    bool changed=width!=width_||height!=height_||levels_[0].size()!=bytes;
    if(!changed)for(uint32_t y=0;y<height;++y)
        if(std::memcmp(levels_[0].data()+size_t(y)*row,pixels+ptrdiff_t(y)*stride,row)){changed=true;break;}
    if(!changed)return true;
    levels_[0].resize(bytes);
    for(uint32_t y=0;y<height;++y)std::memcpy(levels_[0].data()+size_t(y)*row,pixels+ptrdiff_t(y)*stride,row);
    width_=width;height_=height;levelsReady_=1;++revision_;return true;
}
bool Raster::Render(uint32_t kind,uint32_t degree,std::vector<uint8_t>& output) {
    if(!levelsReady_||(kind!=10&&kind!=11))return false;
    const uint32_t d=uint32_t(std::clamp(int32_t(degree),0,255)),index=d>>6,offset=(d&63)*256/63;
    // GLS3 prepares the following level even at offset zero. Keep its exact
    // 16-pass boundaries; source changes reset all levels, degree changes do not.
    while(levelsReady_<=index+1) {
        auto& next=levels_[levelsReady_];next=levels_[levelsReady_-1];
        for(unsigned i=0;i<16;++i) {
            if(!Loop421(next.data(),next.data(),width_,height_,scratch_))return false;
            ++passes_;
        }
        ++levelsReady_;
    }
    const auto& a=levels_[index];const auto& b=levels_[index+1];output.resize(a.size());
    const unsigned light=kind==11?(255*SquareDegree(degree))>>8:0;
    for(size_t i=0;i<a.size();++i) {
        // Apply_Transparency then Render_MoveColor truncate each product
        // separately. Even a constant fractional mix can lose one byte value.
        unsigned value=((unsigned(a[i])*(256-offset))>>8)+((unsigned(b[i])*offset)>>8);
        if((i&3)!=3)value=std::min(value+light,255u);
        output[i]=uint8_t(value);
    }
    return true;
}
}
