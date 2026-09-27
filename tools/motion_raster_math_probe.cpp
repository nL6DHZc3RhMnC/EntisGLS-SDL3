#include "../native/legacy_super_raster_math.h"
#include <cstdio>
#include <cstdlib>
#include <utility>

static void require(bool value,const char* detail) {
    if(!value){std::fprintf(stderr,"Raster math FAIL: %s\n",detail);std::exit(1);}
}
int main() {
    std::array<uint32_t,33> words{};
    words[0]=6;words[2]=33;words[3]=7;words[4]=48;words[5]=128;words[7]=6;
    LegacySuperRasterMath scene(words,0);int32_t offset=0;
    for(auto pair:{std::pair<double,int>{.5,0},{1.5,2},{2.5,2},{-.5,0},{-1.5,-2},{-2.5,-2}})
        require(LegacySuperRasterMath::RoundNearestEven(pair.first,offset)&&offset==pair.second,"Windows x87 tie rounding");
    require(!LegacySuperRasterMath::RoundNearestEven(INFINITY,offset)&&
            !LegacySuperRasterMath::RoundNearestEven(2147483648.,offset),"invalid displacements fail");
    struct Golden {uint32_t degree,row;int32_t displacement;};
    // Exact quarter/half-wave sample positions for the actual game recipe.
    const Golden samples[]={
        {0,64,0},{64,0,-12},{64,64,0},{64,128,12},{64,192,0},
        {128,0,0},{128,64,-24},{128,128,0},{128,192,24},
        {256,0,0},{256,64,48},{256,128,0},{256,192,-48},
        {512,0,0},{512,64,48},{512,128,0},{512,192,-48}};
    for(const auto& sample:samples){scene.SetDegree(sample.degree);
        require(scene.Offset(sample.row,offset)&&offset==sample.displacement,"actual amplitude/phase samples");}
    scene.SetDegree(766);require(!scene.Advance(32,true)&&scene.Degree()==766,"pre-interval hold");
    require(scene.Advance(1,true)&&scene.Degree()==517&&scene.IntervalCounter()==0,"one interval wraps at 768");
    require(scene.Advance(100,true)&&scene.Degree()==538&&scene.IntervalCounter()==1,"multiple intervals retain remainder");
    require(!scene.Advance(31,false)&&scene.Degree()==538,"disable does not reset before boundary");
    require(scene.Advance(1,false)&&scene.Degree()==0,"disabled reset at boundary");
    // The original has signed fields and Win32 integer wrap. A negative step
    // is not silently clamped to zero; a nonpositive interval does not tick.
    words[3]=uint32_t(-7);LegacySuperRasterMath reverse(words,3);
    require(reverse.Advance(33,true)&&reverse.Degree()==uint32_t(-4),"signed step wrap is preserved");
    words[2]=0;words[1]=1;LegacySuperRasterMath transition(words,128);
    require(!transition.Advance(1000,false)&&transition.Degree()==128&&transition.Transparency()==64&&
        transition.Flags()==5,"zero interval, runtime transformation flag, quadratic transparency");
    transition.SetDegree(256);require(transition.Transparency()==256,"transparent endpoint");
    words[2]=33;words[1]=2;words[3]=7;LegacySuperRasterMath loop(words,766);
    require(loop.Advance(33,true)&&loop.Degree()==517&&loop.Transparency()==0&&loop.Flags()==6,
        "loop flag does not become transition alpha");
    std::puts("Raster math PASS: 17 actual-recipe samples, x87 ties, signed degree, interval/remainder, flags and transition endpoints");
}
