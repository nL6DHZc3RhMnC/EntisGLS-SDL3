#include "../native/legacy_particle_model.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using namespace LegacyParticle;
static bool Near(float a,float b){return std::abs(double(a)-b)<0.00001;}
static Particle Still(){Particle p;p.rZoom=1;return p;}

int main() {
    const std::vector<ImageInfo> stillImage={{16,16,0}};
    {
        Model m;m.seed=0x12345678;m.param.rGenWidth=16;m.param.rGenHeight=8;
        assert(m.Generate(1,10,-20,3));
        const auto &p=m.particles.at(0);
        // LCG golden values are independent of the implementation's helpers.
        assert(p.iParticleImage==2);
        assert(p.vPos.x==10+float(16*(int((0x65cef79eu>>8)%0x8000)-0x4000)/16384.0));
        assert(p.vPos.y==-20+float(8*(int((0x977dec67u>>8)%0x8000)-0x4000)/16384.0));
        assert(m.seed==0x6057ddd2u);
        assert(p.nPastTime==0&&p.nAnimeTime==0&&p.rZoom==1);
        assert(m.Generate(0,0,0,0)&&m.seed==0x6057ddd2u);
    }
    {
        Model m;m.param.vGravity={0,10};m.param.vStream={2,3};
        auto p=Still();p.vPos={5,7};p.vShow=p.vPos;p.vVelocity={10,20};m.particles.push_back(p);
        assert(m.Advance(100,0,0,stillImage));
        assert(Near(m.particles[0].vPos.x,6.2f)&&Near(m.particles[0].vPos.y,10.3f));
        assert(m.particles[0].vAcceleration.y==1);
        assert(m.Advance(100,0,0,stillImage));
        assert(Near(m.particles[0].vPos.x,7.4f)&&Near(m.particles[0].vPos.y,14.6f));
        assert(m.particles[0].vAcceleration.y==2); // No extra dt multiplication.
    }
    {
        Model a;a.seed=333;a.param.vGravity={3,7};a.param.rShrink=.25;
        auto p=Still();p.vVelocity={10,20};a.particles.push_back(p);Model b=a;
        assert(a.Advance(60000,4,5,stillImage)&&b.Advance(1000,4,5,stillImage));
        assert(a.seed==b.seed&&a.particles[0].nPastTime==1000);
        assert(a.particles[0].vPos.x==b.particles[0].vPos.x&&a.particles[0].vPos.y==b.particles[0].vPos.y);
    }
    {
        Model m;m.generationCount=100;m.param.nDuration=500;m.param.rGenVelocity=10;
        assert(m.Generate(1,0,0,1));
        assert(m.Advance(1000,12,34,stillImage));
        assert(m.particles.size()==1&&m.particles[0].nPastTime==0&&m.particles[0].nAnimeTime==0);
        assert(m.particles[0].vPos.x==12&&m.particles[0].vPos.y==34); // Born after old-count snapshot.
    }
    {
        Model m;m.seed=0;m.generationCount=50;
        const uint32_t roundedRandom=(0x9a731651u>>8)%100000;
        assert(m.Advance(1000,0,0,stillImage));
        assert(m.particles.size()==(50000>roundedRandom?1u:0u));
        Model zero;zero.seed=0;assert(zero.Advance(0,0,0,{}));assert(zero.seed==0x9a731651u);
    }
    {
        Model loop;loop.param.nFlags=Model::pfAnimationLoop;loop.particles.push_back(Still());
        const std::vector<ImageInfo> animated={{8,8,550}};
        assert(loop.Advance(1000,0,0,animated));assert(loop.particles[0].nAnimeTime==450);
        Model once;once.particles.push_back(Still());assert(once.Advance(1000,0,0,animated));assert(once.particles.empty());
        Model fast;fast.param.nAnimationSpeed=512;fast.particles.push_back(Still());
        assert(fast.Advance(250,0,0,{{8,8,500}})&&fast.particles.empty());
        Model paused;paused.param.nAnimationSpeed=0;paused.particles.push_back(Still());
        assert(paused.Advance(1000,0,0,animated)&&paused.particles.size()==1&&paused.particles[0].nAnimeTime==0);
        Model infinite;infinite.particles.push_back(Still());
        assert(infinite.Advance(1000,0,0,{})&&infinite.particles.size()==1);
    }
    {
        Model m;auto p=Still();p.pfFlickness[0].rAmplitude=2;p.pfFlickness[0].rFrequency=1;
        p.vFlickUnit={1,0};p.rRevSpeed=180;m.particles.push_back(p);
        assert(m.Advance(250,0,0,stillImage));
        assert(Near(m.particles[0].vShow.x,2)&&m.particles[0].vShow.y==0&&m.particles[0].rRevAngle==45);
    }
    {
        Model m;m.param.rShrink=.37;auto p=Still();p.vVelocity={123.456f,-543.21f};m.particles.push_back(p);
        const float factor=float(std::pow(1.0-.37,.007));
        const float expectedX=float(p.vVelocity.x*factor),expectedY=float(p.vVelocity.y*factor);
        assert(m.Advance(7,0,0,stillImage));
        assert(m.particles[0].vVelocity.x==expectedX&&m.particles[0].vVelocity.y==expectedY);
    }
    {
        Model m;m.seed=77;assert(m.Generate(1,0,0,1));const auto seed=m.seed;
        assert(!m.Generate(65536,0,0,1)&&m.particles.size()==1&&m.seed==seed);
        m.param.rGenVelocity=std::numeric_limits<double>::max();
        assert(!m.Generate(1,0,0,1)&&m.particles.size()==1&&m.seed==seed);
        m.param.rGenVelocity=0;m.param.rShrink=1.1;
        assert(!m.Advance(16,0,0,stillImage)&&m.seed==seed&&m.particles[0].nPastTime==0);
        m.param.rShrink=0;m.param.vGravity.x=std::numeric_limits<float>::quiet_NaN();
        assert(!m.Advance(16,0,0,stillImage)&&m.seed==seed&&m.particles[0].nPastTime==0);
        Model full;full.seed=123;full.generationCount=100;full.particles.resize(Model::maxParticles);
        assert(!full.Advance(1000,0,0,stillImage)&&full.particles.size()==Model::maxParticles&&full.seed==123);
    }
    std::cout<<"PASS: GLS3 LCG, spawn order/rate, dt clamp, float truncation, gravity, lifetime/loop/flicker and transactional bounds\n";
}
