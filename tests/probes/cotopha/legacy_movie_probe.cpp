#include "compatibility/sdk/legacy/gls.h"
#include "runtime/cotopha_port/legacy_movie.h"
#include "platform/log.h"
#include "runtime/cotopha_port/legacy_save_io.h"
#include <cstring>

namespace {
class MovieStateImage final:public ECSExecutionImage {
public:
    MovieStateImage(){
        const BYTE code[]={4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));m_exiHeader.nVersion=1;m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096;m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX;m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code));m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code)));m_dwImageSize=sizeof(code);
    }
};
bool Status(ESLError error,const char* stage){if(error)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Movie state %s: %s",stage,GetESLErrorMsg(error));return !error;}
bool Check(bool ok,const char *stage){if(!ok)study::platform::LogPrint(study::platform::LogPriority::Error,"StudySteady","Legacy Movie probe FAIL: %s",stage);return ok;}}
bool CheckLegacyMovie(ECSEnvironment &environment){
    MovieStateImage image;image.AttachCSEnvironment(&environment);ECSContext context;
    auto* previous=ECotophaScript::GetPrimaryContext();
    struct Scope{ECSContext& context;ECSContext* previous;~Scope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}} scope{context,previous};
    if(!Status(context.InitializeContext(&image),"initialize isolated state context"))return false;
    auto* parent=new ECSSprite;image.m_csgData.AddVariable(L"movieParent",parent);image.m_csgData.IndexAllMember();
    ECSMovieSprite movie;parent->NativeSprite().AddChild(&movie.NativeSprite());
    if(!Check(!movie.OpenMovieFile(L"トンネル車窓.mei",&environment),"open real MEI from NOA"))return false;
    auto *frame=movie.GetImage();SakuraGL::SGLImageInfo info;SakuraGL::SGLPalette pixel;
    if(!Check(frame&&!frame->GetImageInfo(info)&&info.width&&info.height&&
        !frame->GetPixelRGBA(pixel,0,0)&&movie.GetTotalFrame()>1&&movie.GetTotalTime()>0,"actual decoded frame and duration"))return false;
    study::platform::LogPrint(study::platform::LogPriority::Info,"StudySteady","Legacy MEI decoded: %ux%u frames=%llu milliseconds=%llu",
        info.width,info.height,(unsigned long long)movie.GetTotalFrame(),(unsigned long long)movie.GetTotalTime());
    if(!Check(!movie.SetVolume(0,0)&&!movie.PlayMovie(0x401),"native MEI playback preserves direct-presentation and no-loop-filter flags"))return false;
    const auto locks=SSystem::UnlockAll();Sleep(180);SSystem::Relock(locks);
    const auto position=movie.GetCurrentFrame();
    if(!Check(position>0&&movie.IsMoviePlaying(),"real decoder advances frames while playing"))return false;
    if(!Check(!movie.StopMovie()&&!movie.IsMoviePlaying(),"native MEI thread stops"))return false;
    if(!Check(!movie.SeekFrame(1)&&movie.GetCurrentFrame()==1,"frame seek is not millisecond seek"))return false;
    SakuraGL::SGLPalette stoppedPixel;movie.GetImage()->GetPixelRGBA(stoppedPixel,17,23);
    EMemoryFile stopped;if(!Status(stopped.Create(1024),"create stopped wire")||!Status(movie.Save(stopped,context),"save stopped real decoder"))return false;
    const auto* tail=static_cast<const uint8_t*>(stopped.GetBuffer())+stopped.GetLength()-24;
    if(!Check(StudySteadyLegacyWire::Read32(tail)==1&&StudySteadyLegacyWire::Read32(tail+4)==1&&
        StudySteadyLegacyWire::Read32(tail+8)==0x401,"old 24-byte movie tail stores stopped frame and decode flags"))return false;
    if(!Check(!movie.CloseMovie()&&!movie.IsMoviePlaying()&&!movie.GetImage(),"MEI closes and releases frame"))return false;
    ECSMovieSprite restored;stopped.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(restored.Load(stopped,context),"load stopped decoder record")||
       !Status(restored.CommitAllReference(context),"reopen NOA MEI and seek at commit"))return false;
    SakuraGL::SGLPalette decoded;
    if(!Check(!restored.IsMoviePlaying()&&restored.GetCurrentFrame()==1&&restored.GetImage()&&
       !restored.GetImage()->GetPixelRGBA(decoded,17,23)&&decoded.ui32==stoppedPixel.ui32&&
       restored.NativeSprite().GetParent()==&parent->NativeSprite(),"stopped frame pixels and persistent parent restored"))return false;
    if(!Status(restored.PlayMovie(0x403),"start real looping decoder before save"))return false;
    {const auto unlocked=SSystem::UnlockAll();Sleep(120);SSystem::Relock(unlocked);}
    EMemoryFile playing;if(!Status(playing.Create(1024),"create playing wire")||
       !Status(restored.Save(playing,context),"save playing loop without stopping original"))return false;
    tail=static_cast<const uint8_t*>(playing.GetBuffer())+playing.GetLength()-24;
    const auto savedFrame=StudySteadyLegacyWire::Read32(tail+4);
    if(!Check(restored.IsMoviePlaying()&&StudySteadyLegacyWire::Read32(tail)==2&&
       StudySteadyLegacyWire::Read32(tail+8)==0x403,"playing loop flag and actual frame snapshot"))return false;
    restored.CloseMovie();ECSMovieSprite resumed;playing.SeekLarge(0,ESLFileObject::FromBegin);
    if(!Status(resumed.Load(playing,context),"load playing movie record")||
       !Status(resumed.CommitAllReference(context),"restore frame then resume actual decoder"))return false;
    if(!Check(resumed.IsMoviePlaying()&&resumed.GetCurrentFrame()>=savedFrame,"saved playing state resumes from saved frame"))return false;
    const auto resumedFrame=resumed.GetCurrentFrame();
    {const auto unlocked=SSystem::UnlockAll();Sleep(120);SSystem::Relock(unlocked);}
    if(!Check(resumed.GetCurrentFrame()>resumedFrame&&resumed.IsMoviePlaying(),"restored decoder advances independently"))return false;
    EMemoryFile again;if(!Status(again.Create(1024),"create repeated wire")||
       !Status(resumed.Save(again,context),"save restored playing graph again"))return false;
    resumed.CloseMovie();
    EMemoryFile shortFile;shortFile.Open(playing.GetBuffer(),playing.GetLength()-1);ECSMovieSprite partial;
    if(!Check(partial.Load(shortFile,context)!=eslErrSuccess,"truncated movie tail reports failure before opening a decoder"))return false;
    EMemoryFile empty;if(!Status(empty.Create(512),"empty movie record")||!Status(movie.Save(empty,context),"save closed movie tail"))return false;
    tail=static_cast<const uint8_t*>(empty.GetBuffer())+empty.GetLength()-24;
    if(!Check(StudySteadyLegacyWire::Read32(tail)==0&&StudySteadyLegacyWire::Read32(tail+4)==UINT32_MAX,
        "closed movie still writes mandatory old tail"))return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady","Legacy Movie probe PASS: real NOA MEI decode, native progression, stopped/playing-loop save and decoder restoration, pixels, parent, repeat-save, truncation and close");return true;
}
