#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_audio_player.h"
#include "platform/log.h"
#include <cstring>
#include <memory>

namespace {
bool Check(bool ok,const char *stage) {
    if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy AudioPlayer probe FAIL: %s",stage);
    return ok;
}
ECSAudioPlayer::PCM Samples(std::initializer_list<int16_t> samples) {
    ECSAudioPlayer::PCM pcm;pcm.format.frequency=22050;pcm.format.channels=1;pcm.format.bitsPerSample=16;
    for(auto sample:samples){pcm.data.push_back(sample&255);pcm.data.push_back(uint16_t(sample)>>8);}
    return pcm;
}
class StreamingImage final:public ECSExecutionImage {
public:
    StreamingImage() {
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
bool StreamingBGM(ECSEnvironment& environment) {
    StreamingImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} scope{context,previous};
    if(!Check(!context.InitializeContext(&image),"initialize isolated streaming context"))return false;
    ECSResource bgm;
    // The actual game opens lower-case bgm05.mio from the NOA whose entry is
    // BGM05.mio. Its 17,930,404-byte PCM exceeds the SDK's 4 MiB static limit.
    for(unsigned pass=0;pass<2;++pass) {
        if(!Check(!bgm.LoadSoundFile(L"bgm05.mio",unsigned(-1),&context),"open actual BGM05 through Resource filename/NOA path"))return false;
        auto* wrapper=ESLTypeCast<SakuraGL::SGLAudioPlayer>(bgm.GetSound());
        auto* reader=wrapper?ESLTypeCast<SakuraGL::SGLAudioBufferReader>(wrapper->GetPlayer()):nullptr;
        if(!Check(reader&&reader->GetStaticBufferSize()==0&&bgm.GetSound()->GetSampleFrequency()==44100&&
            bgm.GetSound()->GetTotalLength()==4482601&&bgm.GetSound()->GetPosition()==0,
            "real BGM uses streaming decoder and each open starts at sample zero"))return false;
        if(!Check(!bgm.SetVolume(0,0)&&!bgm.Play(unsigned(-1),0),"game Play(-1,Music) starts metadata loop"))return false;
        uint64_t first=0,last=0;
        {
            // Allow the SDK's background streaming callbacks while waiting;
            // restore precisely the caller's global-lock nesting afterwards.
            const auto locks=SSystem::UnlockAll();
            struct Relock {decltype(locks) count;~Relock(){SSystem::Relock(count);}} relock{locks};
            for(unsigned attempt=0;attempt<100;++attempt) {
                Sleep(20);last=bgm.GetSound()->GetPosition();
                if(!first)first=last;
                if(first&&last>first)break;
            }
        }
        const bool advancing=first&&last>first&&last<4482601&&bgm.IsPlaying();
        const auto stopError=bgm.Stop();const bool stopped=!bgm.IsPlaying();
        const auto releaseError=bgm.Release();const bool closed=bgm.GetSound()==nullptr;
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
            "Legacy BGM streaming pass=%u first=%llu last=%llu advancing=%d stop=%s stopped=%d release=%s closed=%d",
            pass,static_cast<unsigned long long>(first),static_cast<unsigned long long>(last),advancing,
            GetESLErrorMsg(stopError),stopped,GetESLErrorMsg(releaseError),closed);
        if(!Check(advancing&&!stopError&&stopped&&!releaseError&&closed,
            "streaming playback advances and Stop/Release closes it before reopening"))return false;
    }
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy BGM streaming probe PASS: actual BGM05 NOA, dynamic MIO decode, muted advance, stop and fresh reopen");
    return true;
}
bool SavedNickname(ECSEnvironment& environment) {
    // The game's main script mounts voice.noa dynamically. Standalone tests
    // only load cotopha.xml (which lists voice2.noa), so reproduce that real
    // opener operation for this fixture and remove only our owned mount.
    constexpr const wchar_t* voiceMountID=L"legacy-audio-nickname-probe-voice";
    struct VoiceMount {
        ECSEnvironment& environment;const wchar_t* id;bool mounted=false;
        ~VoiceMount(){if(mounted)environment.RemoveFileOpener(id);}
    } voiceMount{environment,voiceMountID};
    {
        std::unique_ptr<ESLFileObject> available(environment.OpenFileObject("ggr99000.mio"));
        if(!available) {
            if(!Check(!environment.AddFileArchive(L"$(CURRENT)\\voice.noa","",voiceMountID),
                "mount game's dynamic voice.noa for standalone nickname fixture"))return false;
            voiceMount.mounted=true;
        }
    }
    StreamingImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope {ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} scope{context,previous};
    if(!Check(!context.InitializeContext(&image),"initialize isolated nickname state context"))return false;
    ECSAudioPlayer source,restored;
    if(!Check(!source.LoadSoundFile(L"ggr99000.mio",0x1000000,&context),"open actual opening nickname base MIO"))return false;
    ECSAudioPlayer::PCM base,clip,actual;
    SakuraGL::SGLAudioPlayer nickname;
    if(!Check(!ECSAudioPlayer::Decode(*source.GetSound(),base)&&
        !nickname.Open(L"vggrn0000a.mio",SakuraGL::SGLAudioPlayerInterface::modeOpenAuto,&environment)&&
        !ECSAudioPlayer::Decode(nickname,clip)&&base.data.size()==22050*2&&clip.data.size()==29451*2,
        "actual 44.1kHz mono nickname/base decoded samples"))return false;
    std::vector<uint8_t> expected=clip.data;expected.insert(expected.end(),base.data.begin(),base.data.end());
    const std::vector<ECSAudioPlayer::Mark> marks{{0,0}};
    const std::vector<EWideString> names{EWideString(L"vggrn0000a.mio")};
    if(!Check(!source.ReplaceMarkPortionArray(context,marks,names)&&
        !ECSAudioPlayer::Decode(*source.GetSound(),actual)&&actual.data==expected,
        "real fallback nickname insertion is exactly clip then base"))return false;
    EMemoryFile baseWire,wire;baseWire.Create(256);wire.Create(256);
    if(!Check(!source.SetVolume(0,0)&&!source.Play(unsigned(-2),ECSResource::ptfVoice)&&
        !source.Save(wire,context)&&!source.Stop(),"playing composed voice saves a real reconstruction recipe"))return false;
    if(!Check(!source.ECSResource::Save(baseWire,context),"base Resource retains original MIO source"))return false;
    auto rewind=[](EMemoryFile& file){return file.SeekLarge(0,ESLFileObject::FromBegin)==0;};
    if(!Check(rewind(wire)&&!restored.Load(wire,context)&&!restored.CommitAllReference(context)&&
        !restored.IsPlaying()&&!ECSAudioPlayer::Decode(*restored.GetSound(),actual)&&actual.data==expected,
        "load reopens base, reapplies nickname and preserves original stopped voice restore semantics"))return false;
    EMemoryFile again;again.Create(256);ECSAudioPlayer twice;
    if(!Check(!restored.Save(again,context)&&rewind(again)&&!twice.Load(again,context)&&
        !twice.CommitAllReference(context)&&!ECSAudioPlayer::Decode(*twice.GetSound(),actual)&&actual.data==expected,
        "reconstructed nickname voice saves and restores a second time without losing composition"))return false;
    std::unique_ptr<ECSObject> clone(restored.Duplicate());ECSAudioPlayer moved;
    if(!Check(clone&&!moved.Move(context,new ECSReference(clone.get())),"AudioPlayer assignment retains recipe"))return false;
    EMemoryFile copied;copied.Create(256);ECSAudioPlayer copyReload;
    if(!Check(!moved.Save(copied,context)&&rewind(copied)&&!copyReload.Load(copied,context)&&
        !copyReload.CommitAllReference(context)&&!ECSAudioPlayer::Decode(*copyReload.GetSound(),actual)&&actual.data==expected,
        "duplicated/assigned AudioPlayer recipe reconstructs exact PCM"))return false;
    EMemoryFile truncated;truncated.Create(wire.GetLength());truncated.Write(wire.GetBuffer(),wire.GetLength()-1);rewind(truncated);
    if(!Check(copyReload.Load(truncated,context)!=0&&!copyReload.GetSound(),"truncated recipe fails and releases partially restored player"))return false;
    EMemoryFile missing;missing.Create(wire.GetLength());missing.Write(wire.GetBuffer(),wire.GetLength());
    // Empty mark-file (4), one replacement (4), counted name (4), then UTF16.
    auto* bytes=static_cast<uint8_t*>(missing.GetBuffer());bytes[baseWire.GetLength()+12]='x';rewind(missing);
    if(!Check(copyReload.Load(missing,context)!=0&&!copyReload.GetSound(),"missing real replacement file fails instead of returning unmodified base audio"))return false;
    if(!Check(!restored.LoadSoundFile(L"ggr99000.mio",0x1000000,&context),"reuse player for next original voice clears nickname recipe"))return false;
    EMemoryFile next;next.Create(256);
    if(!Check(!restored.Save(next,context)&&rewind(next)&&!copyReload.Load(next,context)&&
        !copyReload.CommitAllReference(context)&&!ECSAudioPlayer::Decode(*copyReload.GetSound(),actual)&&actual.data==base.data,
        "next original voice reload does not accidentally repeat the previous nickname"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy nickname state probe PASS: base=22050 clip=29451 composed=51501 samples, original recipe wire=%lu bytes, exact PCM reload/resave and failure cleanup",wire.GetLength());
    return true;
}
}
bool CheckLegacyAudioPlayer(ECSEnvironment &environment) {
    ECSAudioPlayer player;
    std::unique_ptr<ESLFileObject> audio(environment.OpenFileObject("se517.mio"));
    if(!Check(audio&&!player.ReadSoundFile(*audio)&&player.GetSound(),"real NOA MIO inherited Resource load"))return false;
    if(!Check(!player.SetVolume(0,0)&&!player.Play(0),"explicit whole-file loop begins"))return false;
    Sleep(160);
    if(!Check(player.IsPlaying(),"loop remains active past real clip duration"))return false;
    if(!Check(!player.Play(unsigned(-2)),"legacy -2 switches loop to one-shot"))return false;
    Sleep(160);
    if(!Check(!player.IsPlaying(),"one-shot finishes through actual Android playback state"))return false;
    if(!Check(player.PlayFrom(0,100,false)==eslErrNotSupported,"unsupported one-shot end bound reports failure"))return false;
    ECSAudioPlayer::PCM decoded;
    if(!Check(!ECSAudioPlayer::Decode(*player.GetSound(),decoded)&&decoded.format.frequency==22050&&
        decoded.data.size()==decoded.format.SamplesToBytes(1504),"actual decoded PCM length"))return false;
    auto base=Samples({1,2,3,4,5}),insert=Samples({90,91}),output=Samples({});
    if(!Check(!ECSAudioPlayer::Compose(output,base,{{2,1}},{insert})&&output.data==Samples({1,2,90,91,4,5}).data,
        "marked replacement preserves prefix/suffix and sample ordering"))return false;
    if(!Check(!ECSAudioPlayer::Compose(output,base,{{0,0},{0,0}},{insert,insert})&&
        output.data==Samples({90,91,90,91,1,2,3,4,5}).data,"zero-length fallback marks insert clips in order"))return false;
    if(!Check(ECSAudioPlayer::Compose(output,base,{{4,2}},{insert})==eslErrInvalidParam,"out-of-range marks fail"))return false;
    if(!Check(!player.LoadPCM(output)&&player.GetSound()->GetTotalLength()==9,"composed PCM becomes real SDK WAV player"))return false;
    if(!Check(!ECSAudioPlayer::Decode(*player.GetSound(),decoded)&&decoded.data==output.data,"SDK redecodes composed waveform exactly"))return false;
    if(!Check(!player.SetVolume(0,0)&&!player.Play()&&!player.Stop()&&!player.IsPlaying(),"real muted playback and stopped backend state"))return false;
    if(!Check(player.GetTypeOf(L"AudioPlayer")==&player&&player.GetTypeOf(L"Resource")==&player,"native Resource inheritance"))return false;
    if(!StreamingBGM(environment)||!SavedNickname(environment))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy AudioPlayer probe PASS: NOA MIO, real PCM composition, WAV playback, native state and inheritance");
    return true;
}
