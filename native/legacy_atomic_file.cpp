#include "legacy_compat/gls.h"
#include "legacy_atomic_file.h"
#include <limits>
#include <string>
#include "platform/log.h"

namespace {
bool ResolveNativeScheme(SSystem::SString &path) {
    // SStandardFileOpener::DirectPathOf only normalizes slashes, although its
    // NewOpenFile resolves local:// through the Android URL registry. Follow
    // that registry too before passing paths to POSIX realpath/open/rename.
    for(unsigned attempt=0;attempt<8;++attempt) {
        if(SSystem::g_defURLOpener.FindScheme(path)<0)return true;
        SSystem::SString next;
        if(SSystem::g_defURLOpener.DirectPathOf(next,path)||next==path)return false;
        path=next;
    }
    return false;
}
}

LegacyAtomicSaveFile::LegacyAtomicSaveFile(std::shared_ptr<LegacyAtomicPath> file,unsigned flags)
    :file_(std::move(file)){SetAttribute(flags);}
LegacyAtomicSaveFile *LegacyAtomicSaveFile::TryOpen(ECSEnvironment *environment,
    const wchar_t *path,unsigned flags,bool &candidate) {
    candidate=false;
    if(!environment||!(flags&modeWrite)||!path)return nullptr;
    auto *opener=environment->GetWritableFileOpener();
    if(!opener)return nullptr;
    SSystem::SString direct,root;
    if(opener->DirectPathOf(root,L""))return nullptr;
    std::wstring logical=static_cast<const wchar_t *>(environment->FilterFilePath(path));
    const bool full=!logical.empty()&&(logical[0]==L'/'||logical[0]==L'\\'||logical.find(L':')!=std::wstring::npos);
    if(!environment->m_fAcceptOtherSaveDir) {
        const auto colon=logical.find(L':');
        if(colon!=std::wstring::npos)logical.erase(0,colon+1);
        const auto first=logical.find_first_not_of(L"/\\");
        logical=first==std::wstring::npos?L"":logical.substr(first);
    }
    if(environment->m_fAcceptOtherSaveDir&&full) {
        if(SSystem::SFileOpener::DefaultDirectPathOf(direct,logical.c_str()))return nullptr;
    } else if(opener->DirectPathOf(direct,logical.c_str()))return nullptr;
    if(!ResolveNativeScheme(root)||!ResolveNativeScheme(direct))return nullptr;
    const std::string rootPath=root.ToCharArray().GetConstArray(),filePath=direct.ToCharArray().GetConstArray();
    if(!LegacyAtomicPath::IsWithinRoot(rootPath,filePath))return nullptr;
    candidate=true; // A temp-file failure must not fall back to truncating the slot.
    auto file=LegacyAtomicPath::OpenWithinRoot(rootPath,filePath,flags);
    return file?new LegacyAtomicSaveFile(std::move(file),flags):nullptr;
}
ESLFileObject *LegacyAtomicSaveFile::Duplicate() const {
    auto *copy=new LegacyAtomicSaveFile(file_,GetAttribute());copy->position_=position_;return copy;
}
unsigned long LegacyAtomicSaveFile::Read(void *data,unsigned long length) {
    const auto count=file_->Read(data,length,position_);position_+=count;return count;
}
unsigned long LegacyAtomicSaveFile::Write(const void *data,unsigned long length) {
    const auto count=file_->Write(data,length,position_);position_+=count;return count;
}
unsigned long LegacyAtomicSaveFile::GetLength() const {return GetLargeLength();}
unsigned long LegacyAtomicSaveFile::GetPosition() const {return position_;}
unsigned long LegacyAtomicSaveFile::Seek(long offset,SeekOrigin origin){return SeekLarge(offset,origin);}
UINT64 LegacyAtomicSaveFile::GetLargeLength() const {return file_->Length();}
UINT64 LegacyAtomicSaveFile::GetLargePosition() const {return position_;}
UINT64 LegacyAtomicSaveFile::SeekLarge(INT64 offset,SeekOrigin origin) {
    const UINT64 base=origin==FromBegin?0:origin==FromCurrent?position_:origin==FromEnd?GetLargeLength():UINT64_MAX;
    if(base==UINT64_MAX)return position_;
    if(offset<0){const UINT64 distance=UINT64(-(offset+1))+1;position_=distance>base?0:base-distance;}
    else if(UINT64(offset)<=UINT64_MAX-base)position_=base+UINT64(offset);
    return position_;
}
ESLError LegacyAtomicSaveFile::SetEndOfFile(){return file_->Truncate(position_)?eslErrSuccess:eslErrGeneral;}
void LegacyAtomicSaveFile::BeginSave(){file_->BeginSave();}
ESLError LegacyAtomicSaveFile::StagePrefix(const void *data,size_t length) {
    if(file_.use_count()!=1)return ESLErrorMsg("Cannot stage a thumbnail through duplicated writable handles");
    if(!file_->StagePrefix(data,length,position_))return ESLErrorMsg("Cannot stage thumbnail without changing the old slot");
    position_+=length;return eslErrSuccess;
}
ESLError LegacyAtomicSaveFile::Replace(const void *data,size_t length,UINT64 prefix) {
    if(file_.use_count()!=1)return ESLErrorMsg("Cannot atomically save through duplicated writable file handles");
    if(!file_->Replace(data,length,prefix))return ESLErrorMsg("Atomic save staging, validation or rename failed");
    position_=prefix+length;return eslErrSuccess;
}
ESLError LegacyAtomicSaveFile::FinishClose(){
    return file_.use_count()>1||file_->Close()?eslErrSuccess:eslErrGeneral;
}
