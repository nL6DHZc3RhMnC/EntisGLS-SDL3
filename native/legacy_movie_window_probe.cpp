#include "legacy_compat/gls.h"
#include "legacy_movie.h"
#include "legacy_window.h"
#include "legacy_movie_window_probe.h"
#include <sakuragl/sgl_opengl_context.h>
#include "platform/log.h"
#include <atomic>
#include <chrono>
#include <memory>

namespace {
bool Check(bool ok,const char* message) {
    study::platform::LogPrint(ok?study::platform::LogPriority::Info:study::platform::LogPriority::Error,"StudySteady",
        "Opening window probe %s: %s",ok?"PASS":"FAIL",message);
    return ok;
}
bool Invoke(ECSContext& context,ECSWindow& window,const wchar_t* name,
            ECSObjArray<ECSObject>& args) {
    int index=-1;auto error=window.GetFunction(context,index,name);
    if(!error)error=window.CallFunction(context,index,args);
    std::unique_ptr<ECSObject> value(error?nullptr:context.PopObject());
    INT64 status=eslErrGeneral;
    if(error||!value||value->OperateInteger(status)||status) {
        study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Opening window %ls error=%s status=%lld",
            name,GetESLErrorMsg(error),static_cast<long long>(status));return false;
    }
    return true;
}
// This child runs after SGLSpriteMovie::DrawVideo. A CPU snapshot cannot satisfy
// it: require an actual current GL context on the calling rendering thread.
class FrameWitness final:public SakuraGL::SGLSprite {
    ECSMovieSprite& movie_;
public:
    mutable std::atomic<uint64_t> draws{0},differentFrames{0},lastFrame{UINT64_MAX};
    explicit FrameWitness(ECSMovieSprite& movie):movie_(movie) {}
    void Draw(SakuraGL::S3DRenderContextInterface& render,
              const Virtual3DParam* p3d,Stereo3DView view) const override {
        if(SakuraGL::SGLOpenGLContext::GetCurrentGLContext()) {
            const auto frame=movie_.GetCurrentFrame();
            if(lastFrame.exchange(frame)!=frame)++differentFrames;
            ++draws;
        }
        SGLSprite::Draw(render,p3d,view);
    }
    void Reset(){draws=0;differentFrames=0;lastFrame=UINT64_MAX;}
};
struct GlobalLock {
    GlobalLock(){SSystem::Lock();}
    ~GlobalLock(){SSystem::Unlock();}
};
void YieldMilliseconds(unsigned ms) {
    const auto count=SSystem::UnlockAll();Sleep(ms);SSystem::Relock(count);
}
}

bool CheckLegacyOpeningWindow(ECSContext& context,ECSEnvironment& environment,bool fullLength) {
    ECSWindow window;ECSMovieSprite movie;FrameWitness witness(movie);
    struct Cleanup {
        ECSWindow& window;ECSMovieSprite& movie;FrameWitness& witness;
        ~Cleanup(){
            const auto count=SSystem::UnlockAll();
            movie.StopMovie();movie.CloseMovie();
            {GlobalLock lock;movie.NativeSprite().DetachChild(&witness);
             window.NativeSprite().DetachChild(&movie.NativeSprite());}
            window.CloseDisplay();SSystem::Relock(count);
        }
    } cleanup{window,movie,witness};
    ECSObjArray<ECSObject> args;args.Add(new ECSReference(&window));
    args.Add(new ECSString(L"StudySteady opening movie regression"));
    args.Add(new ECSInteger(SakuraGL::Window::modeWindow));
    args.Add(new ECSInteger(1920));args.Add(new ECSInteger(1080));
    if(!Invoke(context,window,L"CreateDisplay",args))return false;
    {
        GlobalLock lock;
        if(!Check(!witness.CreateBuffer(1,1),"real rendering witness buffer"))return false;
        witness.SetVisible(true);
        movie.NativeSprite().ChangePriority(65535);movie.NativeSprite().SetPosition(0,0);
        movie.NativeSprite().SetVisible(true);
        movie.NativeSprite().AddChild(&witness);
        window.NativeSprite().AddChild(&movie.NativeSprite());
    }
    for(unsigned round=0;round<2;++round) {
        if(!Check(!movie.OpenMovieFile(L"opening.mei",&environment),"open actual opening.mei from game archives"))return false;
        const auto frames=movie.GetTotalFrame(),duration=movie.GetTotalTime();
        if(!Check(frames>90&&duration>5000,"real long opening metadata"))return false;
        {GlobalLock lock;witness.Reset();}
        if(!Check(!movie.SetVolume(0,0)&&!movie.PlayMovie(0x401),
            "original direct-presentation and decode flags start the real audio/video player"))return false;
        // No force-refresh calls: playback must invalidate and render its own
        // live Android window. This was absent from the earlier MEI probe.
        const bool naturalEnd=fullLength&&round==0;
        const auto started=std::chrono::steady_clock::now();
        uint64_t elapsed=0;
        if(naturalEnd) {
            uint64_t nextProgress=15000;
            for(;;) {
                YieldMilliseconds(100);
                elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now()-started).count();
                bool active;uint64_t current;
                {GlobalLock lock;active=movie.IsMoviePlaying();current=movie.GetCurrentFrame();}
                if(elapsed>=nextProgress) {
                    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
                        "Opening full progress elapsed=%llu/%llu frame=%llu/%llu GLdraws=%llu distinct=%llu playing=%d",
                        static_cast<unsigned long long>(elapsed),static_cast<unsigned long long>(duration),
                        static_cast<unsigned long long>(current),static_cast<unsigned long long>(frames),
                        static_cast<unsigned long long>(witness.draws.load()),
                        static_cast<unsigned long long>(witness.differentFrames.load()),active);
                    nextProgress+=15000;
                }
                if(!active||elapsed>=duration+10000)break;
            }
        } else {
            for(unsigned tick=0;tick<30;++tick)YieldMilliseconds(100);
            elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now()-started).count();
        }
        uint64_t frame,draws,distinct;bool playing;
        {GlobalLock lock;frame=movie.GetCurrentFrame();playing=movie.IsMoviePlaying();
         draws=witness.draws.load();distinct=witness.differentFrames.load();}
        study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady",
            "Opening window round=%u frame=%llu/%llu duration=%llu GLdraws=%llu distinct=%llu playing=%d elapsed=%llu full=%d",
            round,static_cast<unsigned long long>(frame),static_cast<unsigned long long>(frames),
            static_cast<unsigned long long>(duration),static_cast<unsigned long long>(draws),
            static_cast<unsigned long long>(distinct),playing,static_cast<unsigned long long>(elapsed),naturalEnd);
        if(naturalEnd) {
            if(!Check(!playing&&elapsed+1500>=duration&&elapsed<=duration+10100&&frame+2>=frames,
                "natural end reaches final frame near the full movie duration without an explicit stop"))return false;
            if(!Check(draws>=100&&distinct>=100&&distinct>=frames/10,
                "full movie renders hundreds of distinct decoded frames on the live GL thread"))return false;
        } else if(!Check(playing&&frame>=10&&draws>=10&&distinct>=10,
            "multiple seconds advance decoded frames and actually draw distinct frames on GL thread"))return false;
        if(!Check(!movie.StopMovie()&&!movie.IsMoviePlaying(),"stop joins actual video and audio streaming threads"))return false;
        const auto stopped=movie.GetCurrentFrame();YieldMilliseconds(120);
        if(!Check(movie.GetCurrentFrame()==stopped,"stopped frame remains stable"))return false;
        if(!Check(!movie.CloseMovie()&&!movie.GetImage()&&!movie.IsMoviePlaying(),
            "close releases decoder before a fresh archive reopen"))return false;
    }
    return Check(true,fullLength?
        "full natural opening completion with live Android GL window, then stop, close and three-second reopen":
        "real opening with live Android GL window: direct flag, frame progression, stop, close and reopen");
}
