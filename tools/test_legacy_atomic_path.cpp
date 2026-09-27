#include "../native/legacy_atomic_path.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <iostream>
#include <vector>
#include <csignal>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

static std::string Read(const std::string &path) {
    std::ifstream input(path,std::ios::binary);
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
static void Write(const std::string &path,const std::string &value) {
    std::ofstream output(path,std::ios::binary|std::ios::trunc);output<<value;output.close();assert(output.good());
}
int main() {
    char pattern[]="/tmp/studysteady-save-XXXXXX";
    const char *made=::mkdtemp(pattern);assert(made);
    const std::string root=made,slot=root+"/slot.dat",old="old save bytes";
    {
        // Match the Android probe's very first Open: no destination file yet.
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(::access(slot.c_str(),F_OK)==-1&&file->Length()==0);
        file->BeginSave();assert(file->Replace(old.data(),old.size(),0));
        assert(file->Close()&&Read(slot)==old);
    }
    Write(slot,old);
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(file->Length()==0&&Read(slot)==old);
        file->BeginSave(); // A serializer fails before Replace is reached.
        assert(file->Close());assert(Read(slot)==old);
    }
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        file->BeginSave();
        const std::string replacement="complete validated save";
        assert(file->Replace(replacement.data(),replacement.size(),0));
        assert(Read(slot)==replacement);assert(file->Close());
    }
    Write(slot,old);
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        file->BeginSave();
        assert(!file->Replace("bad",3,1)); // Prefix exceeds logical truncated length.
        assert(file->Close()&&Read(slot)==old);
    }
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        file->BeginSave();
        struct rlimit original{},limited{};assert(!getrlimit(RLIMIT_FSIZE,&original));
        limited=original;limited.rlim_cur=32;assert(!setrlimit(RLIMIT_FSIZE,&limited));
        const auto handler=std::signal(SIGXFSZ,SIG_IGN);
        const std::vector<char> payload(4096,'x');
        assert(!file->Replace(payload.data(),payload.size(),0));
        assert(!setrlimit(RLIMIT_FSIZE,&original));std::signal(SIGXFSZ,handler);
        assert(file->Close()&&Read(slot)==old); // Actual partial POSIX write failure.
    }
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(file->Write("ordinary",8,0)==8&&Read(slot)=="ordinary");
        assert(file->Write("!",1,8)==1&&Read(slot)=="ordinary!");
        assert(file->Close());
    }
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(file->Close()&&Read(slot).empty()); // Ordinary empty-create semantics.
    }
    Write(slot,old);
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        file->BeginSave();assert(file->StagePrefix("BMP-prefix",10,0));
        assert(file->Length()==10&&Read(slot)==old);
        file->BeginSave();assert(file->Close()&&Read(slot)==old);
    }
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(file->StagePrefix("BMP-prefix",10,0));file->BeginSave();
        assert(file->Replace("EMC-body",8,10)&&Read(slot)=="BMP-prefixEMC-body");
    }
    Write(slot,old);
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,5);assert(file);
        assert(file->StagePrefix("standalone-prefix",17,0));
        assert(file->Close()&&Read(slot)=="standalone-prefix");
    }
    Write(slot,old);
    {
        auto file=LegacyAtomicPath::OpenWithinRoot(root,slot,6);assert(file);
        char contents[3];assert(file->Read(contents,3,0)==3);
        assert(std::string(contents,3)=="old");file->BeginSave();
        assert(file->Replace("payload",7,4)&&Read(slot)=="old payload");
    }
    assert(!LegacyAtomicPath::IsWithinRoot(root,"/tmp/outside.dat"));
    const std::string symlink=root+"/link";
    assert(!::symlink(slot.c_str(),symlink.c_str()));
    assert(!LegacyAtomicPath::OpenWithinRoot(root,symlink,5));
    ::unlink(symlink.c_str());::unlink(slot.c_str());assert(!::rmdir(root.c_str()));
    std::cout<<"PASS: failed serializer, partial write, logical truncate, validated replace, ordinary I/O, prefix, scope and temp cleanup\n";
}
