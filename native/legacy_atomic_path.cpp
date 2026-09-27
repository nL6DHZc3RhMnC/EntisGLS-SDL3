#include "legacy_atomic_path.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <limits>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#if defined(__ANDROID__)
#include "platform/log.h"
#endif

namespace {
constexpr unsigned createFlag=1, readFlag=2, writeFlag=4;
void OpenFailure(const char *stage,const std::string &path,unsigned flags,int code) {
#if defined(__ANDROID__)
    study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady",
        "Atomic path failure: stage=%s flags=0x%x errno=%d (%s) path=%s",
        stage,flags,code,std::strerror(code),path.c_str());
#else
    (void)stage;(void)path;(void)flags;(void)code;
#endif
    errno=code;
}
bool WriteAll(int fd,const void *data,size_t bytes) {
    const auto *source=static_cast<const uint8_t *>(data);
    while(bytes) {
        const ssize_t count=::write(fd,source,std::min<size_t>(bytes,0x100000));
        if(count<0&&errno==EINTR)continue;
        if(count<=0)return false;
        bytes-=count;source+=count;
    }
    return true;
}
bool ReadAllAt(int fd,void *data,size_t bytes,uint64_t at) {
    auto *out=static_cast<uint8_t *>(data);
    while(bytes) {
        const ssize_t count=::pread(fd,out,bytes,off_t(at));
        if(count<0&&errno==EINTR)continue;
        if(count<=0)return false;
        bytes-=count;out+=count;at+=count;
    }
    return true;
}
std::string Canonical(const std::string &path) {
    char *resolved=::realpath(path.c_str(),nullptr);
    if(!resolved)return {};
    std::string result(resolved);std::free(resolved);return result;
}
}

bool LegacyAtomicPath::IsWithinRoot(const std::string &root,const std::string &path) {
    const auto slash=path.find_last_of('/');
    if(slash==std::string::npos||slash+1==path.size())return false;
    const std::string leaf=path.substr(slash+1);
    if(leaf=="."||leaf=="..")return false;
    const std::string base=Canonical(root),parent=Canonical(path.substr(0,slash));
    if(base.empty()||parent.empty()||
        (parent!=base&&(parent.size()<=base.size()||parent.compare(0,base.size(),base)||parent[base.size()]!='/')))
        return false;
    return true;
}
std::shared_ptr<LegacyAtomicPath> LegacyAtomicPath::OpenWithinRoot(
    const std::string &root,const std::string &path,unsigned flags) {
    if(!(flags&writeFlag)){OpenFailure("not writable",path,flags,EINVAL);return {};}
    if(!IsWithinRoot(root,path)){OpenFailure("outside savedata",path,flags,errno?errno:EINVAL);return {};}
    const auto slash=path.find_last_of('/');
    const std::string parent=Canonical(path.substr(0,slash)),leaf=path.substr(slash+1);
    auto result=std::shared_ptr<LegacyAtomicPath>(new LegacyAtomicPath);
    result->path_=parent+"/"+leaf;result->flags_=flags;
    struct stat status{};
    if(::lstat(result->path_.c_str(),&status)==0) {
        // Do not follow symlinks out of savedata or replace directories/devices.
        if(!S_ISREG(status.st_mode)){OpenFailure("target not regular",result->path_,flags,EINVAL);return {};}
    } else if(errno!=ENOENT){OpenFailure("lstat target",result->path_,flags,errno);return {};}
    if(flags&createFlag) {
        if(!result->NewTemporary(result->temporary_,result->fd_))return {};
    } else {
        result->fd_=::open(result->path_.c_str(),O_RDWR|O_CLOEXEC|O_NOFOLLOW);
        if(result->fd_<0){OpenFailure("open existing",result->path_,flags,errno);return {};}
    }
    return result;
}
LegacyAtomicPath::~LegacyAtomicPath(){Close();}
bool LegacyAtomicPath::NewTemporary(std::string &path,int &fd) const {
    std::vector<char> name(path_.begin(),path_.end());
    constexpr char suffix[]=".tmp-XXXXXX";
    name.insert(name.end(),suffix,suffix+sizeof(suffix));
    fd=::mkstemp(name.data());
    if(fd<0){OpenFailure("mkstemp",name.data(),flags_,errno);return false;}
    path=name.data();
    if(::fcntl(fd,F_SETFD,FD_CLOEXEC)<0) {
        const int error=errno;::close(fd);fd=-1;::unlink(path.c_str());path.clear();
        OpenFailure("fcntl close-on-exec",name.data(),flags_,error);return false;
    }
    return true;
}
size_t LegacyAtomicPath::Read(void *data,size_t length,uint64_t offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(fd_<0||!(flags_&readFlag)||offset>uint64_t(std::numeric_limits<off_t>::max()))return 0;
    ssize_t count;
    do { count=::pread(fd_,data,length,off_t(offset)); } while(count<0&&errno==EINTR);
    return count>0?size_t(count):0;
}
size_t LegacyAtomicPath::Write(const void *data,size_t length,uint64_t offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(fd_<0||!(flags_&writeFlag)||offset>uint64_t(std::numeric_limits<off_t>::max()))return 0;
    ssize_t count;
    do {count=::pwrite(fd_,data,length,off_t(offset));}while(count<0&&errno==EINTR);
    if(count<0)return 0;
    // An ordinary write is explicit permission to publish the logical Open.
    discard_=false;
    if(!temporary_.empty()&&!PublishOpen()){discard_=true;return 0;}
    return size_t(count);
}
uint64_t LegacyAtomicPath::Length() const {
    std::lock_guard<std::mutex> lock(mutex_);struct stat info{};
    return fd_>=0&&!::fstat(fd_,&info)&&info.st_size>=0?uint64_t(info.st_size):0;
}
bool LegacyAtomicPath::Truncate(uint64_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(fd_<0||length>uint64_t(std::numeric_limits<off_t>::max())||::ftruncate(fd_,off_t(length)))return false;
    discard_=false;
    if(!temporary_.empty()&&!PublishOpen()){discard_=true;return false;}
    return true;
}
void LegacyAtomicPath::BeginSave(){std::lock_guard<std::mutex> lock(mutex_);discard_=true;}
bool LegacyAtomicPath::StagePrefix(const void *bytes,size_t length,uint64_t offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    discard_=true;
    struct stat current{};
    if(fd_<0||::fstat(fd_,&current)||current.st_size<0||
        offset>uint64_t(std::numeric_limits<off_t>::max())-length)return false;
    std::string staging;int output=-1;
    if(!NewTemporary(staging,output))return false;
    struct Cleanup {std::string &path;int &fd;~Cleanup(){if(fd>=0)::close(fd);if(!path.empty())::unlink(path.c_str());}} cleanup{staging,output};
    uint8_t block[0x10000];
    for(uint64_t at=0;at<uint64_t(current.st_size);) {
        const size_t count=std::min<uint64_t>(sizeof(block),uint64_t(current.st_size)-at);
        if(!ReadAllAt(fd_,block,count,at)||!WriteAll(output,block,count))return false;
        at+=count;
    }
    if(::lseek(output,off_t(offset),SEEK_SET)<0||!WriteAll(output,bytes,length))return false;
    // Keep the complete BMP prefix private until SaveObject/SaveContext commits
    // it together with the EMC body. An ordinary Close still saves a BMP-only
    // file; any later failed save sets discard_ again and preserves the old slot.
    if(fd_>=0)::close(fd_);
    if(!temporary_.empty())::unlink(temporary_.c_str());
    fd_=output;output=-1;temporary_=staging;staging.clear();discard_=false;
    return true;
}
bool LegacyAtomicPath::PublishOpen() {
    if(temporary_.empty())return true;
    if(fd_<0||::fsync(fd_))return false;
    if(::close(fd_)){fd_=-1;return false;}fd_=-1;
    if(::rename(temporary_.c_str(),path_.c_str()))return false;
    temporary_.clear();discard_=false;
    // Rename is the commit point. An external actor changing permissions after
    // it cannot turn this completed save back into a failed/rolled-back save.
    fd_=::open(path_.c_str(),O_RDWR|O_CLOEXEC|O_NOFOLLOW);
    return true;
}
bool LegacyAtomicPath::Replace(const void *bytes,size_t length,uint64_t prefix) {
    std::lock_guard<std::mutex> lock(mutex_);
    discard_=true;
    struct stat current{};
    if(fd_<0||::fstat(fd_,&current)||current.st_size<0||prefix>uint64_t(current.st_size)||
        prefix>uint64_t(std::numeric_limits<off_t>::max())-length)return false;
    std::string staging;int output=-1;
    if(!NewTemporary(staging,output))return false;
    struct Cleanup {
        std::string &path;int &fd;
        ~Cleanup(){if(fd>=0)::close(fd);if(!path.empty())::unlink(path.c_str());}
    } cleanup{staging,output};
    uint8_t block[0x10000];
    for(uint64_t at=0;at<prefix;) {
        const size_t count=std::min<uint64_t>(sizeof(block),prefix-at);
        if(!ReadAllAt(fd_,block,count,at)||!WriteAll(output,block,count))return false;
        at+=count;
    }
    if(!WriteAll(output,bytes,length)||::fsync(output))return false;
    if(::close(output)){output=-1;return false;}output=-1;
    // Reopen the completed file and compare every payload byte before commit.
    output=::open(staging.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(output<0)return false;
    const auto *expected=static_cast<const uint8_t *>(bytes);
    for(size_t at=0;at<length;) {
        const size_t count=std::min(sizeof(block),length-at);
        if(!ReadAllAt(output,block,count,prefix+at)||std::memcmp(block,expected+at,count))return false;
        at+=count;
    }
    if(::close(output)){output=-1;return false;}output=-1;
    if(::rename(staging.c_str(),path_.c_str()))return false;
    staging.clear();
    if(fd_>=0)::close(fd_);
    if(!temporary_.empty())::unlink(temporary_.c_str());
    temporary_.clear();discard_=false;
    fd_=::open(path_.c_str(),O_RDWR|O_CLOEXEC|O_NOFOLLOW);
    return true;
}
bool LegacyAtomicPath::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    bool success=true;
    if(!temporary_.empty()&&!discard_)success=PublishOpen();
    if(fd_>=0){if(::close(fd_))success=false;fd_=-1;}
    if(!temporary_.empty()){::unlink(temporary_.c_str());temporary_.clear();}
    return success;
}
