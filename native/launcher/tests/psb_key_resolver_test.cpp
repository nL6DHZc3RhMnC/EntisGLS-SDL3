#include "launcher/psb_key_resolver.h"
#include "psb_header.h"
#include "game_file_backend_fixture.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs=std::filesystem;
using entis::launcher::PsbKeyResolver;
using entis::launcher::PsbKeyError;
namespace {
void Require(bool ok,const char* why) { if (!ok) throw std::runtime_error(why); }
void U16(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t v) { b[at]=v;b[at+1]=v>>8; }
void U32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v) { for (unsigned i=0;i<4;++i) b[at+i]=v>>(i*8); }
std::vector<std::uint8_t> Psb(std::optional<std::uint32_t> seed={}) {
    std::vector<std::uint8_t> b(256,0);
    std::memcpy(b.data(),"PSB\0",4);U16(b,4,4);
    for (std::size_t i=8;i<56;i+=4) U32(b,i,56+static_cast<std::uint32_t>(i));
    U32(b,8,56);
    auto sum=adler32(1,b.data()+8,32);sum=adler32(sum,b.data()+44,12);U32(b,40,sum);
    if (seed) {
        U16(b,6,1);
        std::uint32_t x=123456789u,y=362436069u,z=521288629u,w=*seed,bytes=0;
        for (std::size_t i=8;i<56;++i) {
            if (!bytes) { const auto t=x^(x<<11);x=y;y=z;z=w;w=w^(w>>19)^t^(t>>8);bytes=w; }
            b[i]^=std::uint8_t(bytes);bytes>>=8;
        }
    }
    return b;
}
std::vector<std::uint8_t> Pe(const std::vector<std::string>& keys,bool pe64=false) {
    std::vector<std::uint8_t> b(2048,0);
    b[0]='M';b[1]='Z';U32(b,60,128);std::memcpy(b.data()+128,"PE\0\0",4);
    U16(b,132,pe64?0x8664:0x14c);U16(b,134,1);U16(b,148,pe64?240:224);U16(b,150,0x2102);
    U16(b,152,pe64?0x20b:0x10b);
    const std::size_t section=152+(pe64?240:224);
    std::memcpy(b.data()+section,".rdata",6);U32(b,section+8,1024);U32(b,section+12,4096);
    U32(b,section+16,1024);U32(b,section+20,512);U32(b,section+36,0x40000040);
    std::size_t at=520;
    for (const auto& key : keys) {
        std::memcpy(b.data()+at,key.data(),key.size());at+=key.size()+1;
        const char marker[]="#c#r#y#p#t#k#e#y#";
        std::memcpy(b.data()+at,marker,sizeof(marker));at+=sizeof(marker)+1;
    }
    return b;
}
void Write(const fs::path& p,const std::vector<std::uint8_t>& b) {
    std::ofstream out(p,std::ios::binary);out.write(reinterpret_cast<const char*>(b.data()),b.size());
    if (!out) throw std::runtime_error("test could not write fixture");
}
void WriteText(const fs::path& p,const std::string& s) { Write(p,{s.begin(),s.end()}); }
std::vector<std::uint8_t> Read(const fs::path& p) {
    std::ifstream in(p,std::ios::binary);if (!in) throw std::runtime_error("cannot read optional fixture");
    const auto size=fs::file_size(p);
    std::vector<std::uint8_t> result(size);
    if (size) in.read(reinterpret_cast<char*>(result.data()),static_cast<std::streamsize>(size));
    if (in.gcount()!=static_cast<std::streamsize>(size)) throw std::runtime_error("cannot read complete optional fixture");
    return result;
}
void Fails(const std::function<void()>& call,const char* contains) {
    try { call(); } catch (const PsbKeyError& e) {
        if (std::string(e.what()).find(contains)!=std::string::npos) return;
        throw std::runtime_error(std::string("wrong error: ")+e.what());
    }
    throw std::runtime_error(std::string("expected failure: ")+contains);
}
std::vector<fs::path> Caches(const fs::path& data) {
    std::vector<fs::path> result;
    const auto dir=data/"psb-key-cache";
    if (fs::exists(dir)) for (const auto& f:fs::directory_iterator(dir)) if (f.path().extension()==".cache") result.push_back(f.path());
    return result;
}
}
int main(int argc,char** argv) {
    const auto root=fs::temp_directory_path()/("entis-psb-resolver-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        fs::create_directories(root/"game");fs::create_directories(root/"data");
        const auto game=root/"game",data=root/"data",dll=game/"emote.DLL";
        // Synthetic values are deliberately unrelated to any shipping game.
        std::mt19937 random(0x51d173u);const std::uint32_t key=random(),other=random();
        const auto encrypted=Psb(key),plain=Psb();
        PsbKeyResolver automatic(game,data);
        Require(automatic.Resolve(plain.data(),plain.size())==0 && automatic.LastSource()=="unencrypted","plain PSB requested DLL");
        Require(Caches(data).empty(),"plain PSB wrote cache");
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"no game DLL");
        Write(dll,Pe({std::to_string(other)}));
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"candidates failed");
        Require(Caches(data).empty(),"failed key wrote cache");
        Write(dll,Pe({std::to_string(other),std::to_string(key)}));
        {
            const auto virtualRoot = root / "provider-game";
            auto backend = std::make_shared<MappedGameTestBackend>(game);
            GameTestMount mount(virtualRoot, backend);
            PsbKeyResolver linked(virtualRoot, {});
            Require(linked.Resolve(encrypted.data(), encrypted.size()) == key && linked.LastSource() == "dll",
                    "provider-backed original DLL must validate the actual PSB header");
            Require(backend->lists == 1 && backend->opens == 1 && !fs::exists(virtualRoot),
                    "DLL discovery uses selected provider without creating a copied resource tree");
        }
        Require(automatic.Resolve(encrypted.data(),encrypted.size())==key && automatic.LastSource()=="dll","did not select actual valid candidate");
        const auto firstCache=Caches(data);Require(firstCache.size()==1,"cache not written");
        Require(automatic.Resolve(encrypted.data(),encrypted.size())==key && automatic.LastSource()=="cache","validated cache not recognized");
        auto decoded=encrypted;studysteady::decodePsbHeader(decoded,key);
        Require(decoded==plain,"resolver stream disagrees with runtime decoder");
        WriteText(firstCache[0],"entis-psb-key-cache-v1\nsha256=forged\nkey="+std::to_string(other)+"\n");
        Require(automatic.Resolve(encrypted.data(),encrypted.size())==key && automatic.LastSource()=="dll","tampered cache selected key");
        // Full-file digest includes unrelated overlay bytes. Keep file size and
        // marker unchanged and ensure a fresh source fingerprint is produced.
        auto mutated=Pe({std::to_string(other),std::to_string(key)});mutated.back()=7;Write(dll,mutated);
        Require(automatic.Resolve(encrypted.data(),encrypted.size())==key && automatic.LastSource()=="dll","DLL overlay change reused cache");
        Require(Caches(data).size()==2,"full DLL content change did not change cache fingerprint");
        Write(dll,Pe({std::to_string(other)}));
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"candidates failed");
        // An explicit wrong value must never silently fall back to DLL discovery.
        Write(dll,Pe({std::to_string(key)}));
        PsbKeyResolver wrong(game,data,other),manual(game,data,key),zero(game,data,0);
        Fails([&]{wrong.Resolve(encrypted.data(),encrypted.size());},"configured psb_key");
        Require(manual.Resolve(encrypted.data(),encrypted.size())==key && manual.LastSource()=="explicit","manual key ignored");
        const auto z=Psb(0);Require(zero.Resolve(z.data(),z.size())==0 && zero.LastSource()=="explicit","explicit zero treated as absent");
        Require(wrong.Resolve(plain.data(),plain.size())==0,"plain PSB incorrectly applies configured key");
        Write(dll,Pe({"0"},true));
        Require(automatic.Resolve(z.data(),z.size())==0,"PE32+ zero key failed");
        // Independently calculated with Python hashlib.sha256 for this exact
        // synthetic PE32+ fixture and filename. Checks SHA padding/full blocks.
        Require(fs::exists(data/"psb-key-cache"/"5f9d17ff257d0705008a207a79a069ec6fd6620ae36eb2f0cc0ec2f85fe496e4.cache"),"SHA-256 fingerprint differs from independent reference");
        Write(dll,Pe({std::to_string(std::numeric_limits<std::uint32_t>::max())}));
        const auto max=Psb(std::numeric_limits<std::uint32_t>::max());
        Require(automatic.Resolve(max.data(),max.size())==std::numeric_limits<std::uint32_t>::max(),"max uint32 rejected");
        Write(dll,Pe({"4294967296","1234567890123","","-123","+123","prefix123"}));
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"No supported E-mote key marker");
        // Conflicting markers are all verified; duplicated copies of the same
        // valid key are one candidate, not false ambiguity.
        Write(dll,Pe({std::to_string(key),std::to_string(key),std::to_string(other)}));
        Write(game/"other.dll",Pe({std::to_string(key)}));
        Require(automatic.Resolve(encrypted.data(),encrypted.size())==key,"duplicate matching keys were ambiguous");
        fs::remove(game/"other.dll");
        auto invalid=Pe({std::to_string(key)});U32(invalid,376+20,2040);Write(dll,invalid);
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"No supported E-mote key marker");
        invalid=Pe({std::to_string(key)});U32(invalid,60,0xfffffff0);Write(dll,invalid);
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"No supported E-mote key marker");
        invalid=Pe({std::to_string(key)});std::memcpy(invalid.data()+376,".text\0\0\0",8);Write(dll,invalid);
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"No supported E-mote key marker");
        WriteText(dll,std::to_string(key)+std::string("\0#c#r#y#p#t#k#e#y#\0",18));
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"No supported E-mote key marker");
        auto corrupt=plain;corrupt[40]^=1;
        Fails([&]{automatic.Resolve(corrupt.data(),corrupt.size());},"invalid checksum");
        corrupt=encrypted;corrupt[25]^=1;
        Fails([&]{manual.Resolve(corrupt.data(),corrupt.size());},"configured psb_key");
        corrupt=plain;U16(corrupt,4,3);
        Fails([&]{automatic.Resolve(corrupt.data(),corrupt.size());},"Unsupported PSB");
        corrupt=plain;U16(corrupt,6,2);
        Fails([&]{automatic.Resolve(corrupt.data(),corrupt.size());},"Unsupported PSB");
        Fails([&]{automatic.Resolve(encrypted.data(),55);},"complete PSB header");
        Fails([&]{automatic.Resolve(nullptr,256);},"complete PSB header");
        Fails([&]{automatic.Resolve(plain.data(),56);},"invalid checksum");
        // A read-only/unusable cache location never blocks a verified game.
        Write(dll,Pe({std::to_string(key)}));WriteText(root/"blocked","file");
        PsbKeyResolver uncached(game,root/"blocked");
        Require(uncached.Resolve(encrypted.data(),encrypted.size())==key && !uncached.LastWarning().empty(),"cache write failure blocked valid key");
        fs::remove(dll);
        Fails([&]{automatic.Resolve(encrypted.data(),encrypted.size());},"no game DLL");
        // Optional real-world fixture: derive the answer from the user's DLL;
        // the source and test contain no expected shipping-game key.
        if (argc==3) {
            const auto raw=Read(argv[2]);PsbKeyResolver real(fs::path(argv[1]),root/"real-cache");
            const auto actual=real.Resolve(raw.data(),raw.size());
            auto copy=raw;studysteady::decodePsbHeader(copy,actual);
            Require(real.LastSource()=="dll" || real.LastSource()=="unencrypted","real fixture did not resolve");
            std::puts("Optional original DLL/PSB fixture: PASS (key not printed)");
        } else if (argc!=1) throw std::runtime_error("Usage: psb_key_resolver_test [game-dir raw-psb-file]");
        fs::remove_all(root);
        std::puts("PSB key resolver PASS: PE32/PE32+, multiple/invalid candidates, explicit override/zero, header validation, full-content cache invalidation, cache tampering, missing DLL, cache write failure");
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr,"PSB key resolver FAIL: %s\nFixtures retained at %s\n",e.what(),root.string().c_str());return 1;
    }
}
