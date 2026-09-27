#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_audio_player.h"
#include "runtime/cotopha_port/legacy_checked_state_file.h"
#include "runtime/cotopha_port/legacy_serialization.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
#include "platform/log.h"

IMPLEMENT_CLASS_INFO(ECSAudioPlayer,ECSResource)
namespace {
constexpr int methodBase=4096;
constexpr uint64_t maxPCMBytes=256ull*1024*1024;
bool SameFormat(const SakuraGL::SGLSoundFormat &a,const SakuraGL::SGLSoundFormat &b) {
    return a.format==b.format&&a.frequency==b.frequency&&a.channels==b.channels&&a.bitsPerSample==b.bitsPerSample;
}
}
const wchar_t *ECSAudioPlayer::GetTypeName() const { return L"AudioPlayer"; }
ECSObject *ECSAudioPlayer::GetTypeOf(const wchar_t *name) {
    return !EWideString::Compare(name,L"AudioPlayer")?this:ECSResource::GetTypeOf(name);
}
ECSObject *ECSAudioPlayer::Duplicate() {
    auto *copy=new ECSAudioPlayer;
    copy->m_image=m_image;copy->m_imageStream=m_imageStream;
    { std::lock_guard<std::mutex> lock(m_volumeMutex);
      if(m_sound)copy->m_sound=std::make_shared<SakuraGL::SGLAudioPlayer>(m_sound->ClonePlayer(),true); }
    copy->m_wstrFileName=m_wstrFileName;copy->m_volume[0]=m_volume[0].load();copy->m_volume[1]=m_volume[1].load();copy->m_nPlayType=m_nPlayType.load();
    copy->markFile_=markFile_;copy->replacementFiles_=replacementFiles_;copy->replacementMarks_=replacementMarks_;
    copy->m_nThreshold=m_nThreshold;copy->m_nStartPos=m_nStartPos;copy->m_nEndPos=m_nEndPos;
    copy->m_nRewindPos=m_nRewindPos;copy->m_nRepeatPlaying=m_nRepeatPlaying;
    return copy;
}
void ECSAudioPlayer::ClearRecipe(){markFile_=L"";replacementFiles_.clear();replacementMarks_.clear();}
ESLError ECSAudioPlayer::Release(){ClearRecipe();return ECSResource::Release();}
ESLError ECSAudioPlayer::Move(ECSContext& context,ECSObject* object) {
    auto* source=ESLTypeCast<ECSAudioPlayer>(ECSObject::GetEntity(object));
    const EWideString mark=source?source->markFile_:EWideString();
    const auto files=source?source->replacementFiles_:std::vector<EWideString>();
    const auto marks=source?source->replacementMarks_:std::vector<Mark>();
    const auto error=ECSResource::Move(context,object);
    if(!error){markFile_=mark;replacementFiles_=files;replacementMarks_=marks;}
    return error;
}
ESLError ECSAudioPlayer::Save(ESLFileObject& file,ECSContext& context) try {
    if(replacementFiles_.size()>65536||replacementMarks_.size()>65536||
       replacementFiles_.size()!=replacementMarks_.size())return eslErrInvalidParam;
    for(const auto& mark:replacementMarks_)if(mark.position>UINT32_MAX||mark.length>UINT32_MAX)return eslErrNotSupported;
    StudySteadyLegacy::CheckedStateFile checked(file);
    if(const auto error=checked.Result(ECSResource::Save(checked,context)))return error;
    using namespace StudySteadyLegacyWire;
    if(!WriteWideString(checked,markFile_))return eslErrGeneral;
    auto write32=[&](uint32_t n){uint8_t bytes[4];Write32(bytes,n);checked.Write(bytes,4);};
    write32(replacementFiles_.size());
    for(const auto& path:replacementFiles_)if(!WriteWideString(checked,path))return eslErrGeneral;
    write32(replacementMarks_.size());
    for(const auto& mark:replacementMarks_){write32(mark.position);write32(mark.length);}
    return checked.Result();
} catch(const std::bad_alloc&) {return eslErrGeneral;}

ESLError ECSAudioPlayer::Load(ESLFileObject& file,ECSContext& context) {
    struct Incomplete {ECSAudioPlayer& player;bool done=false;~Incomplete(){if(!done)player.Release();}} incomplete{*this};
    try {
        ClearRecipe();StudySteadyLegacy::CheckedStateFile checked(file);
        if(const auto error=checked.Result(ECSResource::Load(checked,context)))return error;
        using namespace StudySteadyLegacyWire;
        EWideString markFile;
        if(!ReadWideString(checked,markFile))return eslErrGeneral;
        auto read32=[&](){uint8_t bytes[4];checked.Read(bytes,4);return Read32(bytes);};
        const auto fileCount=read32();
        if(checked.failed||fileCount>65536)return eslErrInvalidParam;
        std::vector<EWideString> paths(fileCount);
        for(auto& path:paths)if(!ReadWideString(checked,path)||path.IsEmpty())return eslErrGeneral;
        const auto markCount=read32();
        const auto position=checked.GetLargePosition(),length=checked.GetLargeLength();
        if(checked.failed||markCount>65536||position>length||markCount>(length-position)/8)return eslErrGeneral;
        std::vector<Mark> marks(markCount);
        for(auto& mark:marks){mark.position=read32();mark.length=read32();}
        if(checked.failed||fileCount!=markCount||(!fileCount&&!markFile.IsEmpty()))return eslErrInvalidParam;
        if(fileCount) {
            if(m_wstrFileName.IsEmpty()||!m_sound||m_refAttachSound.m_pRef)
                return ESLErrorMsg("Saved AudioPlayer replacement has no original audio filename");
            // Resource.Load has reopened the unmodified source and staged its
            // controls. Rebuild PCM now, then retain those controls for Commit.
            const int type=m_nPlayType.load();
            const auto threshold=m_nThreshold,start=m_nStartPos,end=m_nEndPos,
                rewind=m_nRewindPos,repeat=m_nRepeatPlaying,kind=m_restoreResourceKind;
            const auto error=markFile.IsEmpty()?ReplaceMarkPortionArray(context,marks,paths):ReplaceMarkPortion(context,markFile,paths);
            if(error)return error;
            m_nPlayType=type;m_nThreshold=threshold;m_nStartPos=start;m_nEndPos=end;
            m_nRewindPos=rewind;m_nRepeatPlaying=repeat;m_restoreResourceKind=kind;
            m_restorePlayback=true;m_resourceStateCommitted=false;
        }
        incomplete.done=true;return eslErrSuccess;
    } catch(const std::bad_alloc&) {return eslErrGeneral;}
}
ESLError ECSAudioPlayer::GetFunction(ECSContext &context,int &index,const wchar_t *name) {
    if(!EWideString::Compare(name,L"ReplaceMarkPortion")){index=methodBase;return eslErrSuccess;}
    if(!EWideString::Compare(name,L"ReplaceMarkPortionArray")){index=methodBase+1;return eslErrSuccess;}
    return ECSResource::GetFunction(context,index,name);
}
ESLError ECSAudioPlayer::Decode(SakuraGL::SGLAudioPlayerInterface &player,PCM &pcm) {
    if(player.IsPlaying()||player.IsPaused())return eslErrPending;
    const auto position=player.GetPosition();
    auto *stream=player.GetAudioStream();if(!stream)return eslErrNotSupported;
    struct Release {SakuraGL::SGLAudioPlayerInterface &player;SakuraGL::SGLAudioInputStream *stream;uint64_t position;
        ~Release(){stream->SeekAudio(position);player.ReleaseAudioStream(stream);}} release{player,stream,position};
    if(stream->GetAudioFormat(pcm.format))return eslErrGeneral;
    const auto samples=stream->GetAudioLength();
    if(samples<0||!pcm.format.channels||pcm.format.channels>8||!pcm.format.frequency||
       !pcm.format.bitsPerSample||pcm.format.bitsPerSample>64||pcm.format.bitsPerSample%8)return eslErrNotSupported;
    const uint64_t bytesPerFrame=uint64_t(pcm.format.bitsPerSample/8)*pcm.format.channels;
    if(uint64_t(samples)>maxPCMBytes/bytesPerFrame)return eslErrNotSupported;
    if(stream->SeekAudio(0))return eslErrGeneral;
    pcm.data.resize(uint64_t(samples)*bytesPerFrame);
    auto *wrapper=ESLTypeCast<SakuraGL::SGLAudioPlayer>(&player);
    auto *reader=ESLTypeCast<SakuraGL::SGLAudioBufferReader>(wrapper?wrapper->GetPlayer():&player);
    // The supplied SDK's dynamic ReadAudio does not advance its destination
    // between decoder chunks. One frame per call avoids that boundary bug;
    // static PCM reads safely use larger blocks.
    const uint64_t batch=reader&&reader->GetStaticBufferSize()?16384:1;
    uint64_t done=0;
    while(done<uint64_t(samples)) {
        const auto count=std::min<uint64_t>(batch,uint64_t(samples)-done);
        const auto got=stream->ReadAudio(pcm.data.data()+done*bytesPerFrame,count);
        if(!got||got>count)return eslErrGeneral;done+=got;
    }
    return eslErrSuccess;
}
ESLError ECSAudioPlayer::Compose(PCM &output,const PCM &base,const std::vector<Mark> &marks,const std::vector<PCM> &clips) {
    if(marks.size()!=clips.size())return eslErrInvalidParam;
    const uint64_t frame=base.format.SamplesToBytes(1);
    if(!frame||base.data.size()%frame||base.data.size()>maxPCMBytes)return eslErrInvalidParam;
    const uint64_t samples=base.data.size()/frame;
    uint64_t position=0,total=base.data.size();
    for(size_t i=0;i<marks.size();++i) {
        const auto &mark=marks[i];const auto &clip=clips[i];
        if(!SameFormat(base.format,clip.format)||clip.data.size()%frame)return eslErrNotSupported;
        if(mark.position<position||mark.position>samples||mark.length>samples-mark.position)return eslErrInvalidParam;
        total-=mark.length*frame;if(clip.data.size()>maxPCMBytes-total)return eslErrNotSupported;
        total+=clip.data.size();position=mark.position+mark.length;
    }
    PCM result;result.format=base.format;result.data.reserve(total);position=0;
    for(size_t i=0;i<marks.size();++i) {
        const auto &mark=marks[i];
        result.data.insert(result.data.end(),base.data.begin()+position*frame,base.data.begin()+mark.position*frame);
        result.data.insert(result.data.end(),clips[i].data.begin(),clips[i].data.end());
        position=mark.position+mark.length;
    }
    result.data.insert(result.data.end(),base.data.begin()+position*frame,base.data.end());
    output=std::move(result);return eslErrSuccess;
}
ESLError ECSAudioPlayer::LoadPCM(const PCM &pcm) {
    const auto &format=pcm.format;
    if(pcm.data.size()>maxPCMBytes||!format.channels||format.channels>8||!format.frequency||
       !format.bitsPerSample||format.bitsPerSample>64||format.bitsPerSample%8||
       (format.format!=SakuraGL::formatSoundLinearPCM&&format.format!=SakuraGL::formatSoundIEEEFloat))return eslErrNotSupported;
    const uint64_t alignment=format.SamplesToBytes(1);
    const uint64_t byteRate=alignment*format.frequency;
    if(!alignment||pcm.data.size()%alignment||byteRate>UINT32_MAX)return eslErrInvalidParam;
    std::unique_ptr<EMemoryFile> file(new EMemoryFile);
    const size_t padding=pcm.data.size()&1;
    if(file->Create(pcm.data.size()+44+padding))return eslErrGeneral;
    uint8_t header[44]{};
    const auto u16=[&](size_t at,uint16_t value){header[at]=value;header[at+1]=value>>8;};
    const auto u32=[&](size_t at,uint32_t value){for(int i=0;i<4;++i)header[at+i]=value>>(i*8);};
    std::memcpy(header,"RIFF",4);u32(4,pcm.data.size()+36+padding);std::memcpy(header+8,"WAVEfmt ",8);
    u32(16,16);u16(20,format.format==SakuraGL::formatSoundIEEEFloat?3:1);u16(22,format.channels);
    u32(24,format.frequency);u32(28,byteRate);u16(32,alignment);u16(34,format.bitsPerSample);
    std::memcpy(header+36,"data",4);u32(40,pcm.data.size());
    if(file->Write(header,sizeof(header))!=sizeof(header)||file->Write(pcm.data.data(),pcm.data.size())!=pcm.data.size())return eslErrGeneral;
    if(padding){const uint8_t zero=0;if(file->Write(&zero,1)!=1)return eslErrGeneral;}
    file->Seek(0,ESLFileObject::FromBegin);
    auto player=std::make_shared<SakuraGL::SGLAudioPlayer>();
    auto *stream=new SESLFileInterface(file.release(),true);
    const auto result=player->Create(stream,true,SakuraGL::SGLAudioPlayerInterface::modeOpenAuto);
    if(result)return eslErrGeneral;
    CancelVolumeEnvelope();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound=std::move(player); }
    // The composed PCM no longer corresponds to the original MIO filename.
    ClearRecipe();
    m_wstrFileName=L"";
    m_refAttachSound.SetReference(nullptr,nullptr);
    m_restorePlayback=false;
    return ApplyVolume();
}
ESLError ECSAudioPlayer::ReplaceMarkPortion(ECSContext &context,const wchar_t *path,const std::vector<EWideString> &files) {
    std::unique_ptr<ESLFileObject> file(context.OpenFileOnScript(path));if(!file)return eslErrGeneral;
    const auto length=file->GetLargeLength();if(length>4*1024*1024)return eslErrInvalidParam;
    EStreamBuffer buffer;if(file->Read(buffer.PutBuffer(length),length)!=length)return eslErrGeneral;buffer.Flush(length);
    EDescription xml;auto error=xml.ReadDescription(buffer,EDescription::dftXML,EDescription::ceUTF8);if(error)return error;
    auto *root=xml.GetContentTagAs(0,L"marks");if(!root)return eslErrInvalidParam;
    std::vector<Mark> marks;
    for(int i=0;i<root->GetContentTagCount();++i) {
        auto *tag=root->GetContentTagAt(i);if(!tag||tag->Tag()!=L"mark")continue;
        const auto position=tag->GetAttrInteger(L"pos",-1),duration=tag->GetAttrInteger(L"length",-1);
        if(position<0||duration<0)return eslErrInvalidParam;
        marks.push_back({uint64_t(position),uint64_t(duration)});
    }
    error=ReplaceMarkPortionArray(context,marks,files);
    if(!error&&!replacementFiles_.empty())markFile_=path;
    return error;
}
ESLError ECSAudioPlayer::ReplaceMarkPortionArray(ECSContext &context,const std::vector<Mark> &marks,const std::vector<EWideString> &files) {
    if(!m_sound||marks.size()!=files.size())return eslErrInvalidParam;
    if(marks.empty())return eslErrSuccess;
    if(m_sound->IsPlaying()||m_sound->IsPaused())return eslErrPending;
    PCM base;auto error=Decode(*m_sound,base);if(error)return error;
    std::vector<PCM> clips(files.size());
    for(size_t i=0;i<files.size();++i) {
        SakuraGL::SGLAudioPlayer clip;
        if(clip.Open(files[i],SakuraGL::SGLAudioPlayerInterface::modeOpenAuto,context.GetEnvironment()))return eslErrGeneral;
        error=Decode(clip,clips[i]);if(error)return error;
    }
    PCM output;error=Compose(output,base,marks,clips);if(error)return error;
    const EWideString source=m_wstrFileName;
    const bool restorable=!source.IsEmpty()&&replacementFiles_.empty();
    error=LoadPCM(output);if(error)return error;
    if(restorable) {
        // The complete AudioPlayer record restores this base then replays the
        // recipe. Resource alone still rejects anonymous generated audio.
        m_wstrFileName=source;replacementFiles_=files;replacementMarks_=marks;
    }
    // Original wire has one recipe. Sequential edits to already-composed PCM
    // remain playable but no longer claim that the base + last edit reproduces
    // them; anonymous-media Save rejects that otherwise lossy representation.
    return eslErrSuccess;
}
ESLError ECSAudioPlayer::CallFunction(ECSContext &context,int index,ECSObjArray<ECSObject> &args) {
    if(index<methodBase)return ECSResource::CallFunction(context,index,args);
    if(index!=methodBase&&index!=methodBase+1)return ESLErrorMsg("Unknown AudioPlayer method");
    auto error=context.VerifyArgumentCount(args,3);if(error)return error;
    auto *array=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,2,L"Array"));
    if(!array)return ESLErrorMsg("AudioPlayer replacement requires a file-name array");
    std::vector<EWideString> files;
    for(unsigned i=0;i<array->m_varArray.GetSize();++i) {
        auto *item=ECSObject::GetEntity(array->m_varArray.GetAt(i));EWideString name;
        if(!item||item->OperateString(name))return eslErrInvalidParam;files.push_back(name);
    }
    if(index==methodBase) {
        EWideString path;if((error=context.GetArgumentAsStr(path,args,1,L"")))return error;
        error=ReplaceMarkPortion(context,path,files);
    } else {
        auto *positions=ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args,1,L"Array"));
        if(!positions||positions->m_varArray.GetSize()!=files.size()*2)return eslErrInvalidParam;
        std::vector<Mark> marks;
        for(size_t i=0;i<files.size();++i) {
            auto *start=ECSObject::GetEntity(positions->m_varArray.GetAt(i*2));
            auto *length=ECSObject::GetEntity(positions->m_varArray.GetAt(i*2+1));INT64 p,n;
            if(!start||!length||start->OperateInteger(p)||length->OperateInteger(n)||p<0||n<0)return eslErrInvalidParam;
            marks.push_back({uint64_t(p),uint64_t(n)});
        }
        error=ReplaceMarkPortionArray(context,marks,files);
    }
    return context.PushObject(new ECSInteger(error));
}
