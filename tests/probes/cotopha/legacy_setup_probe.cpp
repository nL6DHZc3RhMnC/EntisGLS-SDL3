#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_setup.h"
#include "legacy_setup_probe.h"
#include "platform/log.h"
#include <memory>

namespace {
bool Check(bool ok,const char *stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Setup probe FAIL: %s",stage);
    return ok;
}
std::unique_ptr<ECSObject> Invoke(ECSContext &context,ECSSetup &setup,const wchar_t *method,ECSObject *argument) {
    int index;
    if(setup.GetFunction(context,index,method))return nullptr;
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&setup));if(argument)args.Add(argument);
    if(setup.CallFunction(context,index,args))return nullptr;
    return std::unique_ptr<ECSObject>(context.PopObject());
}
}
bool CheckLegacySetup() {
    if(!Check(ECSSetup::MakeMD5Digest("",0)==L"D41D8CD98F00B204E9800998ECF8427E"&&
       ECSSetup::MakeMD5Digest("abc",3)==L"900150983CD24FB0D6963F7D28E17F72","MD5 known vectors"))return false;
    if(!Check(ECSSetup::CalcCRC32("123456789",9)==0xcbf43926u,"CRC32 known vector"))return false;
    const unsigned char carry[]={255,255,255,255,255};
    if(!Check(ECSSetup::CheckSum32(carry,sizeof(carry))==0xfefeff02u,"Win32 signed-char checksum carry"))return false;
    ECSContext context;ECSSetup setup;
    auto digest=Invoke(context,setup,L"MakeMD5Digest",new ECSString(L"日本語"));
    auto *text=ESLTypeCast<ECSString>(digest.get());
    if(!Check(text&&text->m_varStr==L"387D3D4780C18A32C9C90437E4B50160","script String digest uses CP932 bytes"))return false;
    ECSFile file;
    if(!Check(!file.CreateMemoryFile(8),"open memory file"))return false;
    file.GetFileInterface()->Write("prefixabc",9);file.GetFileInterface()->Seek(6,ESLFileObject::FromBegin);
    digest=Invoke(context,setup,L"MakeMD5Digest",new ECSReference(&file));text=ESLTypeCast<ECSString>(digest.get());
    if(!Check(text&&text->m_varStr==L"900150983CD24FB0D6963F7D28E17F72"&&file.GetFileInterface()->GetPosition()==9,
        "script File digest consumes from current position"))return false;
    digest=Invoke(context,setup,L"GetWindowsProductID",nullptr);text=ESLTypeCast<ECSString>(digest.get());
    if(!Check(text&&text->m_varStr.IsEmpty(),"absent Windows system volume returns no fabricated ID"))return false;
    if(!Check(ECSSetup::MessageBoxStyleToAndroid(0x14)==2&&ECSSetup::MessageBoxResultToWindows(2)==6&&
       ECSSetup::MessageBoxStyleToAndroid(2)==5&&ECSSetup::MessageBoxResultToWindows(5)==3,
       "Win32 Setup dialog button/result mapping"))return false;
    int index;setup.GetFunction(context,index,L"RebootWindows");ECSObjArray<ECSObject> args;args.Add(new ECSReference(&setup));
    if(!Check(setup.CallFunction(context,index,args)!=eslErrSuccess,"Windows-only method fails explicitly"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy Setup probe PASS: MD5/CRC32/checksum, CP932/File digests, absent Windows identity, dialog mapping, unsupported boundary");
    return true;
}
