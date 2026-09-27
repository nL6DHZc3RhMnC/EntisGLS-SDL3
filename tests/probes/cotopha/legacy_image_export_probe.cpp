#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_image_export.h"
#include "legacy_image_export_probe.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include "platform/log.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>
#include <unistd.h>

namespace {
bool Check(bool ok,const char *stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy image export probe FAIL: %s",stage);
    return ok;
}
class ExportImage final:public ECSExecutionImage {
public:
    ExportImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
class FailingExportSave final:public ECSString {
public:ESLError Save(ESLFileObject &,ECSContext &) override{return eslErrNotSupported;}
};
std::vector<uint8_t> ReadDisk(SSystem::SFileOpener &opener,const wchar_t *path) {
    std::unique_ptr<SSystem::SFileInterface> file(opener.NewOpenFile(path,SSystem::SFileOpener::modeRead));
    if(!file||file->GetLength()<0||file->GetLength()>0x1000000)return {};
    std::vector<uint8_t> bytes(file->GetLength());
    if(file->Read(bytes.data(),bytes.size())!=bytes.size())return {};
    return bytes;
}
bool HasSignature(const EMemoryFile &encoded,const wchar_t *mime) {
    const auto *data=static_cast<const uint8_t *>(encoded.GetBuffer());
    if(encoded.GetLength()<8)return false;
    if(!std::wcscmp(mime,L"image/png"))return !std::memcmp(data,"\x89PNG\r\n\x1a\n",8);
    if(!std::wcscmp(mime,L"image/jpeg"))return data[0]==0xff&&data[1]==0xd8&&data[2]==0xff;
    if(!std::wcscmp(mime,L"image/bmp"))return data[0]=='B'&&data[1]=='M'&&StudySteadyLegacyWire::Read32(data+2)==encoded.GetLength();
    return !std::memcmp(data,"Entis\x1a",6);
}
bool CheckAlphaAndThread() {
    using namespace SakuraGL;
    const uint32_t colors[]={0x00000000,0x01010000,0x11070503,0x40201008,0x80402010,0xfe7f4020,0xffc06030};
    struct Format {uint32_t format,depth;};
    const Format formats[]={
        {formatImageARGB,32},{formatImageABGR,32},
        {formatImageARGB|formatImageFlagNoProductOfAlpha,32},
        {formatImageABGR|formatImageFlagNoProductOfAlpha,32},
        {formatImageRGB,24},{formatImageBGR,24}};
    for(const auto format:formats) {
        SGLImage source;
        if(!Check(!source.CreateImage(7,1,format.format,format.depth,SGLImageObject::bufferOnMemory),"alpha/layout fixture"))return false;
        SGLImageInfo nativeInfo;auto *native=source.LockBuffer(nativeInfo,SGLImageObject::lockWrite);
        if(!Check(native!=nullptr,"alpha/layout native pixels"))return false;
        for(int x=0;x<7;++x) {
            uint32_t pixel=colors[x];
            if(!(format.format&formatImageFlagAlpha))pixel|=0xff000000;
            else if(format.format&formatImageFlagNoProductOfAlpha) {
                const unsigned alpha=pixel>>24;uint32_t straight=alpha<<24;
                for(unsigned shift:{0u,8u,16u})straight|=(alpha?
                    std::min(255u,(((pixel>>shift)&255)*255+alpha/2)/alpha):0)<<shift;
                pixel=straight;
            }
            if((format.format&formatImageColorSpaceMask)==formatImageBGR)
                pixel=(pixel&0xff00ff00)|((pixel&0xff)<<16)|((pixel>>16)&0xff);
            // Write real source bytes, independently of SDK SetPixelRGBA's
            // own BGR/alpha conversions, so the fixture cannot mask that bug.
            std::memcpy(native+x*nativeInfo.pitchPixel,&pixel,format.depth/8);
        }
        if(!Check(!source.UnlockBuffer(SGLImageObject::lockWrite),"alpha/layout source ready"))return false;
        EMemoryFile encoded;
        if(!Check(!LegacyEncodeImage(source,encoded,L"image/png",95),"PNG encodes actual source color/depth/alpha mode"))return false;
        SESLFileInterface stream(encoded.Duplicate(),true);SGLImage decoded;
        if(!Check(!decoded.ReadImage(&stream,L"image/png")&&decoded.GetImageSize().w==7&&decoded.GetImageSize().h==1,
            "reopen alpha/layout PNG"))return false;
        unsigned maximum=0;
        for(int x=0;x<7;++x) {
            SGLPalette expected,actual;expected.ui32=colors[x];
            if(!(format.format&formatImageFlagAlpha))expected.ui32|=0xff000000;
            if(!Check(!decoded.GetPixelRGBA(actual,x,0),"alpha/layout readable pixels"))return false;
            if(!Check((expected.ui32>>24)==(actual.ui32>>24),"PNG preserves exact alpha"))return false;
            // GetPixelRGBA returns the SDK's premultiplied values. Compare
            // visible channels; low-alpha unpremultiplication is quantized.
            for(unsigned shift:{0u,8u,16u})maximum=std::max(maximum,unsigned(std::abs(
                int((expected.ui32>>shift)&255)-int((actual.ui32>>shift)&255))));
        }
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy image alpha: format=0x%x depth=%u max_premultiplied_error=%u",
            format.format,format.depth,maximum);
        if(!Check(maximum<=2,"PNG preserves visible alpha colors without double multiplication or channel swap"))return false;
    }
    SGLImage source;
    if(!Check(!source.CreateImage(4,4,formatImageARGB,32,SGLImageObject::bufferOnMemory),"native worker CPU source"))return false;
    SGLPalette color;color.ui32=0xff124578;
    if(!Check(!source.FillImage(color),"native worker opaque pixels"))return false;
    ESLError encoded=eslErrGeneral;bool encodedPng=false;
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
    int before=JNI_ERR,after=JNI_ERR;
#endif
    std::thread worker([&] {
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
        JNIEnv *env=nullptr;before=JNI::g_JavaVM->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_6);
#endif
        EMemoryFile output;encoded=LegacyEncodeImage(source,output,L"image/png",-1);
        encodedPng=!encoded&&HasSignature(output,L"image/png");
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
        after=JNI::g_JavaVM->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_6);
        // Report an attachment leak as a failed probe instead of crashing ART
        // as the worker exits. This is only cleanup of the test's own thread.
        if(after==JNI_OK)JNI::g_JavaVM->DetachCurrentThread();
#endif
    });
    worker.join();
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    return Check(!encoded&&encodedPng,"native worker encodes a real PNG through the SDL image backend");
#else
    return Check(before==JNI_EDETACHED&&!encoded&&encodedPng&&after==JNI_EDETACHED,
        "native worker encodes PNG and releases only its own JNI attachment");
#endif
}
}
bool CheckLegacyImageExport(ECSEnvironment &environment) {
    using namespace SakuraGL;
    ExportImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    ECSContext *previous=ECotophaScript::GetPrimaryContext();
    struct Restore {ECSContext &context;ECSContext *previous;
        ~Restore(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} restore{context,previous};
    if(!Check(!context.InitializeContext(&image),"isolated export context"))return false;
    ECSSprite sprite;
    if(!Check(!sprite.NativeSprite().CreateBuffer(32,24,formatImageARGB,32)&&sprite.GetImage(),"actual dynamic Sprite framebuffer source"))return false;
    auto *source=sprite.GetImage();
    for(int y=0;y<24;++y)for(int x=0;x<32;++x) {
        SGLPalette pixel;pixel.ui32=0xff000000u|uint32_t(20+x*6)<<16|uint32_t(30+y*7)<<8|uint32_t(16+(x+y)*3);
        if(!Check(!source->SetPixelRGBA(x,y,pixel),"source gradient pixels"))return false;
    }
    const wchar_t *formats[]={L"image/x-eri",L"image/x-erina",L"image/x-erisa",L"image/bmp",L"image/png",L"image/jpeg"};
    for(const auto *mime:formats) {
        EMemoryFile encoded;
        const bool jpeg=!std::wcscmp(mime,L"image/jpeg");
        if(!Check(!encoded.Create(4096)&&!sprite.WriteImageFile(encoded,mime,95)&&HasSignature(encoded,mime),
            "real requested codec signature"))return false;
        std::unique_ptr<ESLFileObject> copy(encoded.Duplicate());
        SESLFileInterface stream(copy.release(),true);SGLImage decoded;
        if(!Check(!decoded.ReadImage(&stream,nullptr)&&decoded.GetImageSize().w==32&&decoded.GetImageSize().h==24,
            "actual SDK reopens encoded image dimensions"))return false;
        double totalError=0;unsigned maxError=0;
        for(int y=0;y<24;++y)for(int x=0;x<32;++x) {
            SGLPalette original,actual;
            if(!Check(!source->GetPixelRGBA(original,x,y)&&!decoded.GetPixelRGBA(actual,x,y),"encoded image pixels readable"))return false;
            for(unsigned shift:{0u,8u,16u}) {
                const unsigned error=unsigned(std::abs(int((original.ui32>>shift)&255)-int((actual.ui32>>shift)&255)));
                maxError=std::max(maxError,error);totalError+=error;
            }
        }
        const double mean=totalError/(32*24*3);
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy image codec: %ls bytes=%lu max_rgb_error=%u mean_rgb_error=%.4f",
            mime,encoded.GetLength(),maxError,mean);
        if(!Check(jpeg?(maxError<=25&&mean<=5):maxError==0,jpeg?"JPEG q95 bounded RGB loss":"lossless codec preserves exact opaque RGB"))return false;
    }
    if(!CheckAlphaAndThread())return false;
    ECSFile memory;
    if(!Check(!memory.CreateMemoryFile(4096)&&!memory.SaveThumbnailImage(&sprite,16,12),"BMP thumbnail smooth resize"))return false;
    const auto thumbnailLength=memory.GetFileLength();
    const auto *thumbnailBytes=static_cast<const uint8_t*>(memory.GetBuffer(0,14,false));
    if(!Check(thumbnailBytes&&thumbnailBytes[0]=='B'&&thumbnailBytes[1]=='M'&&
        StudySteadyLegacyWire::Read32(thumbnailBytes+2)==thumbnailLength,"BMP file-size prefix boundary"))return false;
    ECSString payload(L"thumbnail metadata"),title(L"image title");
    if(!Check(!memory.SaveObject(payload,&title,context)&&memory.GetFileLength()>thumbnailLength,"EMC body appended after bitmap prefix"))return false;
    memory.Seek(0,ESLFileObject::FromBegin);
    ECSResource thumbnail;
    if(!Check(!thumbnail.ReadImageFile(*memory.GetFileInterface())&&thumbnail.GetImage()->GetImageSize().w==16&&
        thumbnail.GetImage()->GetImageSize().h==12,"Resource.LoadImage reads true thumbnail from BMP plus EMC"))return false;
    memory.Seek(0,ESLFileObject::FromBegin);ECSObject *loaded=nullptr;
    if(!Check(!memory.LoadObject(loaded,context),"File.LoadObject skips BMP prefix and reads EMC"))return false;
    std::unique_ptr<ECSObject> loadedObject(loaded);
    auto *loadedString=ESLTypeCast<ECSString>(loaded);
    if(!Check(loadedString&&loadedString->m_varStr==payload.m_varStr,"thumbnail save contains complete script object"))return false;

    auto *opener=environment.GetWritableFileOpener();if(!opener)return false;
    const std::wstring directory=L".__image_probe_"+std::to_wstring(::getpid())+L"_"+std::to_wstring(timeGetTime());
    const std::wstring path=directory+L"/slot.dat",screenshot=directory+L"/screenshot.png";
    if(!Check(!opener->CreateSubDirectory(directory.c_str(),SSystem::SFileOpener::permissionRWXU),
        "isolated export directory"))return false;
    struct Cleanup {SSystem::SFileOpener &opener;const std::wstring &directory,&slot,&screen;
        ~Cleanup(){opener.RemoveSubFile(slot.c_str());opener.RemoveSubFile(screen.c_str());opener.RemoveSubDirectory(directory.c_str());}
    } cleanup{*opener,directory,path,screenshot};
    const std::vector<uint8_t> old={'o','l','d',' ','s','a','v','e'};
    {
        std::unique_ptr<SSystem::SFileInterface> seed(opener->NewOpenFile(path.c_str(),SSystem::SFileOpener::modeCreateFile));
        if(!Check(seed&&seed->Write(old.data(),old.size())==old.size(),"old slot before thumbnail"))return false;
    }
    {
        ECSFile slot;FailingExportSave failure;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context)&&!slot.SaveThumbnailImage(&sprite,16,12)&&
            ReadDisk(*opener,path.c_str())==old,"thumbnail prefix stays private until context save"))return false;
        if(!Check(slot.SaveObject(failure,nullptr,context)!=eslErrSuccess&&!slot.Close()&&ReadDisk(*opener,path.c_str())==old,
            "failure after successful thumbnail still preserves old slot"))return false;
    }
    {
        ECSFile slot;
        if(!Check(!slot.Open(path.c_str(),ESLFileObject::modeCreate,&context)&&!slot.SaveThumbnailImage(&sprite,16,12)&&
            !slot.SaveObject(payload,nullptr,context)&&!slot.Close(),"thumbnail and context commit together"))return false;
        const auto bytes=ReadDisk(*opener,path.c_str());
        if(!Check(bytes.size()>54&&bytes[0]=='B'&&bytes[1]=='M'&&
            StudySteadyLegacyWire::Read32(bytes.data()+2)<bytes.size(),"atomic disk record includes BMP prefix and body"))return false;
    }
    {
        int index=-1;
        if(!Check(!sprite.GetFunction(context,index,L"SaveImage"),"inherited native SaveImage binding"))return false;
        ECSObjArray<ECSObject> args;args.Add(new ECSReference(&sprite));args.Add(new ECSString(screenshot.c_str()));
        args.Add(new ECSString(L"image/png"));args.Add(new ECSInteger(90));
        if(!Check(!sprite.CallFunction(context,index,args),"native SaveImage writes requested PNG"))return false;
        std::unique_ptr<ECSObject> status(context.PopObject());INT64 error=-1;
        if(!Check(status&&!status->OperateInteger(error)&&!error,"native SaveImage result"))return false;
        const auto bytes=ReadDisk(*opener,screenshot.c_str());
        if(!Check(bytes.size()>8&&!std::memcmp(bytes.data(),"\x89PNG\r\n\x1a\n",8),"saved screenshot is actually PNG"))return false;
        if(!Check(sprite.SaveImageFile(screenshot.c_str(),L"image/not-supported",0,&context)!=eslErrSuccess&&
            ReadDisk(*opener,screenshot.c_str())==bytes,"unsupported codec does not truncate old screenshot"))return false;
    }
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady",
        "Legacy image export probe PASS: dynamic framebuffer, real ERI/ERINA/ERISA/BMP/PNG/JPEG, decoded pixels/dimensions, BMP plus EMC and atomic thumbnail failure");
    return true;
}
