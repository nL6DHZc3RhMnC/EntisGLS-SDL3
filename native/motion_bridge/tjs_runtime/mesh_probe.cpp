#include "mesh_combinator.h"
#include "probe_asset.h"
#include <fstream>
#include <iostream>
#include <set>
#include <cstring>
#include <limits>
#include "tjsDictionary.h"
#include "tjsArray.h"

using namespace studysteady::motion;
namespace {
tTJSVariant dictionary(std::initializer_list<std::pair<const tjs_char *,tTJSVariant>> entries) {
    auto *d=TJSCreateDictionaryObject();tTJSVariant out(d,d);d->Release();
    for(const auto &e:entries)d->PropSet(TJS_MEMBERENSURE,e.first,nullptr,&e.second,d);
    return out;
}
void checkRejections(const tTJSVariant &source) {
    using namespace ::motion::detail;
    const auto first=motionPropGetByNum(motionPropGet(source,TJS_W("combinatorList")),0);
    const auto originalVar=motionPropGet(first,TJS_W("variable"));
    auto make=[&](int type,const tTJSVariant &raw,const tTJSVariant &variable){
        auto axis=dictionary({{TJS_W("meshType"),tTJSVariant(type)},
            {TJS_W("neutralIndex"),motionPropGet(first,TJS_W("neutralIndex"))},
            {TJS_W("rawMeshList"),raw},{TJS_W("variable"),variable}});
        auto *a=TJSCreateArrayObject();tTJSVariant array(a,a);a->Release();
        a->PropSetByNum(TJS_MEMBERENSURE,0,&axis,a);
        return dictionary({{TJS_W("combinatorList"),array}});
    };
    auto reject=[](const tTJSVariant &v){
        try {MeshCombinator m(v);}catch(const std::runtime_error &){return;}
        throw std::runtime_error("malformed/unsupported mesh was accepted");
    };
    const auto raw=motionPropGet(first,TJS_W("rawMeshList"));
    reject(make(2,raw,originalVar));
    const uint8_t shortData[127]={};
    reject(make(1,tTJSVariant(shortData,127),originalVar));
    const auto zeroRange=dictionary({
        {TJS_W("key"),motionPropGet(originalVar,TJS_W("key"))},
        {TJS_W("rangeBegin"),tTJSVariant(0)}, {TJS_W("rangeEnd"),tTJSVariant(0)},
        {TJS_W("meshCount"),motionPropGet(originalVar,TJS_W("meshCount"))}});
    reject(make(1,raw,zeroRange));
    MeshCombinator m(source);m.update({});
    try {m.update({{m.axes().front().key,std::numeric_limits<float>::quiet_NaN()}});}
    catch(const std::runtime_error &){return;}
    throw std::runtime_error("nonfinite variable accepted");
}
}
int main(int argc,char **argv) {
    try {
        // Binary math-only protocol used by the Windows machine-code oracle.
        // Records: uint32 operation (0=lerp,1=add,2=add-sub), float ratio,
        // three arrays of 32 floats. Output is exactly 32 floats per record.
        if(argc==4 && std::string(argv[1])=="--kernels") {
            std::ifstream in(argv[2],std::ios::binary);std::ofstream out(argv[3],std::ios::binary);
            if(!in || !out) throw std::runtime_error("kernel file open failed");
            uint32_t op;float ratio;MeshPatch a,b,c,r;
            while(in.read(reinterpret_cast<char *>(&op),4)) {
                in.read(reinterpret_cast<char *>(&ratio),4);
                in.read(reinterpret_cast<char *>(a.data()),sizeof(a));
                in.read(reinterpret_cast<char *>(b.data()),sizeof(b));
                in.read(reinterpret_cast<char *>(c.data()),sizeof(c));
                if(!in || op>2) throw std::runtime_error("invalid kernel input");
                if(op==0) meshInterpolate(r,a,b,ratio);
                else if(op==1)meshAdd(r,a,b);
                else meshAddSubtract(r,a,b,c);
                out.write(reinterpret_cast<const char *>(r.data()),sizeof(r));
            }
            return 0;
        }
        if(argc!=3) {std::cerr<<"Usage: motion_mesh_probe haz_a.psb header_seed\n";return 2;}
        spdlog::stdout_color_mt("tjs2");spdlog::stdout_color_mt("core");
        auto *engine=new tTJS();motionSetTjsEngine(engine);
        {
            motionSetPsbHeaderSeed(static_cast<uint32_t>(std::stoull(argv[2])));
            PSB::PSBFile file;if(!file.LoadStorage(ttstr(argv[1])))throw std::runtime_error("PSB load failed");
            size_t nodes=0,axes=0,nonNeutral=0,steps=0,published=0;
            std::set<std::string> keys;
            probeMotions(file,[&](const auto &name,const auto &content){
                probeLayers(::motion::detail::motionPropGet(content,TJS_W("layer")),name,[&](const auto &path,const auto &layer){
                    const auto value=::motion::detail::motionPropGet(layer,TJS_W("meshCombinator"));
                    if(value.Type()==tvtVoid)return;
                    if(nodes==0)checkRejections(value);
                    MeshCombinator mesh(value);++nodes;axes+=mesh.axes().size();
                    if(!mesh.update({}) || !mesh.allNeutral())throw std::runtime_error(path+": neutral state mismatch");
                    std::vector<::motion::detail::MeshPoint> points;
                    mesh.publish(points);if(!points.empty())throw std::runtime_error("neutral publish must clear");
                    if(mesh.update({}))throw std::runtime_error("unchanged mesh reported dirty");
                    for(const auto &axis:mesh.axes()) {
                        keys.insert(axis.key);
                        for(float v:{axis.begin,(3*axis.begin+axis.end)/4,(axis.begin+3*axis.end)/4,axis.end}) {
                            MeshVariables vars{{axis.key,v}};
                            mesh.update(vars);++steps;
                            if(!mesh.allNeutral())++nonNeutral;
                            mesh.publish(points);published+=points.size();
                            if(points.size()!=(mesh.allNeutral()?0u:16u))throw std::runtime_error(path+": output size disagrees with neutral state");
                        }
                        mesh.update({});mesh.publish(points);
                        if(!points.empty())throw std::runtime_error(path+": return-to-neutral failed");
                    }
                });
            });
            if(nodes!=78 || axes!=278)throw std::runtime_error("unexpected haz_a combinator coverage");
            std::cout<<"real PSB/TJS meshCombinator nodes="<<nodes<<" axes="<<axes<<" variables="<<keys.size()<<" nonneutral_steps="<<nonNeutral<<" published_points="<<published<<": PASS\n";
            std::cout<<"DLL-derived mesh update steps="<<steps<<" all neutral outputs cleared; rendering not performed\n";
            std::cout<<"unsupported type, truncated mesh, zero range, nonfinite variable rejection: PASS\n";
        }
        engine->Shutdown();engine->Release();motionSetTjsEngine(nullptr);return 0;
    } catch(const eTJSError &e) {std::cerr<<"TJS error: "<<e.GetMessage().AsStdString()<<'\n';return 1;}
      catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
}
