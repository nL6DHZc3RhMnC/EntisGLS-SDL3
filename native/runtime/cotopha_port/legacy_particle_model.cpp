#include "runtime/cotopha_port/legacy_particle_model.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <new>

#if defined(__clang__)
#pragma clang fp contract(off)
#endif

namespace LegacyParticle {
namespace {
uint32_t Random(uint32_t &seed,uint32_t limit) {
    seed=seed*5u+0x9a731651u;
    return limit?(seed>>8)%limit:0;
}
Vec2 Add(Vec2 a,Vec2 b){return {float(a.x+b.x),float(a.y+b.y)};}
// E3D_VECTOR_2D::operator* multiplies in double then rounds each component.
Vec2 Multiply(Vec2 value,double scale){return {float(value.x*scale),float(value.y*scale)};}
// In contrast its operator*= first rounds the multiplier to REAL32.
void MultiplyInPlace(Vec2 &value,double scale) {
    const float factor=float(scale);value.x=float(value.x*factor);value.y=float(value.y*factor);
}
bool Finite(Vec2 value){return std::isfinite(value.x)&&std::isfinite(value.y);}
bool Finite(const Flick &value) {
    return std::isfinite(value.rAmplitude)&&std::isfinite(value.rAmplitudeRange)&&
        std::isfinite(value.rFrequency)&&std::isfinite(value.rFrequencyRange);
}
bool Valid(const Param &p) {
    for(double n:{p.rFadeZoom,p.rGenWidth,p.rGenHeight,p.rGenAngle,p.rGenAngleRange,
        p.rGenVelocity,p.rGenVelocityRange,p.rShrink,p.rRevSpeed,p.rRevSpeedRange,
        p.rZoom,p.rZoomRange,p.rGenSpeedRange})if(!std::isfinite(n))return false;
    // A negative pow() base has no real result for fractional frame times.
    return p.rShrink<=1&&Finite(p.pfFlickness[0])&&Finite(p.pfFlickness[1])&&
        Finite(p.vGenSpeed)&&Finite(p.vStream)&&Finite(p.vGravity);
}
bool Finite(const Particle &p) {
    return Finite(p.vShow)&&Finite(p.vPos)&&Finite(p.vVelocity)&&Finite(p.vAcceleration)&&
        Finite(p.vFlickUnit)&&Finite(p.pfFlickness[0])&&Finite(p.pfFlickness[1])&&
        std::isfinite(p.rRevAngle)&&std::isfinite(p.rRevSpeed)&&std::isfinite(p.rZoom)&&
        std::isfinite(p.rFlicknessPhase);
}
Particle NewParticle(const Param &p,uint32_t &seed,float emitterX,float emitterY,uint32_t imageCount) {
    Particle out;
    out.iParticleImage=Random(seed,imageCount);
    const Vec2 local{
        float(p.rGenWidth*(int32_t(Random(seed,0x8000))-0x4000)/0x4000),
        float(p.rGenHeight*(int32_t(Random(seed,0x8000))-0x4000)/0x4000)};
    out.vPos=Add({emitterX,emitterY},local);out.vShow=out.vPos;
    const double velocity=p.rGenVelocity+p.rGenVelocityRange*Random(seed,0x1000)/0x1000;
    double angle=p.rGenAngle+p.rGenAngleRange*Random(seed,0x1000)/0x1000;
    angle*=3.1415926535897932384626433832795/180;
    out.vVelocity={float(velocity*std::cos(angle)),float(velocity*std::sin(angle))};
    out.vVelocity=Add(out.vVelocity,Multiply(p.vGenSpeed,1+p.rGenSpeedRange*Random(seed,0x1000)/0x1000));
    out.rRevSpeed=p.rRevSpeed+p.rRevSpeedRange*Random(seed,0x1000)/0x1000;
    out.rZoom=p.rZoom+p.rZoomRange*Random(seed,0x1000)/0x1000;
    for(unsigned i=0;i<2;++i) {
        out.pfFlickness[i].rAmplitude=p.pfFlickness[i].rAmplitude+
            p.pfFlickness[i].rAmplitudeRange*Random(seed,0x1000)/0x1000;
        out.pfFlickness[i].rFrequency=p.pfFlickness[i].rFrequency+
            p.pfFlickness[i].rFrequencyRange*Random(seed,0x1000)/0x1000;
    }
    out.vFlickUnit={out.vVelocity.y,-out.vVelocity.x};
    const float squareX=out.vFlickUnit.x*out.vFlickUnit.x;
    const float squareY=out.vFlickUnit.y*out.vFlickUnit.y;
    const double square=float(squareX+squareY);
    if(square<1.0e-5) {
        // Preserve the lower-precision pi literal in the original fallback.
        const double angle=3.1415926*2.0*double(Random(seed,0x1000))/0x1000;
        out.vFlickUnit={float(std::cos(angle)),float(std::sin(angle))};
    } else MultiplyInPlace(out.vFlickUnit,1.0/std::sqrt(square));
    out.rFlicknessPhase=6.283185307179586476925286766559*(double(Random(seed,0x1000))/0x1000);
    return out;
}
void AdvanceOne(Particle &p,const Param &param,uint32_t ms) {
    const double seconds=ms*0.001;
    p.nPastTime+=ms;
    p.nAnimeTime+=uint32_t(uint64_t(ms)*param.nAnimationSpeed/0x100);
    if(param.rShrink!=0)MultiplyInPlace(p.vVelocity,std::pow(1.0-param.rShrink,seconds));
    p.vAcceleration=Add(p.vAcceleration,Multiply(param.vGravity,seconds));
    p.vPos=Add(p.vPos,Multiply(p.vVelocity,seconds));
    // This is intentionally the original formula: accumulated gravity is
    // added directly, without another seconds factor.
    p.vPos=Add(p.vPos,p.vAcceleration);
    p.vPos=Add(p.vPos,Multiply(param.vStream,seconds));
    p.rRevAngle+=p.rRevSpeed*seconds;
    constexpr double phasePerMs=3.1415926535897932384626433832795*2/1000.0;
    double amplitude=0;
    for(const Flick &f:p.pfFlickness)if(f.rAmplitude!=0&&f.rFrequency!=0)
        amplitude+=f.rAmplitude*std::sin(p.nPastTime*phasePerMs/f.rFrequency+p.rFlicknessPhase);
    p.vShow=Add(p.vPos,Multiply(p.vFlickUnit,amplitude));
}
}

bool Model::Generate(uint32_t count,float emitterX,float emitterY,size_t imageCount) {
    if(count==0)return true;
    if(particles.size()>maxParticles||count>maxParticles-particles.size()||
        imageCount>uint32_t(std::numeric_limits<int32_t>::max())||!Valid(param)||
        !std::isfinite(emitterX)||!std::isfinite(emitterY))return false;
    const auto oldSize=particles.size();const auto oldSeed=seed;
    try {
        particles.reserve(oldSize+count);
        for(uint32_t i=0;i<count;++i) {
            auto particle=NewParticle(param,seed,emitterX,emitterY,uint32_t(imageCount));
            if(!Finite(particle)){particles.resize(oldSize);seed=oldSeed;return false;}
            particles.push_back(particle);
        }
    } catch(const std::bad_alloc &) {particles.resize(oldSize);seed=oldSeed;return false;}
    return true;
}

bool Model::Advance(uint32_t ms,float emitterX,float emitterY,const std::vector<ImageInfo>& images) {
    if(particles.size()>maxParticles||!Valid(param)||!std::isfinite(emitterX)||!std::isfinite(emitterY)||
        images.size()>uint32_t(std::numeric_limits<int32_t>::max()))return false;
    const uint32_t elapsed=std::min(ms,1000u);
    const uint64_t units=uint64_t(generationCount)*elapsed;
    uint32_t nextSeed=seed;
    uint32_t count=uint32_t(units/100000);
    if(units%100000>Random(nextSeed,100000))++count;
    if(count>maxParticles-particles.size())return false;
    try {
        // Generate first, but only advance particles which existed at entry.
        // Staging also makes invalid/overflow/capacity failures reversible.
        Model born;born.param=param;born.seed=nextSeed;
        if(!born.Generate(count,emitterX,emitterY,images.size()))return false;
        std::vector<Particle> result;result.reserve(particles.size()+count);
        for(auto particle:particles) {
            if(!Finite(particle))return false;
            AdvanceOne(particle,param,elapsed);
            if(!Finite(particle))return false;
            if(param.nDuration>0&&particle.nPastTime>=param.nDuration)continue;
            const uint32_t length=particle.iParticleImage<images.size()?images[particle.iParticleImage].totalTime:0;
            if(length) {
                if(param.nFlags&pfAnimationLoop)particle.nAnimeTime%=length;
                else if(int32_t(particle.nAnimeTime)>=int32_t(length))continue;
            }
            result.push_back(particle);
        }
        result.insert(result.end(),born.particles.begin(),born.particles.end());
        particles.swap(result);seed=born.seed;
    } catch(const std::bad_alloc &) {return false;}
    return true;
}
}
