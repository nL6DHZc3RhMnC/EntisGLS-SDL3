#include "legacy_compat/gls.h"
#include "legacy_atomic_save_probe.h"
#include "legacy_atomic_file.h"
#include "platform/log.h"
#include <cstring>
#include <memory>
#include <vector>
#include <unistd.h>

namespace {
bool Check(bool ok,const char *stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy atomic save probe FAIL: %s",stage);
    return ok;
}
class SaveImage final:public ECSExecutionImage {
public:
    SaveImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
class FailingSave final:public ECSString {
public:
    bool called=false;
    ESLError Save(ESLFileObject &file,ECSContext &) override {
        called=true;const char partial[]="partial serialized object";
        file.Write(partial,sizeof(partial));return eslErrNotSupported;
    }
};
std::vector<uint8_t> ReadSlot(SSystem::SFileOpener &opener,const wchar_t *path) {
    std::unique_ptr<SSystem::SFileInterface> file(opener.NewOpenFile(path,SSystem::SFileOpener::modeRead));
    if(!file||file->GetLength()<0||file->GetLength()>0x1000000)return {};
    std::vector<uint8_t> bytes(file->GetLength());
    if(file->Read(bytes.data(),bytes.size())!=bytes.size())return {};
    return bytes;
}
}

bool CheckLegacyAtomicSave(ECSEnvironment &environment) {
    auto *opener=environment.GetWritableFileOpener();
    if(!Check(opener!=nullptr,"actual savedata opener"))return false;
    const std::wstring directory=L".__atomic_probe_"+std::to_wstring(::getpid())+L"_"+std::to_wstring(timeGetTime());
    const std::wstring path=directory+L"/slot.dat";
    // SDK's default CreateSubDirectory mode is 0666, which cannot be traversed
    // on Android. Explicit owner rwx is required for an actual directory.
    if(!Check(!opener->CreateSubDirectory(directory.c_str(),SSystem::SFileOpener::permissionRWXU),
        "create isolated savedata test directory"))return false;
    struct Cleanup {
        SSystem::SFileOpener &opener;const std::wstring &directory,&path;
        ~Cleanup(){opener.RemoveSubFile(path.c_str());opener.RemoveSubDirectory(directory.c_str());}
    } cleanup{*opener,directory,path};
    SaveImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    ECSContext *previous=ECotophaScript::GetPrimaryContext();
    struct RestoreContext {
        ECSContext &context;ECSContext *previous;
        ~RestoreContext(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } restore{context,previous};
    if(!Check(!context.InitializeContext(&image),"initialize isolated save context"))return false;
    ECSString oldValue(L"以前の正常なセーブ"),newValue(L"new completed save 日本語");
    {
        ECSFile slot;
        const auto open=slot.Open(path.c_str(),ESLFileObject::modeCreate,&context);
        const bool atomic=dynamic_cast<LegacyAtomicSaveFile*>(slot.GetFileInterface())!=nullptr;
        if(!Check(!open&&atomic,"savedata Open resolves to real atomic local path")) {
            SSystem::SString root,direct;opener->DirectPathOf(root,L"");opener->DirectPathOf(direct,path.c_str());
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Atomic open details: error=%s atomic=%d root=%ls target=%ls",
                GetESLErrorMsg(open),int(atomic),static_cast<const wchar_t*>(root),static_cast<const wchar_t*>(direct));
            return false;
        }
        const auto saved=slot.SaveObject(oldValue,nullptr,context);
        if(!Check(!saved,"commit initial actual savedata object")) {
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Atomic initial save error: %s",GetESLErrorMsg(saved));return false;
        }
        if(!Check(!slot.Close(),"close committed atomic file"))return false;
    }
    const auto original=ReadSlot(*opener,path.c_str());
    if(!Check(!original.empty(),"old slot exists"))return false;
    {
        ECSFile slot;FailingSave failure;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context) && slot.GetFileLength()==0 &&
            ReadSlot(*opener,path.c_str())==original,"Open create exposes logical empty stream without truncating old slot"))return false;
        if(!Check(slot.SaveObject(failure,nullptr,context)!=eslErrSuccess&&failure.called&&
            ReadSlot(*opener,path.c_str())==original&&slot.GetFilePosition()==0,
            "partial serializer failure preserves old disk bytes and cursor"))return false;
        if(!Check(!slot.Close()&&ReadSlot(*opener,path.c_str())==original,"failure Close discards delayed truncate"))return false;
    }
    {
        ECSFile slot;FailingSave title;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context) &&
            slot.SaveObject(newValue,&title,context)!=eslErrSuccess&&title.called&&
            !slot.Close()&&ReadSlot(*opener,path.c_str())==original,"title serializer failure also preserves prior slot"))return false;
    }
    auto *badGlobal=new FailingSave;
    image.m_csgGlobal.AddVariable(L"unsupportedSaveProbe",badGlobal);
    for(bool raw:{false,true}) {
        ECSFile slot;badGlobal->called=false;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context) &&
            slot.SaveContext(nullptr,raw,context)!=eslErrSuccess&&badGlobal->called&&
            !slot.Close()&&ReadSlot(*opener,path.c_str())==original,
            raw?"uncompressed SaveContext failure preserves old slot":"compressed SaveContext failure preserves old slot"))return false;
    }
    {
        ECSFile slot;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context) &&
            !slot.SaveObject(newValue,nullptr,context)&&!slot.Close()&&ReadSlot(*opener,path.c_str())!=original,
            "successful save publishes a complete replacement"))return false;
        ECSObject *loaded=nullptr;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeRead,&context)&&!slot.LoadObject(loaded,context),
            "reopen and decode actual committed savedata"))return false;
        std::unique_ptr<ECSObject> owned(loaded);
        auto *string=ESLTypeCast<ECSString>(loaded);
        if(!Check(string&&string->m_varStr==newValue.m_varStr,"committed saved object contains complete replacement value"))return false;
    }
    {
        ECSFile memory;FailingSave failure;
        const char sentinel[]="memory bytes before failed save";
        if(!Check(!memory.CreateMemoryFile(64)&&memory.Write(sentinel,sizeof(sentinel))==sizeof(sentinel),"memory sentinel"))return false;
        memory.Seek(0,ESLFileObject::FromBegin);
        auto *originalStream=memory.GetFileInterface();
        if(!Check(memory.SaveObject(failure,nullptr,context)!=eslErrSuccess&&failure.called&&
            memory.GetFileInterface()==originalStream&&memory.GetFileLength()==sizeof(sentinel)&&memory.GetFilePosition()==0,
            "failed memory save preserves owned stream and cursor"))return false;
        char actual[sizeof(sentinel)];
        if(!Check(memory.Read(actual,sizeof(actual))==sizeof(actual)&&!std::memcmp(actual,sentinel,sizeof(actual)),"failed memory save preserves original bytes"))return false;
        memory.Seek(0,ESLFileObject::FromBegin);
        if(!Check(!memory.SaveObject(newValue,nullptr,context)&&memory.GetFileInterface()!=originalStream,
            "completed memory save swaps the owned buffer only after validation"))return false;
    }
    {
        const auto committed=ReadSlot(*opener,path.c_str());
        ECSFile untracked;
        // DefaultNewOpenFile is the global environment opener, so pass the
        // logical savedata path. Passing its already-resolved absolute path
        // would apply the environment's writable-root prefix a second time.
        const auto opened=untracked.Open(path.c_str(),ESLFileObject::modeReadWrite,nullptr);
        if(!Check(!opened,"open actual slot through untracked default-opener path")) {
            study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Atomic untracked Open error: %s",GetESLErrorMsg(opened));
            return false;
        }
        if(!Check(dynamic_cast<LegacyFileAdapter*>(untracked.GetFileInterface())!=nullptr,
            "untracked fixture uses ordinary SDK adapter"))return false;
        std::vector<uint8_t> before(untracked.GetFileLength());
        if(!Check(untracked.Read(before.data(),before.size())==before.size()&&before==committed,
            "untracked fixture opens the same committed slot"))return false;
        untracked.Seek(0,ESLFileObject::FromBegin);
        const auto rejected=untracked.SaveObject(newValue,nullptr,context);
        if(!Check(rejected!=eslErrSuccess,"untracked stream explicitly rejects non-atomic save"))return false;
        if(!Check(!untracked.Close(),"close rejected untracked save"))return false;
        if(!Check(ReadSlot(*opener,path.c_str())==committed,"untracked rejection preserves committed slot bytes"))return false;
    }
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady",
        "Legacy atomic save probe PASS: actual savedata Open/serializer/title/context rollback, closed failure, committed decode, memory swap and unsupported stream safety");
    return true;
}
