#include "legacy_compat/gls.h"
#include "legacy_setup.h"
#include "legacy_window.h"
#include <sakura/ssys_std_ui.h>
#include <sakuracl/erisa/sgl_erisa_md5_context.h>
#include <sakuracl/erisa/sgl_erisa_crc32_context.h>
#include "platform/log.h"
#include "platform/environment.h"
#include <array>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cwctype>
#include <string>
#include <vector>

IMPLEMENT_CLASS_INFO(ECSSetup,ECSObject)
const wchar_t *	ECSSetup::m_pwszFuncName[61] =
{
	// ユーザーインターフェース関数
	L"CreateInstallationDialog",
	L"CloseInstallationDialog",
	L"IsInstallationDialogCanceled",
	L"SetInstallationDialogFileText",
	L"SetInstallationDialogProgress",
	L"InstallationMessageBox",
	// インストール支援関数
	L"GetFontList",
	L"GetWindowsProductID",	L"MakeMD5Digest",
	L"CalcCRC32",	L"CheckSum32",
	L"GetDesktopDirectory",	L"GetStartMenuDirectory",
	L"GetAppDataDirectory",	L"GetWindowsDirectory",
	L"GetCurrentModulePath",	L"GetEnvironmentVariable",
	L"FilterEnvironmentPath",
	L"GetDiskVolumeName",	L"GetDiskSerialNumber",
	L"GetDiskFreeSpace",	L"ShellExecute",
	L"ExecuteProcess",	L"GetExecuteExitCode",
	L"BrowseForFolder", L"BrowseFileDialog",
	// インストールファイルリスト関数
	L"ReadInstalledLog",	L"WriteInstalledLog",
	L"MeasureInstallSize",	L"AddInstallDirectory",
	L"AddInstallArchiveDirectory",	L"AddInstallFile",
	L"AddInstallArchiveTree",	L"AddInstallDirectoryTree",
	// インストール処理関数
	L"BootCheck",	L"ReleaseBootCheck",
	L"GetLastErrorMsg",	L"BeginInstall",
	L"IsFinishedInstall",	L"InstallNextFile",
	L"GetCurrentCopiedBytes",	L"GetTotalCopiedBytes",
	L"WaitForCurrentCopy",	L"EndInstall",
	L"AddInstallFileLog",
	L"InstallCreateDirectory",	L"InstallCreateShortcutFile",
	// レジストリ関数
	L"GetUninstallInfo",	L"RegisterUninstall",
	L"GetRegUninstallInteger32",	L"GetRegUninstallInteger64",
	L"GetRegUninstallString",	L"SetRegUninstallInteger32",
	L"SetRegUninstallInteger64",	L"SetRegUninstallString",
	// アンインストール関数
	L"Uninstall",	L"DeleteInstalledFile",
	L"IsNecessaryRebootToDelete", L"UnRegisterUninstall",
	L"RebootWindows",
	NULL
} ;
ECSSetup::ECSSetup() { m_vtType=csvtObject; }
const wchar_t *ECSSetup::GetTypeName() const { return L"Setup"; }
ECSObject *ECSSetup::GetTypeOf(const wchar_t *name) { return !EWideString::Compare(name,L"Setup")?this:ECSObject::GetTypeOf(name); }
ECSObject *ECSSetup::Duplicate() { return new ECSSetup; }
ESLError ECSSetup::Move(ECSContext &context,ECSObject *object) {
    if(!ESLTypeCast<ECSSetup>(ECSObject::GetEntity(object)))return ESLErrorMsg("Setup assignment requires Setup");
    context.delete_CSObject(object);return eslErrSuccess;
}
ESLError ECSSetup::UnaryOperate(ECSContext &,CSUnaryOperatorType) { return ESLErrorMsg("Setup has no unary operator"); }
ESLError ECSSetup::Operate(ECSContext &,CSOperatorType,ECSObject *) { return ESLErrorMsg("Setup has no arithmetic operator"); }
ESLError ECSSetup::Compare(ECSContext &,int &,CSCompareType,ECSObject &) { return ESLErrorMsg("Setup has no value comparison"); }
ESLError ECSSetup::GetFunction(ECSContext &,int &index,const wchar_t *name) {
    for(index=0;index<60;++index)if(!EWideString::Compare(name,m_pwszFuncName[index]))return eslErrSuccess;
    index=-1;return ESLErrorMsg("Unknown Setup method");
}
EWideString ECSSetup::MakeMD5Digest(const void *bytes,size_t length) {
    SakuraCL::MD5Context digest;digest.Stream(static_cast<const uint8_t *>(bytes),length);digest.Flush();
    SSystem::SString text;digest.GetMD5DigestHex(text);text.MakeUpper();return EWideString(text);
}
uint32_t ECSSetup::CalcCRC32(const void *bytes,size_t length) {
    SakuraCL::CRC32Context crc;crc.Stream(static_cast<const uint8_t *>(bytes),length);return crc.GetCRC32();
}
uint32_t ECSSetup::CheckSum32(const void *bytes,size_t length) {
    // Win32's original char is signed; preserve its sign extension and end-
    // around carry rather than depending on Android's unsigned-char default.
    const auto *data=static_cast<const int8_t *>(bytes);uint32_t sum=0;
    for(size_t i=0;i<length;++i) {
        const auto word=static_cast<uint32_t>(static_cast<int32_t>(data[i]))<<((i&3)*8);
        const bool carry=word>~sum;sum+=word;if(carry)++sum;
    }
    return sum;
}
int ECSSetup::MessageBoxStyleToAndroid(int style) {
    // Win32 flags other than the button group (e.g. MB_ICONERROR) do not select
    // different Android buttons. No icon is fabricated for those flags.
    static constexpr int styles[]={0,1,5,3,2,4};
    return (style&15)<6?styles[style&15]:-1;
}
int ECSSetup::MessageBoxResultToWindows(int result) {
    static constexpr int windows[]={1,2,6,7,4,3,5};
    return result>=0&&result<7?windows[result]:0;
}
ESLError ECSSetup::StreamArgument(ECSContext &context,ECSObjArray<ECSObject> &args,
    const std::function<void(const uint8_t *,size_t)> &consume) {
    auto *object=ESLTypeCast<ECSFile>(context.GetArgumentObjectAs(args,1,L"File"));
    if(object) {
        auto *file=object->GetFileInterface();if(!file)return ESLErrorMsg("Setup digest requires an open File");
        const auto end=file->GetLargeLength(),start=file->GetLargePosition();
        if(start>end)return eslErrInvalidParam;
        std::array<uint8_t,16384> buffer;auto remaining=end-start;
        while(remaining) {
            const auto count=static_cast<size_t>(std::min<UINT64>(remaining,buffer.size()));
            const auto read=file->Read(buffer.data(),count);if(read!=count)return eslErrGeneral;
            consume(buffer.data(),read);remaining-=read;
        }
        return eslErrSuccess;
    }
    EWideString wide;const auto error=context.GetArgumentAsStr(wide,args,1,L"");if(error)return error;
    const EString encoded(wide); // Original API hashes CP932 bytes, not native wchar_t.
    consume(reinterpret_cast<const uint8_t *>(static_cast<const char *>(encoded)),encoded.GetLength());
    return eslErrSuccess;
}
ESLError ECSSetup::CallFunction(ECSContext &context,int index,ECSObjArray<ECSObject> &args) {
    if(index<0||index>=60)return ESLErrorMsg("Unknown Setup method");
    const auto named=[&](const wchar_t *name){return !EWideString::Compare(name,m_pwszFuncName[index]);};
    const auto push=[&](int64_t value)->ESLError{return context.PushObject(new ECSInteger(value));};
    ESLError error;
    if(named(L"InstallationMessageBox")) {
        if((error=context.VerifyArgumentCount(args,3,5)))return error;
        EWideString message,caption;int style;
        if((error=context.GetArgumentAsStr(message,args,1,L"")) ||
           (error=context.GetArgumentAsStr(caption,args,2,L"")) ||
           (error=context.GetArgumentAsInt(style,args,3,0)))return error;
        const int nativeStyle=MessageBoxStyleToAndroid(style);
        if(nativeStyle<0)return ESLErrorMsg("Setup.MessageBox button group is unsupported");
        auto *window=ESLTypeCast<ECSWindow>(context.GetArgumentObjectAs(args,4,L"Window"));
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Game message: %s",
            SSystem::SString(message).ToCharArray().GetConstArray());
        return push(MessageBoxResultToWindows(SSystem::MessageBox(message,caption,nativeStyle,window?window->GetWindow():nullptr)));
    }
    if(named(L"BrowseFileDialog")) {
        if((error=context.VerifyArgumentCount(args,5,6)))return error;
        auto* output=ESLTypeCast<ECSString>(context.GetArgumentObjectAs(args,1,L"String"));
        if(!output)return ESLErrorMsg("BrowseFileDialog requires a mutable String path");
        EWideString caption,filterText;int save;
        if((error=context.GetArgumentAsInt(save,args,2,0))||
           (error=context.GetArgumentAsStr(caption,args,3,L""))||
           (error=context.GetArgumentAsStr(filterText,args,4,L"")))return error;
        auto* window=ESLTypeCast<ECSWindow>(context.GetArgumentObjectAs(args,5,L"Window"));
        // GLS3 packs label/pattern pairs with '|'. Android UIFileChoiceDialog
        // expects bare extension names; its filter adds the dot itself.
        std::vector<std::wstring> filters;
        const std::wstring packed=filterText.CharPtr()?filterText.CharPtr():L"";
        for(size_t begin=0;begin<packed.size();) {
            const auto end=packed.find(L'|',begin);
            filters.push_back(packed.substr(begin,end==std::wstring::npos?end:end-begin));
            if(end==std::wstring::npos)break;begin=end+1;
        }
        while(!filters.empty()&&filters.back().empty())filters.pop_back();
        if(filters.size()%2)return ESLErrorMsg("BrowseFileDialog filter requires label/pattern pairs");
        bool allFiles=filters.empty();
        for(size_t i=1;i<filters.size();i+=2) {
            std::wstring translated;
            for(size_t begin=0;begin<filters[i].size();) {
                const auto end=filters[i].find(L';',begin);
                std::wstring extension=filters[i].substr(begin,end==std::wstring::npos?end:end-begin);
                if(extension==L"*"||extension==L"*.*")allFiles=true;
                else {
                    if(extension.compare(0,2,L"*.")==0)extension.erase(0,2);
                    if(extension.empty()||extension.find_first_of(L"*?/\\")!=std::wstring::npos)
                        return ESLErrorMsg("BrowseFileDialog requires extension-only file filters on Android");
                    for(auto& ch:extension)ch=std::towlower(ch);
                    if(!translated.empty())translated+=L';';translated+=extension;
                }
                if(end==std::wstring::npos)break;begin=end+1;
            }
            filters[i]=std::move(translated);
        }
        std::vector<const wchar_t*> filterPointers;
        for(const auto& part:filters)filterPointers.push_back(part.c_str());filterPointers.push_back(nullptr);
        SSystem::SString directory;
        std::wstring original=output->m_varStr.CharPtr()?output->m_varStr.CharPtr():L"";
        for(auto& ch:original)if(ch==L'\\')ch=L'/';
        const auto slash=original.find_last_of(L'/');
        if(!original.empty()&&original[0]==L'/'&&slash!=std::wstring::npos)
            directory=original.substr(0,slash?slash:1).c_str();
        if(directory.IsEmpty()) {
            if(SSystem::SFile::GetDefaultDirectory(directory,SSystem::SFile::DefaultDirectory::AndroidExternalStoragePrivate)||directory.IsEmpty())
                if(SSystem::SFile::GetDefaultDirectory(directory,SSystem::SFile::DefaultDirectory::ApplicationData)||directory.IsEmpty())return push(eslErrGeneral);
            directory+=L"/screenshots";
            // The SDK's default Unix directory mode is 0666 (no traversal).
            // Apply explicit owner rwx only to this app-owned export directory.
            constexpr auto directoryPermissions=SSystem::SFileOpener::permissionRUSR|
                SSystem::SFileOpener::permissionWUSR|SSystem::SFileOpener::permissionXUSR;
            if(SSystem::SFile::CreateFullDirectory(directory,directoryPermissions))return push(eslErrGeneral);
            const auto exportPath=directory.ToCharArray();struct stat exportDirectory{};
            if(::lstat(exportPath.GetConstArray(),&exportDirectory)||!S_ISDIR(exportDirectory.st_mode))return push(eslErrGeneral);
            if(::access(exportPath.GetConstArray(),R_OK|W_OK|X_OK)!=0) {
                // Repair only an existing screenshots directory left by the
                // earlier default mode, never a path picked by the user.
                if(::chmod(exportPath.GetConstArray(),(exportDirectory.st_mode&0777)|S_IRWXU)||
                   ::access(exportPath.GetConstArray(),R_OK|W_OK|X_OK))return push(eslErrGeneral);
            }
        }
        SSystem::SString selected;
        const auto dialog=save?SSystem::BrowseSaveFileDialog(selected,caption,directory,allFiles?nullptr:filterPointers.data(),0,window?window->GetWindow():nullptr):
            SSystem::BrowseOpenFileDialog(selected,caption,directory,allFiles?nullptr:filterPointers.data(),0,window?window->GetWindow():nullptr);
        if(dialog!=SSystem::msgboxResultOk||selected.IsEmpty())return push(eslErrAbort);
        if(save) {
            const auto path=selected.ToCharArray();struct stat existing{};
            if(::stat(path.GetConstArray(),&existing)==0) {
                if(!S_ISREG(existing.st_mode))return push(eslErrInvalidParam);
                SSystem::SString question=L"文件已存在，是否覆盖？\n";question+=selected;
                if(SSystem::MessageBox(question,caption,SSystem::msgboxStyleYesNo,window?window->GetWindow():nullptr)!=SSystem::msgboxResultYes)
                    return push(eslErrAbort);
            } else if(errno!=ENOENT)return push(eslErrGeneral);
        }
        // The path reference is changed only after an actual positive result.
        output->m_varStr=static_cast<const wchar_t*>(selected);
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy file dialog accepted: save=%d path=%s",save,selected.ToCharArray().GetConstArray());
        return push(eslErrSuccess);
    }
    if(named(L"GetWindowsProductID")) {
        if((error=context.VerifyArgumentCount(args,1)))return error;
        // GLS3 returns an empty string when it cannot query its Windows system
        // volume. Android has no such volume; do not invent a Windows identity.
        return context.PushObject(new ECSString(L""));
    }
    if(named(L"MakeMD5Digest")||named(L"CalcCRC32")||named(L"CheckSum32")) {
        if((error=context.VerifyArgumentCount(args,2)))return error;
        SakuraCL::MD5Context digest;SakuraCL::CRC32Context crc;uint32_t checksum=0;size_t position=0;
        error=StreamArgument(context,args,[&](const uint8_t *data,size_t count){
            if(named(L"MakeMD5Digest"))digest.Stream(data,count);
            else if(named(L"CalcCRC32"))crc.Stream(data,count);
            else for(size_t i=0;i<count;++i,++position) {
                const auto word=static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(data[i])))<<((position&3)*8);
                const bool carry=word>~checksum;checksum+=word;if(carry)++checksum;
            }
        });
        if(error)return error;
        if(named(L"MakeMD5Digest")) {digest.Flush();SSystem::SString text;digest.GetMD5DigestHex(text);text.MakeUpper();return context.PushObject(new ECSString(EWideString(text)));}
        return push(named(L"CalcCRC32")?crc.GetCRC32():checksum);
    }
    if(named(L"FilterEnvironmentPath")) {
        if((error=context.VerifyArgumentCount(args,2)))return error;
        EWideString path;if((error=context.GetArgumentAsStr(path,args,1,L"")))return error;
        if(auto *env=context.GetEnvironment())path=env->FilterFilePath(path);
        return context.PushObject(new ECSString(path));
    }
    if(named(L"GetEnvironmentVariable")) {
        if((error=context.VerifyArgumentCount(args,2)))return error;
        auto *hash=ESLTypeCast<ECSHash>(context.GetArgumentObjectAs(args,1,L"Hash"));
        if(!hash)return ESLErrorMsg("Setup.GetEnvironmentVariable requires a Hash");
        std::vector<std::string> entries;
        if(!study::platform::ReadEnvironmentVariables(entries))return eslErrGeneral;
        for(const auto& entry:entries) {
            const char *equal=std::strchr(entry.c_str(),'=');if(!equal)continue;
            const SSystem::SString line(entry.c_str());const auto split=line.Find(L'=');
            auto key=line.Left(split);key.MakeUpper();const auto value=line.Middle(split+1);
            hash->m_varArray.SetAs(ECSWideString(static_cast<const wchar_t *>(key)),new ECSString(EWideString(value)));
        }
        return push(0);
    }
    if(named(L"GetAppDataDirectory")) {
        if((error=context.VerifyArgumentCount(args,1)))return error;
        SSystem::SString directory;
        const auto result=SSystem::SFile::GetDefaultDirectory(directory,SSystem::SFile::DefaultDirectory::AndroidLocalFiles);
        if(result)return ESLErrorMsg("Application data directory is unavailable");
        return context.PushObject(new ECSString(EWideString(directory)));
    }
    if(named(L"ShellExecute")) {
        if((error=context.VerifyArgumentCount(args,3,4)))return error;
        EWideString verb,path,parameters;
        if((error=context.GetArgumentAsStr(verb,args,1,L"")) || (error=context.GetArgumentAsStr(path,args,2,L"")) ||
           (error=context.GetArgumentAsStr(parameters,args,3,L"")))return error;
        if((!verb.IsEmpty()&&verb!=L"open")||!parameters.IsEmpty())return push(eslErrNotSupported);
        const auto result=SSystem::OpenShellFile(path);
        return push(result?eslErrGeneral:eslErrSuccess);
    }
    if(named(L"GetDiskVolumeName")||named(L"GetDiskSerialNumber")||named(L"GetDiskFreeSpace")) {
        if((error=context.VerifyArgumentCount(args,named(L"GetDiskFreeSpace")?5:3)))return error;
        // Windows drive-letter volume queries have no Android counterpart. The
        // script receives a real failure and its output reference stays untouched.
        return push(eslErrNotSupported);
    }
    study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Setup method unavailable on this platform: %s",
        SSystem::SString(m_pwszFuncName[index]).ToCharArray().GetConstArray());
    return ESLErrorMsg("Requested Windows Setup operation is unavailable on this platform");
}
