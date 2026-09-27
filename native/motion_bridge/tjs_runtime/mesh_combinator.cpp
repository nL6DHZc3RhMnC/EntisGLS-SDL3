#include "mesh_combinator.h"
#include "MotionDispatch.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace studysteady::motion {
namespace {
constexpr float epsilon = 0x1p-23f;
tTJSVariant required(const tTJSVariant &v, const tjs_char *name) {
    if(v.Type()!=tvtObject || !v.AsObjectNoAddRef())
        throw std::runtime_error("meshCombinator: expected object");
    tTJSVariant out;
    auto *o=v.AsObjectNoAddRef();
    if(TJS_FAILED(o->PropGet(0,name,nullptr,&out,o)) || out.Type()==tvtVoid)
        throw std::runtime_error("meshCombinator: missing " + ttstr(name).AsStdString());
    return out;
}
float position(const MeshCombinator::Axis &axis, const MeshVariables &vars) {
    const auto it=vars.find(axis.key);
    const float raw=it==vars.end()?0.0f:it->second;
    if(!std::isfinite(raw)) throw std::runtime_error("meshCombinator: nonfinite variable " + axis.key);
    const float ratio=(raw-axis.begin)/(axis.end-axis.begin);
    const float clamped=ratio>=1.0f?1.0f:(ratio>0.0f?ratio:0.0f);
    return clamped*static_cast<float>(axis.frames.size()-1);
}
void sample(MeshCombinator::Axis &axis, float value) {
    axis.position=value;
    const size_t index=static_cast<size_t>(value);
    const float ratio=value-static_cast<float>(index);
    // At the upper endpoint the DLL passes an unused one-past pointer. Its
    // epsilon branch copies only the first patch; do not form a C++ reference
    // to that nonexistent next element.
    const auto &next=axis.frames[std::min(index+1,axis.frames.size()-1)];
    meshInterpolate(axis.current,axis.frames.at(index),next,ratio);
    axis.neutral=axis.neutralIndex==static_cast<int>(index) && std::fabs(ratio)<epsilon;
}
}
void meshInterpolate(MeshPatch &out,const MeshPatch &a,const MeshPatch &b,float ratio) {
    // DLL 0x10059270. Separate operations, deliberately no std::lerp/FMA.
    if(std::fabs(ratio)<epsilon) { out=a;return; }
    const float inverse=1.0f-ratio;
    for(size_t i=0;i<out.size();++i) out[i]=a[i]*inverse+b[i]*ratio;
}
void meshAdd(MeshPatch &out,const MeshPatch &a,const MeshPatch &b) {
    // DLL 0x10059480, including in-place output.
    for(size_t i=0;i<out.size();++i) out[i]=a[i]+b[i];
}
void meshAddSubtract(MeshPatch &out,const MeshPatch &a,const MeshPatch &add,const MeshPatch &subtract) {
    // DLL 0x100595C0: (oldSum + newAxis) - oldAxis, not oldSum+(new-old).
    for(size_t i=0;i<out.size();++i) out[i]=(a[i]+add[i])-subtract[i];
}
MeshCombinator::MeshCombinator(const tTJSVariant &value) {
    const auto list=required(value,TJS_W("combinatorList"));
    const auto count=::motion::detail::motionPropGetCount(list);
    if(count<1 || count>1024) throw std::runtime_error("meshCombinator: invalid combinator count");
    axes_.reserve(static_cast<size_t>(count));
    for(int i=0;i<count;++i) {
        const auto entry=::motion::detail::motionPropGetByNum(list,i);
        if(required(entry,TJS_W("meshType")).AsInteger()!=1)
            throw std::runtime_error("meshCombinator: only DLL's 16-point meshType 1 is supported");
        Axis a;
        a.neutralIndex=::motion::detail::motionPropGetInt(entry,TJS_W("neutralIndex"),-1);
        const auto variable=required(entry,TJS_W("variable"));
        a.key=ttstr(required(variable,TJS_W("key"))).AsStdString();
        a.begin=static_cast<float>(required(variable,TJS_W("rangeBegin")).AsReal());
        a.end=static_cast<float>(required(variable,TJS_W("rangeEnd")).AsReal());
        const auto n=required(variable,TJS_W("meshCount")).AsInteger();
        if(n<1 || n>65536 || !std::isfinite(a.begin) || !std::isfinite(a.end) || a.begin>=a.end)
            throw std::runtime_error("meshCombinator: invalid variable range/count");
        if(a.neutralIndex < -1 || a.neutralIndex>=n)
            throw std::runtime_error("meshCombinator: neutral index out of range");
        const auto raw=required(entry,TJS_W("rawMeshList"));
        if(raw.Type()!=tvtOctet || raw.AsOctetNoAddRef()->GetLength()!=static_cast<size_t>(n)*sizeof(MeshPatch))
            throw std::runtime_error("meshCombinator: rawMeshList size mismatch");
        a.frames.resize(static_cast<size_t>(n));
        // All supported hosts and Android ABIs are little-endian IEEE binary32.
        static_assert(sizeof(float)==4 && std::numeric_limits<float>::is_iec559);
        const uint32_t endian=1;
        if(*reinterpret_cast<const uint8_t *>(&endian)!=1) throw std::runtime_error("meshCombinator: unsupported byte order");
        std::memcpy(a.frames.data(),raw.AsOctetNoAddRef()->GetData(),static_cast<size_t>(n)*sizeof(MeshPatch));
        for(const auto &patch:a.frames) for(float f:patch)
            if(!std::isfinite(f)) throw std::runtime_error("meshCombinator: nonfinite mesh point");
        axes_.push_back(std::move(a));
    }
}
bool MeshCombinator::update(const MeshVariables &variables) {
    std::vector<size_t> dirty;
    for(size_t i=0;i<axes_.size();++i) {
        auto &a=axes_[i];const float next=position(a,variables);
        if(!initialized_ || next!=a.position) {
            a.previous=a.current;sample(a,next);dirty.push_back(i);
        }
    }
    if(dirty.empty()) return false;
    // Exact branch from 0x1005BD0B: count >= (total >> 1), not total/2.0.
    if(!initialized_ || dirty.size()>=(axes_.size()>>1)) {
        combined_=axes_.front().current;
        for(size_t i=1;i<axes_.size();++i) meshAdd(combined_,combined_,axes_[i].current);
    } else {
        for(size_t i:dirty) meshAddSubtract(combined_,combined_,axes_[i].current,axes_[i].previous);
    }
    allNeutral_=std::all_of(axes_.begin(),axes_.end(),[](const Axis &a){return a.neutral;});
    initialized_=true;return true;
}
void MeshCombinator::publish(std::vector<::motion::detail::MeshPoint> &points) const {
    if(!initialized_) throw std::runtime_error("meshCombinator: update required before publish");
    if(allNeutral_) {points.clear();return;}
    points.resize(16);
    for(size_t i=0;i<16;++i) points[i]={combined_[2*i],combined_[2*i+1]};
}
}
