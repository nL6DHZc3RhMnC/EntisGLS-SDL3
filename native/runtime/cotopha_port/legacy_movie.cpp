#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_movie.h"
#include "runtime/cotopha_port/legacy_sprite_callbacks.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include <sakuraglx/sprite/sglx_sprite_movie.h>
#include <sakuragl/media/sgl_mei_media_player.h>
#include "platform/log.h"

IMPLEMENT_CLASS_INFO(ECSMovieSprite,ECSSprite)
namespace {
constexpr int methodBase=5120;
const wchar_t *methods[]={L"OpenMovie",L"CloseMovie",L"PlayMovie",L"StopMovie",L"IsMoviePlaying",
    L"SeekFrame",L"GetCurrentFrame",L"GetTotalFrame",L"GetTotalTime",L"GetInfo",L"GetImageInfo"};
ESLError MediaError(SakuraGL::SGLError error){return error?eslErrGeneral:eslErrSuccess;}
}
class ECSMovieSprite::MovieNative final:public LegacyCallbackSprite<SakuraGL::SGLSpriteMovie> {
    class MEIPlayer final:public SakuraGL::SGLMEIMediaPlayer {
    public:
        void SetDecodeFlags(uint32_t flags) {
            m_flagsDecode = (m_flagsDecode & ~uint32_t(0x0c00)) | (flags & 0x0c00);
        }
        SakuraGL::SGLError Pause() override {
            const auto error=SakuraGL::SGLMEIMediaPlayer::Pause();
            if(!error)m_status=statusPaused;
            return error;
        }
    };
public:
    SakuraGL::SGLMediaPlayer *Player() const {return m_player;}
    SakuraGL::SGLMEIMediaPlayer *MEI() const {
        auto *player=Player();return player?ESLTypeCast<SakuraGL::SGLMEIMediaPlayer>(player->GetPlayer()):nullptr;
    }
    void ConfigureDecode(uint32_t flags) {if(auto *player=MEI())static_cast<MEIPlayer*>(player)->SetDecodeFlags(flags);}
    ESLError Open(const wchar_t *path,ECSEnvironment *environment) {
        CloseMovieFile();
        auto *player=new SakuraGL::SGLMediaPlayer(new MEIPlayer,true);
        const auto status=player->Open(path,0,environment);
        if(status){delete player;return eslErrGeneral;}
        Lock();m_player=player;m_strMovieFile=path;m_flagEndOfDuration=false;
        player->SetNotificationListener(this);
        const auto sizeStatus=player->GetVideoSize(m_sizeMovieFrame);
        NotifyUpdate();Unlock();
        if(sizeStatus){CloseMovieFile();return eslErrGeneral;}
        return eslErrSuccess;
    }
};
ECSMovieSprite::ECSMovieSprite():ECSSprite(new MovieNative) {m_nPlayType=ptfMusic;}
ECSMovieSprite::~ECSMovieSprite(){CloseMovie();}
ECSMovieSprite::MovieNative &ECSMovieSprite::Movie() const {return *static_cast<MovieNative*>(sprite_.get());}
const wchar_t *ECSMovieSprite::GetTypeName() const{return L"MovieSprite";}
ECSObject *ECSMovieSprite::GetTypeOf(const wchar_t *name){return !EWideString::Compare(name,L"MovieSprite")?this:ECSSprite::GetTypeOf(name);}
ECSObject *ECSMovieSprite::Duplicate(){
    auto *copy=new ECSMovieSprite;
    if(copy->CopySprite(*this)){delete copy;return nullptr;}
    if(!m_wstrFileName.IsEmpty()) {
        if(copy->OpenMovieFile(m_wstrFileName,environment_)||copy->SeekFrame(GetCurrentFrame())){delete copy;return nullptr;}
    }
    copy->SetVolume(m_volume[0],m_volume[1]);return copy;
}
ESLError ECSMovieSprite::Move(ECSContext &context,ECSObject *source){
    auto *movie=ESLTypeCast<ECSMovieSprite>(ECSObject::GetEntity(source));
    if(!movie)return ESLErrorMsg("MovieSprite assignment requires MovieSprite");
    if(movie!=this){
        const auto error=CopySprite(*movie);if(error)return error;
        if(movie->m_wstrFileName.IsEmpty())CloseMovie();
        else {
            const auto open=OpenMovieFile(movie->m_wstrFileName,movie->environment_);if(open)return open;
            const auto seek=SeekFrame(movie->GetCurrentFrame());if(seek)return seek;
        }
    }
    context.delete_CSObject(source);return eslErrSuccess;
}
SakuraGL::SGLImageObject *ECSMovieSprite::GetImage() const {
    auto *mei=Movie().MEI();return mei?mei->CurrentFrame():ECSSprite::GetImage();
}
ESLError ECSMovieSprite::OpenMovieFile(const wchar_t *path,ECSEnvironment *environment){
    if(!path||!environment)return eslErrInvalidParam;
    CloseMovie();
    const auto filtered=environment->FilterFilePath(path);
    const auto error=Movie().Open(filtered,environment);if(error)return error;
    environment_=environment;m_wstrFileName=path;
    // Resource's ordinary volume and total-volume controls now reach the same
    // real media player. This wrapper borrows it; CloseMovie releases it first.
    { std::lock_guard<std::mutex> lock(m_volumeMutex);
      m_sound=std::make_shared<SakuraGL::SGLAudioPlayer>(Movie().Player(),false); }
    const auto volume=ApplyVolume();if(volume){CloseMovie();return volume;}
    return eslErrSuccess;
}
ESLError ECSMovieSprite::CloseMovie(){
    CancelVolumeEnvelope();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound.reset(); }
    const auto result=Movie().CloseMovieFile();m_wstrFileName=L"";environment_=nullptr;
    movieFlags_=0;loopStart_=0;loopEnd_=UINT32_MAX;movieRestorePending_=false;
    return MediaError(result);
}
ESLError ECSMovieSprite::Release(){CloseMovie();return ECSSprite::Release();}
ESLError ECSMovieSprite::Save(ESLFileObject &file,ECSContext &context){
    if(movieRestorePending_)return ESLErrorMsg("MovieSprite restoration has not been committed");
    const bool open=Movie().Player()!=nullptr;
    const uint32_t status=open?(IsMoviePlaying()?2:1):0;
    const uint64_t frame=open?GetCurrentFrame():UINT32_MAX;
    if(frame>UINT32_MAX||(open&&m_wstrFileName.IsEmpty()))return eslErrInvalidParam;
    const EWideString path=m_wstrFileName;
    // GLS3 Movie owns a second filename separate from its empty Resource
    // portion. Our sound wrapper only borrows this decoder for volume control;
    // it must never be saved/reopened as an independent audio Resource.
    std::shared_ptr<SakuraGL::SGLAudioPlayer> borrowed;
    if(open){std::lock_guard<std::mutex> lock(m_volumeMutex);borrowed=std::move(m_sound);m_wstrFileName=L"";}
    ESLError error;
    {
        struct RestoreBorrowed {
            ECSMovieSprite& owner;std::shared_ptr<SakuraGL::SGLAudioPlayer>& sound;const EWideString& path;bool active;
            ~RestoreBorrowed(){if(active){std::lock_guard<std::mutex> lock(owner.m_volumeMutex);owner.m_sound=std::move(sound);owner.m_wstrFileName=path;}}
        } restore{*this,borrowed,path,open};
        error=ECSSprite::Save(file,context);
    }
    const auto volume=open?ApplyVolume():eslErrSuccess;
    if(error)return error;
    if(volume)return volume;
    LegacySave::Writer out{file};
    // ECSString body: counted UTF-16 followed by cursor and locked-buffer size.
    out.String(open?path.CharPtr():L"");out.U32(0);out.U32(0);
    out.U32(status);out.U32(uint32_t(frame));out.U32(movieFlags_);
    out.U32(loopStart_);out.U32(loopEnd_);out.U32(0);
    return out.error;
}
ESLError ECSMovieSprite::Load(ESLFileObject &file,ECSContext &context){
    if(const auto error=ECSSprite::Load(file,context))return error;
    LegacySave::Reader in{file};const auto path=in.String();
    const auto stringIndex=in.U32(),lockedLength=in.U32();
    const auto status=in.U32(),frame=in.U32(),flags=in.U32(),loopStart=in.U32(),loopEnd=in.U32();in.Reserved();
    if(in.error)return in.error;
    if(stringIndex||lockedLength||status>2||(flags&~uint32_t(0x0c03))||
       (status==0&&(!path.IsEmpty()||frame!=UINT32_MAX))||
       (status!=0&&(path.IsEmpty()||frame==UINT32_MAX))||
       (loopEnd!=UINT32_MAX&&loopEnd<loopStart))return eslErrInvalidParam;
    m_wstrFileName=path;movieFlags_=flags;loopStart_=loopStart;loopEnd_=loopEnd;
    restoredStatus_=status;restoredFrame_=frame;movieRestorePending_=true;
    return eslErrSuccess;
}
ESLError ECSMovieSprite::CommitAllReference(ECSContext &context){
    if(!movieRestorePending_)return ECSSprite::CommitAllReference(context);
    const auto path=m_wstrFileName;const auto status=restoredStatus_,frame=restoredFrame_;
    const auto flags=movieFlags_,start=loopStart_,end=loopEnd_;const int type=m_nPlayType;
    // Resource reference resolution remains real, but the Movie decoder owns
    // playback. Generic Resource.PlayFrom would start its borrowed audio view
    // before the saved frame has been restored.
    m_nPlayType=ptfNothing;
    const auto base=ECSSprite::CommitAllReference(context);m_nPlayType=type;
    if(base)return base;
    ESLError error=eslErrSuccess;
    if(status) {
        error=OpenMovieFile(path,context.GetEnvironment());
        if(!error&&(frame>=GetTotalFrame()||start>=GetTotalFrame()||
            (end!=UINT32_MAX&&end>GetTotalFrame())))error=eslErrInvalidParam;
        if(!error)error=SeekFrame(frame);
        movieFlags_=flags;loopStart_=start;loopEnd_=end;m_nPlayType=type;
        if(!error)error=MediaError(Movie().SetMovieLoop((flags&2)!=0,start,end==UINT32_MAX?-1:int64_t(end)));
        if(!error&&status==2)error=PlayMovie(flags,type);
    }
    if(error){CloseMovie();m_wstrFileName=path;movieFlags_=flags;loopStart_=start;loopEnd_=end;movieRestorePending_=true;return error;}
    movieRestorePending_=false;return eslErrSuccess;
}
bool ECSMovieSprite::IsPlaying(){return Movie().Player()?IsMoviePlaying():ECSResource::IsPlaying();}
ESLError ECSMovieSprite::PlayMovie(uint32_t flags,int playType){
    if(!Movie().Player())return eslErrGeneral;
    // The modern "half filter" bits use the same deblocking operation and bit
    // values as GLS3's loop-filter controls. no-skip still lacks an SDK switch.
    if(flags&~uint32_t(0x0c03))return eslErrNotSupported;
    {const auto error=StopMovie();if(error)return error;}
    Movie().ConfigureDecode(flags);
    if(playType>=0&&playType<ptfMax)m_nPlayType=playType;
    const auto volume=ApplyVolume();if(volume)return volume;
    const auto loop=Movie().SetMovieLoop((flags&2)!=0,loopStart_,loopEnd_==UINT32_MAX?-1:int64_t(loopEnd_));if(loop)return eslErrGeneral;
    movieFlags_=flags;
    std::lock_guard<std::mutex> lock(m_volumeMutex);
    // GLS3's bit 0 selects a DirectDraw/GDI presentation optimization. Android
    // must present on the GL thread: the SDK direct path holds the global UI
    // mutex in MEI::Run, then ReleaseRenderContext -> Finish synchronously waits
    // for that thread while its OnDraw is waiting for the same mutex. Keep the
    // original flags for save/restore, but let the real MovieSprite participate
    // in normal window composition (same decoder, size, position and priority).
    return MediaError(Movie().PlayMovie(0));
}
ESLError ECSMovieSprite::StopMovie(){
    if(!Movie().Player())return eslErrSuccess;
    const bool active=Movie().Player()->IsPlaying();
    const auto result=Movie().StopMovie();
    // A naturally completed decoder thread retains statusPlayed until Stop.
    // Calling Stop even after IsPlaying becomes false resets replay state.
    return result&&active?eslErrGeneral:eslErrSuccess;
}
bool ECSMovieSprite::IsMoviePlaying() const{return Movie().IsMoviePlaying();}
ESLError ECSMovieSprite::SeekFrame(uint64_t frame){
    auto *mei=Movie().MEI();if(!mei||frame>=mei->GetAllFrameCount())return eslErrInvalidParam;
    const auto error=StopMovie();if(error)return error;
    // SDK SeekPosition mixes milliseconds and frames for MEI. Use its actual
    // frame-index API to retain the legacy SeekFrame contract exactly.
    SSystem::Lock();const auto result=mei->SeekToFrame(frame);
    if(!result)Movie().OnFrameUpdate(Movie().Player());SSystem::Unlock();
    return result?eslErrGeneral:eslErrSuccess;
}
uint64_t ECSMovieSprite::GetCurrentFrame() const {auto *mei=Movie().MEI();return mei?mei->CurrentIndex():0;}
uint64_t ECSMovieSprite::GetTotalFrame() const {auto *mei=Movie().MEI();return mei?mei->GetAllFrameCount():0;}
uint64_t ECSMovieSprite::GetTotalTime() const {auto *mei=Movie().MEI();return mei?mei->GetTotalTime():0;}
ESLError ECSMovieSprite::GetFunction(ECSContext &context,int &index,const wchar_t *name){
    for(size_t i=0;i<sizeof(methods)/sizeof(methods[0]);++i)if(!EWideString::Compare(name,methods[i])){index=methodBase+i;return eslErrSuccess;}
    return ECSSprite::GetFunction(context,index,name);
}
ESLError ECSMovieSprite::CallFunction(ECSContext &context,int index,ECSObjArray<ECSObject> &args){
    if(index<methodBase)return ECSSprite::CallFunction(context,index,args);
    index-=methodBase;if(index<0||index>=11)return ESLErrorMsg("Unknown MovieSprite method");
    const int low=index==0?2:1,high=index==0?2:index==2?3:index==5?2:1;
    auto error=context.VerifyArgumentCount(args,low,high);if(error)return error;
    const auto push=[&](int64_t value)->ESLError{return context.PushObject(new ECSInteger(value));};
    if(index==0){EWideString path;error=context.GetArgumentAsStr(path,args,1,L"");return error?error:push(OpenMovieFile(path,context.GetEnvironment()));}
    if(index==1)return push(CloseMovie());
    if(index==2){int flags,type;if((error=context.GetArgumentAsInt(flags,args,1,0))||(error=context.GetArgumentAsInt(type,args,2,-1)))return error;return push(PlayMovie(flags,type));}
    if(index==3)return push(StopMovie());
    if(index==4)return push(IsMoviePlaying()?-1:0);
    if(index==5){int frame;if((error=context.GetArgumentAsInt(frame,args,1,0)))return error;return push(frame<0?eslErrInvalidParam:SeekFrame(frame));}
    if(index==6)return push(GetCurrentFrame());if(index==7)return push(GetTotalFrame());if(index==8)return push(GetTotalTime());
    auto *frame=GetImage();if(!frame)return eslErrGeneral;
    SakuraGL::SGLImageInfo native;if(frame->GetImageInfo(native))return eslErrGeneral;
    auto *info=context.CreateUserStructure(L"ImageInfo");if(!info)return eslErrGeneral;
    info->SetMemberAsInt(L"nFormatType",native.format);info->SetMemberAsInt(L"nImageWidth",native.width);
    info->SetMemberAsInt(L"nImageHeight",native.height);info->SetMemberAsInt(L"nBitsPerPixel",native.depth);
    info->SetMemberAsInt(L"nFrameCount",GetTotalFrame());info->SetMemberAsInt(L"nResourceBytes",uint64_t(std::abs(native.pitchLine))*native.height);
    info->SetMemberAsInt(L"xHotSpot",native.ptOrigin.x);info->SetMemberAsInt(L"yHotSpot",native.ptOrigin.y);
    return context.PushObject(*info);
}
