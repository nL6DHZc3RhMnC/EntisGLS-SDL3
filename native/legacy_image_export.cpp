#include "legacy_compat/gls.h"
#include "legacy_image_export.h"
#include <sakuragl/sgl2d/sgl_image_encoder.h>
#include <sakuragl/sgl2d/sgl_paint_context.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <algorithm>
#include <cstring>
#include <cwchar>
#include <mutex>
#include <new>
#include <vector>
#if defined(STUDYSTEADY_PLATFORM_SDL3)
#include "platform/sdl/sdk_image_codec.h"
#endif

namespace {
using namespace SakuraGL;
struct GraphicsLock {GraphicsLock(){SSystem::Lock();}~GraphicsLock(){SSystem::Unlock();}};
struct ImageBufferLock {
    SGLImageObject &image;int flags;SGLImageInfo info;uint8_t *pixels;
    ImageBufferLock(SGLImageObject &value,int access):image(value),flags(access),pixels(value.LockBuffer(info,access)){}
    ~ImageBufferLock(){if(pixels)image.UnlockBuffer(flags);}
    SGLError Unlock(){if(!pixels)return sglErrFailed;pixels=nullptr;return image.UnlockBuffer(flags);}
};
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
struct ExportJNIThread {
    JavaVM *vm=JNI::g_JavaVM;JNIEnv *env=nullptr;bool attached=false;
    ExportJNIThread() {
        if(!vm)return;
        const auto state=vm->GetEnv(reinterpret_cast<void**>(&env),JNI_VERSION_1_6);
        if(state==JNI_EDETACHED)attached=vm->AttachCurrentThread(&env,nullptr)==JNI_OK;
        if(state!=JNI_OK&&!attached)env=nullptr;
    }
    ~ExportJNIThread(){if(attached)vm->DetachCurrentThread();}
};
#endif
class ExportStream final:public SSystem::SFileInterface {
public:
    explicit ExportStream(EMemoryFile &file):file_(file){}
    SSystem::SFileInterface *Duplicate() const override{return nullptr;}
    size_t Read(void *data,size_t count) override{return file_.Read(data,count);}
    size_t Write(const void *data,size_t count) override {
        const auto result=file_.Write(data,count);failed|=result!=count;return result;
    }
    bool IsSeekable() const override{return true;}
    int64_t GetLength() const override{return file_.GetLargeLength();}
    int64_t GetPosition() const override{return file_.GetLargePosition();}
    int64_t Seek(int64_t offset,SeekOrigin origin) override{return file_.SeekLarge(offset,static_cast<ESLFileObject::SeekOrigin>(origin));}
    SSystem::SError SetEndOfFile() override {
        const auto error=file_.SetEndOfFile();failed|=error!=0;return error?SSystem::errFailed:SSystem::errSuccess;
    }
    bool failed=false;
private:
    EMemoryFile &file_;
};
ESLError Snapshot(SGLImageObject &source,SGLImage &copy) {
    GraphicsLock lock;
    SGLImageInfo info;
    if(source.GetImageInfo(info)||!info.width||!info.height||
        uint64_t(info.width)*info.height>0x4000000u)return eslErrInvalidParam;
    const auto colorSpace=info.format&formatImageTypeMask;
    if((colorSpace!=formatImageRGB&&colorSpace!=formatImageBGR&&colorSpace!=formatImageGray)||
        (info.format&(formatImageFlagSideBySide|formatImageFlagPalette|formatImageFlagClipping))||
        !info.depth||info.depth>32||(info.depth%8))return eslErrNotSupported;
    const size_t frames=source.GetFrameCount();
    if(!frames||frames>0x10000||uint64_t(info.width)*info.height*frames>0x4000000u)return eslErrInvalidParam;
    // ReadFrameBuffer only copies bytes in this SDK: it does not honor a new
    // destination color/alpha tag. First read the source's actual layout, then
    // convert color/depth and premultiplication in separate checked stages.
    // Keeping alpha mode unchanged in stage one also handles ABGR correctly:
    // the SDK's BGR converter swaps channels but does not divide by alpha.
    const uint32_t outputFormat=formatImageARGB|formatImageFlagNoProductOfAlpha;
    if(copy.CreateImage(info.width,info.height,outputFormat,32,SGLImageObject::bufferOnMemory,frames,source.GetTotalTime()))
        return eslErrGeneral;
    SGLImage nativeFrame,rgbaFrame;
    const uint32_t rgbaFormat=formatImageRGB|(info.format&(formatImageFlagAlpha|formatImageFlagNoProductOfAlpha));
    if(nativeFrame.CreateImage(info.width,info.height,info.format,info.depth,SGLImageObject::bufferOnMemory)||
        rgbaFrame.CreateImage(info.width,info.height,rgbaFormat,32,SGLImageObject::bufferOnMemory))return eslErrGeneral;
    for(size_t frame=0;frame<frames;++frame) {
        if(copy.SelectFrame(frame))return eslErrGeneral;
        ImageBufferLock native(nativeFrame,SGLImageObject::lockRead|SGLImageObject::lockWrite);
        ImageBufferLock rgba(rgbaFrame,SGLImageObject::lockRead|SGLImageObject::lockWrite);
        ImageBufferLock buffer(copy,SGLImageObject::lockWrite);
        if(!native.pixels||!rgba.pixels||!buffer.pixels)return eslErrGeneral;
        if(source.ReadFrameBuffer(native.info,native.pixels,frame))return eslErrGeneral;
        SGLImageBuffer from=native.info,intermediate=rgba.info,to=buffer.info;
        from.ptrBuffer=native.pixels;intermediate.ptrBuffer=rgba.pixels;to.ptrBuffer=buffer.pixels;
        if(sglConvertImageBuffer(intermediate,from)||sglConvertImageBuffer(to,intermediate))return eslErrGeneral;
        const auto nativeUnlocked=native.Unlock(),rgbaUnlocked=rgba.Unlock(),unlocked=buffer.Unlock();
        if(nativeUnlocked||rgbaUnlocked||unlocked)return eslErrGeneral;
    }
    const size_t selected=source.GetSelectedFrame();
    if(copy.SelectFrame(std::min(selected,frames-1)))return eslErrGeneral;
    const size_t sequence=source.GetSequenceLength();
    if(sequence>0x100000)return eslErrInvalidParam;
    if(sequence) {
        std::vector<uint32_t> indices(sequence);
        const size_t count=source.GetSequenceTable(indices.data(),indices.size());
        if(count>indices.size())return eslErrGeneral;
        for(size_t i=0;i<count;++i)if(indices[i]>=frames)return eslErrInvalidParam;
        copy.SetSequenceTable(indices.data(),count);
    }
    copy.SetImageOrigin(info.ptOrigin.x,info.ptOrigin.y);
    return eslErrSuccess;
}
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
bool JNIError(JNIEnv *env){if(!env->ExceptionCheck())return false;env->ExceptionClear();return true;}
class AndroidBitmapEncoder final:public SGLImageEncoderInterface {
public:
    bool IsMatchableFileExtension(const wchar_t *extension,SSystem::SString &mime) override {
        if(!SSystem::SString::CompareNoCase(extension,L"png")){mime=L"image/png";return true;}
        if(!SSystem::SString::CompareNoCase(extension,L"jpg")||!SSystem::SString::CompareNoCase(extension,L"jpeg")) {
            mime=L"image/jpeg";return true;
        }
        return false;
    }
    bool IsMatchableMIMEType(const wchar_t *mime) override {
        return mime&&(!SSystem::SString::CompareNoCase(mime,L"image/png")||!SSystem::SString::CompareNoCase(mime,L"image/jpeg"));
    }
    SGLError WriteImage(SSystem::SFileInterface &file,SGLImageObject &image,
        const wchar_t *mime,const Options *options) override {
        if(!IsMatchableMIMEType(mime))return sglErrFailed;
        JNIEnv *env=JNI::GetJNIEnv();if(!env)return sglErrFailed;
        if(env->PushLocalFrame(32)<0){JNIError(env);return sglErrFailed;}
        struct Frame {JNIEnv *env;~Frame(){env->PopLocalFrame(nullptr);}} frame{env};
        ImageBufferLock buffer(image,SGLImageObject::lockRead);
        if(!buffer.pixels)return sglErrFailed;
        const auto info=buffer.info;
        std::vector<jint> colors(size_t(info.width)*info.height);
        for(size_t y=0;y<info.height;++y)
            std::memcpy(colors.data()+y*info.width,buffer.pixels+int64_t(y)*info.pitchLine,info.width*sizeof(jint));
        const auto unlocked=buffer.Unlock();
        if(unlocked)return sglErrFailed;
        jclass bitmap=env->FindClass("android/graphics/Bitmap");
        if(JNIError(env)||!bitmap)return sglErrFailed;
        jclass config=env->FindClass("android/graphics/Bitmap$Config");
        if(JNIError(env)||!config)return sglErrFailed;
        jclass formats=env->FindClass("android/graphics/Bitmap$CompressFormat");
        if(JNIError(env)||!formats)return sglErrFailed;
        jclass stream=env->FindClass("java/io/ByteArrayOutputStream");
        if(JNIError(env)||!stream)return sglErrFailed;
        jfieldID argbField=env->GetStaticFieldID(config,"ARGB_8888","Landroid/graphics/Bitmap$Config;");
        if(JNIError(env)||!argbField)return sglErrFailed;
        const char *format=!SSystem::SString::CompareNoCase(mime,L"image/png")?"PNG":"JPEG";
        jfieldID formatField=env->GetStaticFieldID(formats,format,"Landroid/graphics/Bitmap$CompressFormat;");
        if(JNIError(env)||!formatField)return sglErrFailed;
        jmethodID create=env->GetStaticMethodID(bitmap,"createBitmap","([IIILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        if(JNIError(env)||!create)return sglErrFailed;
        jmethodID compress=env->GetMethodID(bitmap,"compress","(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z");
        if(JNIError(env)||!compress)return sglErrFailed;
        jmethodID recycle=env->GetMethodID(bitmap,"recycle","()V");
        if(JNIError(env)||!recycle)return sglErrFailed;
        jmethodID construct=env->GetMethodID(stream,"<init>","()V");
        if(JNIError(env)||!construct)return sglErrFailed;
        jmethodID bytes=env->GetMethodID(stream,"toByteArray","()[B");
        if(JNIError(env)||!bytes)return sglErrFailed;
        jintArray array=env->NewIntArray(colors.size());
        if(JNIError(env)||!array)return sglErrFailed;
        env->SetIntArrayRegion(array,0,colors.size(),colors.data());
        if(JNIError(env))return sglErrFailed;
        jobject argb=env->GetStaticObjectField(config,argbField);
        if(JNIError(env)||!argb)return sglErrFailed;
        jobject compression=env->GetStaticObjectField(formats,formatField);
        if(JNIError(env)||!compression)return sglErrFailed;
        jobject encodedBitmap=env->CallStaticObjectMethod(bitmap,create,array,jint(info.width),jint(info.height),argb);
        if(JNIError(env)||!encodedBitmap)return sglErrFailed;
        struct BitmapOwner {
            JNIEnv *env;jobject object;jmethodID recycle;
            bool Recycle(){if(!object)return true;auto value=object;object=nullptr;env->CallVoidMethod(value,recycle);return !JNIError(env);}
            ~BitmapOwner(){Recycle();}
        } bitmapOwner{env,encodedBitmap,recycle};
        jobject output=env->NewObject(stream,construct);
        if(JNIError(env)||!output)return sglErrFailed;
        const int quality=options&&(options->nFlags&optionQuality)?
            int(std::min<uint32_t>(256,options->nQuality)*100+128)/256:75;
        const bool success=env->CallBooleanMethod(encodedBitmap,compress,compression,quality,output);
        if(JNIError(env)||!success)return sglErrFailed;
        if(!bitmapOwner.Recycle())return sglErrFailed;
        auto encoded=static_cast<jbyteArray>(env->CallObjectMethod(output,bytes));
        if(JNIError(env)||!encoded)return sglErrFailed;
        const jsize length=env->GetArrayLength(encoded);
        if(JNIError(env)||length<=0)return sglErrFailed;
        std::vector<jbyte> data(length);
        env->GetByteArrayRegion(encoded,0,length,data.data());
        if(JNIError(env)||file.Write(data.data(),data.size())!=data.size())return sglErrFailed;
        return sglErrSuccess;
    }
};
#endif
}

ESLError LegacyEncodeImage(SakuraGL::SGLImageObject &source,EMemoryFile &encoded,
    const wchar_t *mime,int quality) try {
    using namespace SakuraGL;
    if(!mime||!*mime)return eslErrInvalidParam;
    const bool bitmap=!SSystem::SString::CompareNoCase(mime,L"image/bmp");
    const bool erisa=!SSystem::SString::CompareNoCase(mime,L"image/x-eri")||
        !SSystem::SString::CompareNoCase(mime,L"image/x-erina")||!SSystem::SString::CompareNoCase(mime,L"image/x-erisa");
    const bool portable=!SSystem::SString::CompareNoCase(mime,L"image/png")||!SSystem::SString::CompareNoCase(mime,L"image/jpeg");
    if(!bitmap&&!erisa&&!portable)return eslErrNotSupported;
#if !defined(STUDYSTEADY_PLATFORM_SDL3)
    // The real SDK decoder used for verification also needs JNI. Keep a new
    // attachment until its images/local references have been destroyed, and
    // never detach a thread owned by Java or by the engine's thread wrapper.
    ExportJNIThread thread;
    if(!thread.env||JNIError(thread.env))return eslErrGeneral;
#endif
    SGLImage image;
    if(const auto error=Snapshot(source,image))return error;
    SGLImage bitmapImage;
    if(bitmap) {
        const auto size=image.GetImageSize();
        // NormalizeFormat retains the original pitchPixel in this SDK. A
        // 24-bit tag on four-byte pixels makes its BMP encoder write four
        // bytes per pixel while the decoder consumes three. Allocate a real
        // packed RGB24 image and perform an explicit checked conversion.
        if(bitmapImage.CreateImage(size.w,size.h,formatImageRGB,24,SGLImageObject::bufferOnMemory))return eslErrGeneral;
        ImageBufferLock from(image,SGLImageObject::lockRead),to(bitmapImage,SGLImageObject::lockWrite);
        if(!from.pixels||!to.pixels||to.info.depth!=24||to.info.pitchPixel!=3)return eslErrGeneral;
        SGLImageBuffer sourceBuffer=from.info,destinationBuffer=to.info;
        sourceBuffer.ptrBuffer=from.pixels;destinationBuffer.ptrBuffer=to.pixels;
        if(sglConvertImageBuffer(destinationBuffer,sourceBuffer))return eslErrGeneral;
        const auto sourceUnlocked=from.Unlock(),destinationUnlocked=to.Unlock();
        if(sourceUnlocked||destinationUnlocked)return eslErrGeneral;
    }
    if(encoded.Create(0x10000))return eslErrGeneral;
    ExportStream file(encoded);
    SGLImageEncoderInterface::Options options{};
    if(quality>=0){options.nFlags=SGLImageEncoderInterface::optionQuality;options.nQuality=(std::min(quality,100)*256+50)/100;}
    SGLError result;
    if(bitmap){SGLWindowsBitmapEncoder encoder;result=encoder.WriteImage(file,bitmapImage,mime,nullptr);}
    else if(erisa){SGLERImageEncoder encoder;result=encoder.WriteImage(file,image,mime,nullptr);}
    else {
#if defined(STUDYSTEADY_PLATFORM_SDL3)
        SGLSDLImageEncoder encoder;
#else
        AndroidBitmapEncoder encoder;
#endif
        result=encoder.WriteImage(file,image,mime,quality>=0?&options:nullptr);
    }
    if(result||file.failed||!encoded.GetLength())return eslErrGeneral;
    encoded.SeekLarge(0,ESLFileObject::FromBegin);
    SGLImage decoded;
    SGLError decodeResult;
#if defined(STUDYSTEADY_PLATFORM_SDL3)
    if(portable){SGLSDLImageDecoder decoder;decodeResult=decoder.ReadImage(decoded,file);}
    else
#endif
        decodeResult=decoded.ReadImage(&file,erisa?L"image/x-eri":mime);
    if(decodeResult||decoded.GetImageSize().w!=image.GetImageSize().w||
        decoded.GetImageSize().h!=image.GetImageSize().h)return eslErrGeneral;
    SGLPalette last;
    if(decoded.GetPixelRGBA(last,decoded.GetImageSize().w-1,decoded.GetImageSize().h-1))return eslErrGeneral;
    encoded.SeekLarge(encoded.GetLargeLength(),ESLFileObject::FromBegin);
    return eslErrSuccess;
} catch(const std::bad_alloc &) {
    return eslErrGeneral;
}
ESLError LegacyCreateThumbnail(SakuraGL::SGLImageObject &source,SakuraGL::SGLImage &thumbnail,
    int width,int height) try {
    using namespace SakuraGL;
    if(width<0||height<0)return eslErrInvalidParam;
    SGLImage snapshot;
    if(const auto error=Snapshot(source,snapshot))return error;
    const auto size=snapshot.GetImageSize();
    const bool original=!width||!height;
    if(original){width=size.w;height=size.h;}
    if(!width||!height||uint64_t(width)*height>0x4000000u)return eslErrInvalidParam;
    if(thumbnail.CreateImage(width,height,formatImageRGB,32,SGLImageObject::bufferOnMemory))return eslErrGeneral;
    SGLPaintContext paint;
    if(paint.AttachTargetImage(&thumbnail,nullptr)||paint.FillClearTarget(0xff000000))return eslErrGeneral;
    SGLAffine transform;
    SGLPaintParam parameters;
    parameters.nFlags=paintSmoothStretch;
    parameters.SetAffine(transform,0,0,0,0,original?1.0:(width+0.5)/size.w,original?1.0:(height+0.5)/size.h);
    const auto drawn=paint.DrawImage(parameters,&snapshot);
    const auto finished=paint.Finish();const auto detached=paint.DetachTargetImage();
    return drawn||finished||detached?eslErrGeneral:eslErrSuccess;
} catch(const std::bad_alloc &) {
    return eslErrGeneral;
}
