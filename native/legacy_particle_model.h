#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

// Legacy GLS3 two-dimensional particle mathematics. No engine/JNI types.
namespace LegacyParticle {
struct Vec2 {float x=0,y=0;};
struct Flick {
    double rAmplitude=0,rAmplitudeRange=0,rFrequency=0,rFrequencyRange=0;
};
struct Param {
    uint32_t nFlags=0,nDuration=0,nAnimationSpeed=0x100,nFadein=0,nFadeout=0,nFadeTransparency=0;
    double rFadeZoom=1,rGenWidth=0,rGenHeight=0,rGenAngle=0,rGenAngleRange=0;
    double rGenVelocity=0,rGenVelocityRange=0,rShrink=0,rRevSpeed=0,rRevSpeedRange=0,rZoom=1,rZoomRange=0;
    Flick pfFlickness[2];
    Vec2 vGenSpeed;
    double rGenSpeedRange=0;
    Vec2 vStream,vGravity;
};
struct Particle {
    uint32_t iParticleImage=0,nPastTime=0,nAnimeTime=0;
    Vec2 vShow,vPos,vVelocity,vAcceleration;
    double rRevAngle=0,rRevSpeed=0,rZoom=1;
    Flick pfFlickness[2];
    Vec2 vFlickUnit;
    double rFlicknessPhase=0;
};
struct ImageInfo {uint32_t width=0,height=0,totalTime=0;};

class Model {
public:
    static constexpr size_t maxParticles=65536;
    static constexpr uint32_t pfAnimationLoop=1;
    Param param;
    std::vector<Particle> particles;
    uint32_t seed=0,generationCount=0; // generationCount is particles / 100 sec.
    // False leaves particles and seed unchanged (invalid/non-finite input,
    // overflow, allocation failure or capacity exceeded). A missing image is
    // allowed, as in GLS3: its particles still age but cannot be rendered.
    bool Generate(uint32_t count,float emitterX,float emitterY,size_t imageCount);
    bool Advance(uint32_t ms,float emitterX,float emitterY,const std::vector<ImageInfo>& images);
};
}
