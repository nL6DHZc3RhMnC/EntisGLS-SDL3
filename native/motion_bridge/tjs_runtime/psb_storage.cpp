#include "psb_storage.h"
#include "tjs_host.h"
#include "tjsUtils.h"
#include "psbfile/PSBRawFile.h"
#include "psb_header.h"
#include <cstring>
#include <fstream>
#include <filesystem>
#include <thread>
namespace {
uint32_t headerSeed = 0;
StudyMotionPsbKeyResolver keyResolver=nullptr;
void *keyResolverUser=nullptr;
std::thread::id keyResolverThread;
StudyMotionRead storageRead=nullptr;
StudyMotionRelease storageRelease=nullptr;
void *storageUser=nullptr;
std::thread::id storageThread;
void checkStorageThread() {
    if(storageRead && storageThread!=std::this_thread::get_id())
        throw std::runtime_error("motion storage callback called from wrong thread");
}
}
void motionSetPsbHeaderSeed(uint32_t seed) { headerSeed = seed; }
void motionSetPsbKeyResolver(StudyMotionPsbKeyResolver resolver,void *user) {
    if(keyResolver && keyResolverThread!=std::this_thread::get_id())
        throw std::runtime_error("motion PSB key resolver called from wrong thread");
    keyResolver=resolver;keyResolverUser=user;keyResolverThread=std::this_thread::get_id();
}
void motionSetStorageReader(StudyMotionRead read,StudyMotionRelease release,void *user) {
    checkStorageThread();
    if(bool(read)!=bool(release))throw std::runtime_error("motion reader and release callbacks must be set together");
    storageRead=read;storageRelease=release;storageUser=user;storageThread=std::this_thread::get_id();
}
ttstr TVPGetPlacedPath(const ttstr &name) {
    if(storageRead){checkStorageThread();return name;}
    return ttstr(std::filesystem::weakly_canonical(std::filesystem::path(name.AsStdString())).string());
}
bool TVPIsExistentStorage(const ttstr &name) {
    if(storageRead) {
        checkStorageThread();char error[512]={};size_t size=0;
        const int status=storageRead(storageUser,name.AsStdString().c_str(),nullptr,&size,error,sizeof(error));
        error[sizeof(error)-1]=0;
        if(status<0)throw std::runtime_error(error[0]?error:"motion archive existence query failed");
        if(status>1)throw std::runtime_error("motion archive existence query returned invalid status");
        return status==1;
    }
    return std::filesystem::is_regular_file(std::filesystem::path(name.AsStdString()));
}
// One storage/decode/adoption boundary for filesystem and host NOA callbacks.
bool PSB::PSBFile::LoadStorage(const ttstr &name,const OwnerFilter &filter) {
    std::vector<uint8_t> bytes;
    if(storageRead) {
        checkStorageThread();char error[512]={};void *buffer=nullptr;size_t size=0;
        const auto freeBuffer=storageRelease;void *const user=storageUser;
        const int status=storageRead(user,name.AsStdString().c_str(),&buffer,&size,error,sizeof(error));
        struct Release {StudyMotionRelease callback;void *user;void *buffer;size_t size;~Release(){if(buffer)callback(user,buffer,size);}} release{freeBuffer,user,buffer,size};
        error[sizeof(error)-1]=0;
        if(status<0)throw std::runtime_error(error[0]?error:"motion archive read failed");
        if(status==0)return false;
        if(status!=1 || !buffer || size<56 || size>256*1024*1024)
            throw std::runtime_error("PSB archive buffer/size out of range");
        const auto *data=static_cast<const uint8_t *>(buffer);
        bytes.assign(data,data+size);
    } else {
    std::ifstream input(name.AsStdString(),std::ios::binary|std::ios::ate);
    if(!input) return false;
    const auto size = input.tellg();
    if(size<56 || size>256*1024*1024) throw std::runtime_error("PSB storage size out of range");
    bytes.resize(static_cast<size_t>(size));
    input.seekg(0);input.read(reinterpret_cast<char *>(bytes.data()),bytes.size());
    if(!input) throw std::runtime_error("PSB storage read failed");
    }
    uint32_t seed=headerSeed;
    // Reject unsupported files before consulting a resolver. It receives the
    // existing storage buffer directly; resolving keys needs no whole-file copy.
    if(bytes.size()<56 || std::memcmp(bytes.data(),"PSB\0",4))
        throw std::runtime_error("Not a PSB file");
    const uint16_t version=bytes[4] | (bytes[5]<<8);
    const uint16_t flags=bytes[6] | (bytes[7]<<8);
    if(version!=4 || (flags & ~1u))
        throw std::runtime_error("Unsupported PSB version/encryption flags");
    if((flags & 1u) && keyResolver) {
        if(keyResolverThread!=std::this_thread::get_id())
            throw std::runtime_error("motion PSB key resolver called from wrong thread");
        char error[1024]={};seed=0;
        const int status=keyResolver(keyResolverUser,name.AsStdString().c_str(),
            bytes.data(),bytes.size(),&seed,error,sizeof(error));
        error[sizeof(error)-1]=0;
        if(status!=1) {
            if(status!=0 && status!=-1)
                throw std::runtime_error("motion PSB key resolver returned invalid status");
            throw std::runtime_error(error[0]?error:"No decryption key available for encrypted PSB; configure psb_key or supply the game's E-mote DLL");
        }
    }
    studysteady::decodePsbHeader(bytes,seed);
    auto *data = static_cast<uint8_t *>(TJSAlignedAlloc(bytes.size(),4));
    std::memcpy(data,bytes.data(),bytes.size());
    if(!Adopt(data,bytes.size(),filter)) {
        // With a filter, Adopt may already own the buffer. Only free it when
        // adoption failed before publishing the raw owner.
        if(!GetOwner() || GetOwner()->GetData()!=data) TJSAlignedDealloc(data);
        return false;
    }
    return true;
}
