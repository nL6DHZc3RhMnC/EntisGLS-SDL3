#include "../native/legacy_super_shading_math.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <vector>

int main() {
    using namespace LegacySuperShading;
    std::vector<uint8_t> scratch,dst;
    // Analytic central impulse: the scalar path loses an odd half; SSE pavgb
    // retains it. All four channels use the same kernel, including alpha.
    for(unsigned width:{5u,6u}) {
        std::vector<uint8_t> input(width*3*4);dst.resize(input.size());
        for(unsigned c=0;c<4;++c)input[(width+2)*4+c]=255;
        assert(Loop421(dst.data(),input.data(),width,3,scratch));
        for(unsigned c=0;c<4;++c) {
            assert(dst[(width+2)*4+c]==(width==5?63:64));
            assert(dst[(width+1)*4+c]==(width==5?31:32));
            assert(dst[(width+3)*4+c]==(width==5?31:32));
            assert(dst[2*4+c]==0&&dst[(2*width+2)*4+c]==0);
        }
        auto inplace=input;assert(Loop421(inplace.data(),inplace.data(),width,3,scratch));assert(inplace==dst);
        std::fill(input.begin(),input.end(),0);input[(width+2)*4]=1;
        assert(Loop421(dst.data(),input.data(),width,3,scratch));assert(dst[(width+2)*4]==(width==5?0:1));
    }
    for(auto size: {std::pair<unsigned,unsigned>{1,4},{4,1},{2,2}}) {
        std::vector<uint8_t> input(size.first*size.second*4);
        for(size_t i=0;i<input.size();++i)input[i]=uint8_t(i*17);
        dst.resize(input.size());assert(Loop421(dst.data(),input.data(),size.first,size.second,scratch));assert(input==dst);
    }
    // Boundary rows still receive the horizontal pass, left/right pixels
    // still receive the vertical pass; this is not edge-clamped convolution.
    std::vector<uint8_t> edge(5*3*4);edge[4]=255;dst.resize(edge.size());
    assert(Loop421(dst.data(),edge.data(),5,3,scratch));assert(dst[4]==127&&dst[8]==63&&dst[0]==0);
    edge.assign(edge.size(),0);edge[5*4]=255;
    assert(Loop421(dst.data(),edge.data(),5,3,scratch));assert(dst[5*4]==127&&dst[0]==0);

    Raster raster;std::vector<uint8_t> constant(2*2*4,77),out;
    assert(raster.SetSource(constant.data(),2,2,8));assert(raster.Render(10,0,out)&&out==constant);
    assert(raster.Passes()==16);const auto revision=raster.Revision();
    assert(raster.SetSource(constant.data(),2,2,8)&&raster.Revision()==revision);
    assert(raster.Render(10,1,out)&&out[0]==76&&out[3]==76); // separate product truncations
    assert(raster.Render(10,63,out)&&out==constant&&raster.Passes()==16);
    assert(raster.Render(10,64,out)&&out==constant&&raster.Passes()==32);
    assert(raster.Render(10,255,out)&&out==constant&&raster.Passes()==64);
    assert(raster.Render(10,256,out)&&out==constant&&raster.Passes()==64);
    assert(raster.Render(11,128,out)&&out[0]==140&&out[3]==77); // saturating addition, not lerp to white
    assert(raster.Render(11,256,out)&&out[0]==255&&out[3]==77);
    assert(DrawTransparency(10,1,128,19)==64&&DrawTransparency(10,0,128,19)==19);
    assert(DrawTransparency(11,1,128,19)==19&&SquareDegree(UINT32_MAX)==0);
    assert(raster.Render(11,UINT32_MAX,out)&&out==constant);
    assert(SquareDegree(uint32_t(-256))==256&&SquareDegree(0x80000000u)==256);
    constant[0]=91;assert(raster.SetSource(constant.data(),2,2,8)&&raster.Revision()==revision+1);
    assert(raster.Render(10,0,out)&&out==constant&&raster.Passes()==80);
    std::vector<uint8_t> inverted={0,1,2,3,4,5,6,7};
    assert(raster.SetSource(inverted.data()+4,1,2,-4)&&raster.Render(10,0,out));
    assert(out[0]==4&&out[4]==0);
    assert(!raster.SetSource(nullptr,2,2,8)&&!raster.SetSource(inverted.data(),0,2,4));
    assert(!raster.SetSource(inverted.data(),UINT32_MAX,UINT32_MAX,4)&&!raster.Render(9,0,out));
    const auto begin=std::chrono::steady_clock::now();
    std::vector<uint8_t> hd(1920*1080*4);
    for(size_t i=0;i<hd.size();++i)hd[i]=uint8_t(i*13+(i/1920)%255);
    assert(raster.SetSource(hd.data(),1920,1080,1920*4)&&raster.Render(10,255,out));
    const auto milliseconds=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-begin).count();
    std::printf("Shading math PASS: odd/even kernels, borders, alpha, stage rounding, cache, signed stride, 1080p64passes=%lldms\n",static_cast<long long>(milliseconds));
}
