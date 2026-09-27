#include "legacy_compat/gls.h"
#include "legacy_tone_filter.h"
#include "legacy_save_io.h"
#include "legacy_tone_math.h"
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <algorithm>
#include <cstring>
#include <cwchar>
#include "platform/log.h"

IMPLEMENT_CLASS_INFO(ECSToneFilter,ECSObject)
namespace {
struct ToneLock {ToneLock(){SSystem::Lock();}~ToneLock(){SSystem::Unlock();}};
ESLError MakeTone(std::array<uint8_t,256>& table,int value,int type) {
    if(type==0)SakuraGL::sglMakeBrightnessToneFilter(table.data(),value);
    else if(type==2)SakuraGL::sglMakeAdditionalToneFilter(table.data(),value);
    else if(type==1)LegacyToneInversion(table.data(),value);
    else if(!value){for(unsigned i=0;i<256;++i)table[i]=uint8_t(i);}
    else return ESLErrorMsg("Unknown legacy tone operation");
    return eslErrSuccess;
}
}
LegacyToneState::LegacyToneState(){for(auto& channel:bgra)for(unsigned i=0;i<256;++i)channel[i]=uint8_t(i);}
bool HasLegacyToneEffect(const LegacyToneState& state) {
    if(state.flags&14)return true;
    for(const auto& channel:state.bgra)for(unsigned i=0;i<256;++i)if(channel[i]!=i)return true;
    return false;
}
ECSToneFilter::ECSToneFilter():state_(std::make_shared<LegacyToneState>()){m_vtType=csvtObject;}
const wchar_t* ECSToneFilter::GetTypeName()const{return L"ToneFilter";}
ECSObject* ECSToneFilter::GetTypeOf(const wchar_t* name){return name&&!std::wcscmp(name,L"ToneFilter")?this:ECSObject::GetTypeOf(name);}
void ECSToneFilter::Commit(const LegacyToneState& state){const auto revision=state_->revision;*state_=state;state_->revision=revision+1;}
ECSObject* ECSToneFilter::Duplicate(){ToneLock lock;auto* copy=new ECSToneFilter;copy->Commit(*state_);return copy;}
ESLError ECSToneFilter::Move(ECSContext& context,ECSObject* value){
    ToneLock lock;auto* other=ESLTypeCast<ECSToneFilter>(ECSObject::GetEntity(value));
    if(!other)return ESLErrorMsg("ToneFilter assignment requires ToneFilter");
    if(other!=this)Commit(*other->state_);context.delete_CSObject(value);return eslErrSuccess;
}
ESLError ECSToneFilter::UnaryOperate(ECSContext&,CSUnaryOperatorType){return ESLErrorMsg("ToneFilter has no unary operator");}
ESLError ECSToneFilter::Operate(ECSContext&,CSOperatorType,ECSObject*){return ESLErrorMsg("ToneFilter has no arithmetic operator");}
ESLError ECSToneFilter::Compare(ECSContext&,int&,CSCompareType,ECSObject&){return ESLErrorMsg("ToneFilter has no value comparison");}
ESLError ECSToneFilter::SetGeneralTone(const std::array<int,8>& values,uint32_t flags){
    ToneLock lock;if(flags&~15u)return ESLErrorMsg("Unknown legacy tone flags");LegacyToneState next;
    constexpr unsigned order[]={2,1,0,3};
    for(unsigned c=0;c<4;++c)if(const auto error=MakeTone(next.bgra[order[c]],values[c*2],values[c*2+1]))return error;
    next.flags=flags;if(flags&1)for(auto& channel:next.bgra)channel[0]=0;
    Commit(next);return eslErrSuccess;
}
ESLError ECSToneFilter::MorphingFilter(const ECSToneFilter& first,const ECSToneFilter& second,uint32_t degree){
    ToneLock lock;degree=std::min(degree,256u);LegacyToneState next;next.flags=first.state_->flags;
    // GLS3 deliberately truncates both weighted terms before adding them.
    for(unsigned c=0;c<4;++c)for(unsigned i=0;i<256;++i)
        next.bgra[c][i]=uint8_t((unsigned(first.state_->bgra[c][i])*(256-degree)>>8)+(unsigned(second.state_->bgra[c][i])*degree>>8));
    Commit(next);return eslErrSuccess;
}
ESLError ECSToneFilter::Save(ESLFileObject& file,ECSContext&){
    ToneLock lock;LegacySave::Writer out{file};out.U32(state_->flags);out.Bytes(state_->bgra.data(),1024);return out.error;
}
ESLError ECSToneFilter::Load(ESLFileObject& file,ECSContext&){
    ToneLock lock;LegacySave::Reader in{file};LegacyToneState next;next.flags=in.U32();in.Bytes(next.bgra.data(),1024);
    if(in.error)return in.error;if(next.flags&~15u)return eslErrInvalidParam;Commit(next);return eslErrSuccess;
}
ESLError ECSToneFilter::LoadFilterFile(const wchar_t* path,ECSContext& context){
    std::unique_ptr<ESLFileObject> file(context.OpenFileOnScript(path));return file?ReadFilterFile(*file):eslErrGeneral;
}
ESLError ECSToneFilter::ReadFilterFile(ESLFileObject& file){
    ToneLock lock;LegacyToneState next=*state_;next.flags=0;
    uint8_t header[64];if(file.Read(header,64)!=64||std::memcmp(header,"Entis\x1a\0\0",8))return eslErrInvalidParam;
    const auto length=file.GetLength();
    while(file.GetPosition()<length){
        uint8_t record[16];if(file.Read(record,16)!=16)return eslErrGeneral;
        const auto bytes=StudySteadyLegacyWire::Read64(record+8);
        const auto position=file.GetPosition();
        if(position>length||bytes>length-position)return eslErrGeneral;
        int channel=-1;const char* tags[]={"blue    ","green   ","red     ","alpha   "};
        for(int i=0;i<4;++i)if(!std::memcmp(record,tags[i],8))channel=i;
        if(channel>=0){if(bytes!=256||file.Read(next.bgra[channel].data(),256)!=256)return eslErrGeneral;}
        else if(!std::memcmp(record,"info    ",8)){
            uint8_t flags[4];if(bytes!=4||file.Read(flags,4)!=4)return eslErrGeneral;next.flags=StudySteadyLegacyWire::Read32(flags);
        }else if(file.SeekLarge(position+bytes,ESLFileObject::FromBegin)!=position+bytes)return eslErrGeneral;
    }
    if(next.flags&~15u)return eslErrInvalidParam;Commit(next);return eslErrSuccess;
}
ESLError ECSToneFilter::GetFunction(ECSContext&,int& index,const wchar_t* name){
    const wchar_t* names[]={L"LoadFilterFile",L"SetGeneralTone",L"MorphingFilter"};
    for(index=0;index<3;++index)if(name&&!std::wcscmp(name,names[index]))return eslErrSuccess;
    return ESLErrorMsg("Unknown ToneFilter method");
}
ESLError ECSToneFilter::CallFunction(ECSContext& context,int index,ECSObjArray<ECSObject>& args){
    const auto error=context.VerifyArgumentCount(args,index==0?2:index==1?9:4,index==1?10:index==0?2:4);
    if(error)return error;ESLError status=eslErrSuccess;
    if(index==0){ECSWideString path;if(const auto e=context.GetArgumentAsStr(path,args,1,nullptr))return e;status=LoadFilterFile(path,context);}
    else if(index==1){std::array<int,8> values;int flags=0;for(int i=0;i<8;++i)if(const auto e=context.GetArgumentAsInt(values[i],args,i+1,0))return e;
        if(const auto e=context.GetArgumentAsInt(flags,args,9,0))return e;status=SetGeneralTone(values,uint32_t(flags));}
    else if(index==2){auto* first=ESLTypeCast<ECSToneFilter>(context.GetArgumentObjectAs(args,1,L"ToneFilter"));
        auto* second=ESLTypeCast<ECSToneFilter>(context.GetArgumentObjectAs(args,2,L"ToneFilter"));int degree=0;
        if(!first||!second)return ESLErrorMsg("MorphingFilter requires two ToneFilter sources");
        if(const auto e=context.GetArgumentAsInt(degree,args,3,0))return e;status=MorphingFilter(*first,*second,uint32_t(degree));}
    else return ESLErrorMsg("Invalid ToneFilter method index");
    return context.PushObject(new ECSInteger(status));
}
ESLError ApplyLegacyToneImage(SakuraGL::SGLImageObject& image,const LegacyToneState& state,bool sourceHasAlpha){
    ToneLock lock;if(!HasLegacyToneEffect(state))return eslErrSuccess;
    SakuraGL::SGLImageBuffer buffer;auto* bytes=image.LockBuffer(buffer);
    if(!bytes)return eslErrGeneral;
    struct Unlock {SakuraGL::SGLImageObject& image;~Unlock(){image.UnlockBuffer();}} unlock{image};
    if(buffer.depth!=32||(buffer.format&255)!=SakuraGL::formatImageRGB)return eslErrInvalidParam;
    for(uint32_t y=0;y<buffer.height;++y)for(uint32_t x=0;x<buffer.width;++x){
        auto* pixel=bytes+int64_t(y)*buffer.pitchLine+int64_t(x)*buffer.pitchPixel;
        if(state.flags&8){const auto gray=((unsigned(pixel[0])+3u*pixel[1]+2u*pixel[2])*0x2aaa+0x8000)>>16;pixel[0]=pixel[1]=pixel[2]=uint8_t(gray);}
        else if(state.flags&4)LegacyToneRGBToYUV(pixel[0],pixel[1],pixel[2]);
        const bool alpha=sourceHasAlpha&&(buffer.format&SakuraGL::formatImageFlagAlpha);
        for(int c=0;c<(alpha?4:3);++c)pixel[c]=state.bgra[c][pixel[c]];
        if(state.flags&4)LegacyToneYUVToRGB(pixel[0],pixel[1],pixel[2]);
        if((state.flags&2)&&alpha)for(int c=0;c<3;++c)pixel[c]=uint8_t(unsigned(pixel[c])*(unsigned(pixel[3])+1)>>8);
    }
    return eslErrSuccess;
}


bool CheckLegacyTone(){
    auto check=[](bool good,const char* stage){study::platform::LogPrint(good?study::platform::LogPriority::Debug:study::platform::LogPriority::Error,
        "StudySteady","Legacy Tone probe %s: %s",good?"OK":"FAIL",stage);return good;};
    ECSToneFilter tone,copy,morph;ECSContext context;SakuraGL::SGLImage image;
    if(!check(!HasLegacyToneEffect(*tone.ToneState()),"default object is an actual identity LUT"))return false;
    if(!check(!image.CreateImage(2,1,SakuraGL::formatImageARGB,32),"real tone test image"))return false;
    image.SetPixelRGBA(0,0,SakuraGL::SGLPalette(0xff123456));
    if(!check(!ApplyLegacyToneImage(image,*tone.ToneState()),"identity image operation"))return false;
    SakuraGL::SGLPalette pixel;image.GetPixelRGBA(pixel,0,0);
    if(!check(pixel.ui32==0xff123456,"identity leaves actual pixel unchanged"))return false;
    if(!check(!tone.SetGeneralTone({256,1,0,0,0,0,0,0}),"old inversion table rather than modern multiply"))return false;
    if(!check(!ApplyLegacyToneImage(image,*tone.ToneState()),"inversion applies to real pixel"))return false;
    image.GetPixelRGBA(pixel,0,0);if(!check(pixel.ui32==0xffed3456,"red-channel inversion preserves other channels"))return false;
    EMemoryFile file;if(!check(!file.Create(1028)&&!tone.Save(file,context)&&file.GetLength()==1028,"exact 1028-byte Win32 tone wire"))return false;
    file.SeekLarge(0,ESLFileObject::FromBegin);
    if(!check(!copy.Load(file,context)&&copy.ToneState()->bgra==tone.ToneState()->bgra,"LUT round trip"))return false;
    if(!check(!tone.SetGeneralTone({0,0,0,0,0,0,0,0},8),"legacy gray flag"))return false;
    image.SetPixelRGBA(0,0,SakuraGL::SGLPalette(0xffff0000));ApplyLegacyToneImage(image,*tone.ToneState());image.GetPixelRGBA(pixel,0,0);
    if(!check(pixel.ui32==0xff555555,"GLS3 gray red becomes 85 rather than modern 74"))return false;
    if(!check(!tone.SetGeneralTone({0,0,0,0,0,0,0,0},4),"legacy YUV flag"))return false;
    image.SetPixelRGBA(0,0,SakuraGL::SGLPalette(0xff123456));
    uint8_t b=0x56,g=0x34,r=0x12;LegacyToneRGBToYUV(b,g,r);LegacyToneYUVToRGB(b,g,r);
    ApplyLegacyToneImage(image,*tone.ToneState());image.GetPixelRGBA(pixel,0,0);
    if(!check(pixel.ui32==(0xff000000u|(uint32_t(r)<<16)|(uint32_t(g)<<8)|b),"old SSE YUV round trip and carry rounding"))return false;
    ECSToneFilter identity;if(!check(!morph.MorphingFilter(identity,identity,128)&&morph.ToneState()->bgra[0][255]==254,
        "morph truncates each weighted LUT term separately"))return false;
    auto previous=copy.ToneState()->bgra;EMemoryFile shortened;shortened.Open(file.GetBuffer(),1027);
    if(!check(copy.Load(shortened,context)!=eslErrSuccess&&copy.ToneState()->bgra==previous,"truncated tone leaves previous tables intact"))return false;
    EMemoryFile records;records.Create(2048);uint8_t header[64]={};std::memcpy(header,"Entis\x1a\0\0",8);records.Write(header,64);
    uint8_t record[16]={};std::memcpy(record,"red     ",8);StudySteadyLegacyWire::Write64(record+8,256);records.Write(record,16);
    std::array<uint8_t,256> red;for(unsigned i=0;i<256;++i)red[i]=uint8_t(255-i);records.Write(red.data(),256);records.SeekLarge(0,ESLFileObject::FromBegin);
    if(!check(!identity.ReadFilterFile(records)&&identity.ToneState()->bgra[2]==red&&identity.ToneState()->bgra[0][73]==73,
        "EMC tone channel loading preserves omitted tables"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy Tone probe PASS: identity, inversion, exact gray/YUV, LUT morph, fixed wire and checked EMC/truncated loading");return true;
}
