#include "legacy_compat/gls.h"
#include "legacy_resource_state_probe.h"
#include "../tools/legacy_serialization.h"
#include "platform/log.h"
#include <cmath>
#include <cstring>
#include <memory>

namespace {
bool Check(bool ok, const char *stage) {
    if (!ok) study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady", "Legacy resource state probe FAIL: %s", stage);
    return ok;
}
class StateImage final : public ECSExecutionImage {
public:
    StateImage() {
        const BYTE code[] = {4,0,0,0,0,0,0,0,0,18,0};
        std::memset(&m_exiHeader,0,sizeof(m_exiHeader));
        m_exiHeader.nVersion=1; m_exiHeader.nIntBase=64;
        m_exiHeader.nStackSize=4096; m_exiHeader.nHeapSize=4096;
        m_exiHeader.fnStaticInitialize=UINT32_MAX; m_exiHeader.fnResumePrepare=UINT32_MAX;
        std::memcpy(m_bufImage.PutBuffer(sizeof(code)),code,sizeof(code)); m_bufImage.Flush(sizeof(code));
        m_pImage=static_cast<BYTE*>(m_bufImage.ModifyBuffer(0,sizeof(code))); m_dwImageSize=sizeof(code);
    }
};
class StateResource : public ECSResource {
public:
    LegacyVolumePoint Volume() const { return {m_volume[0].load(),m_volume[1].load()}; }
    void SetSourcePathForFailureTest(const wchar_t *path) { m_wstrFileName=path; }
};
class OwnedSource final : public ECSResource {
public:
    explicit OwnedSource(int &destroyed) : destroyed_(destroyed) {}
    ~OwnedSource() override { ++destroyed_; }
private:
    int &destroyed_;
};
bool Rewind(EMemoryFile &file) { return file.SeekLarge(0,ESLFileObject::FromBegin)==0; }
bool RoundTrip(ECSResource &source,ECSResource &restored,EMemoryFile &file,ECSContext &context) {
    return !file.Create(128) && !source.Save(file,context) && Rewind(file) &&
        !restored.Load(file,context) && !restored.CommitAllReference(context);
}
}

bool CheckLegacyResourceState(ECSEnvironment &environment) {
    StateImage image; image.AttachCSEnvironment(&environment);
    ECSContext context;
    ECSContext *previous=ECotophaScript::GetPrimaryContext();
    struct ContextScope {
        ECSContext &context; ECSContext *previous;
        ~ContextScope(){context.ReleaseContext(true);ECotophaScript::SetPrimaryContext(previous);}
    } scope{context,previous};
    if (!Check(!context.InitializeContext(&image),"initialize isolated serialization context")) return false;
    EMemoryFile file;
    StateResource empty,restoredEmpty;
    if (!Check(RoundTrip(empty,restoredEmpty,file,context) && !restoredEmpty.GetResource(),"empty resource round trip")) return false;
    const auto *wire=static_cast<const uint8_t*>(file.GetBuffer());
    if (!Check(file.GetLength()>=40 && StudySteadyLegacyWire::Read32(wire)==0 &&
        StudySteadyLegacyWire::Read32(wire+4)==UINT32_MAX && StudySteadyLegacyWire::Read32(wire+36)==0,
        "fixed Win32 LE header and filename boundary")) return false;

    StateResource picture,restoredPicture;
    if (!Check(!picture.LoadImageFile(L"particle_light1.eri",&context) &&
        RoundTrip(picture,restoredPicture,file,context),"file-backed ERI reopens through actual environment")) return false;
    SakuraGL::SGLImageInfo pictureInfo;
    SakuraGL::SGLPalette originalPixel,restoredPixel;
    if (!Check(restoredPicture.GetImage() && !restoredPicture.GetImage()->GetImageInfo(pictureInfo) &&
        pictureInfo.width==6 && pictureInfo.height==6 &&
        !picture.GetImage()->GetPixelRGBA(originalPixel,0,0) &&
        !restoredPicture.GetImage()->GetPixelRGBA(restoredPixel,0,0) && originalPixel.ui32==restoredPixel.ui32,
        "restored image dimensions and decoded pixels")) return false;

    StateResource sound,restoredSound;
    if (!Check(!sound.LoadSoundFile(L"se517.mio",1234,&context) && !sound.SetVolume(0.25f,0.75f) &&
        RoundTrip(sound,restoredSound,file,context),"file-backed MIO control state round trip")) return false;
    wire=static_cast<const uint8_t*>(file.GetBuffer());
    const auto volume=restoredSound.Volume();
    if (!Check(StudySteadyLegacyWire::Read32(wire)==2 && StudySteadyLegacyWire::Read32(wire+8)==1234 &&
        restoredSound.GetSound() && restoredSound.GetSound()->GetTotalLength()==1504 &&
        std::abs(volume.left-0.25f)<0.0001f && std::abs(volume.right-0.75f)<0.0001f && !restoredSound.IsPlaying(),
        "media kind, threshold, sample count, stereo volume and stopped state")) return false;

    if (!Check(!sound.SetVolume(0,0) && !sound.Play(0,ECSResource::ptfMusic) &&
        RoundTrip(sound,restoredSound,file,context),"playing loop snapshot and deferred native restoration")) return false;
    Sleep(160);
    if (!Check(restoredSound.IsPlaying(),"restored native loop survives more than one real clip duration")) return false;
    sound.Stop(); restoredSound.Stop();

    const LegacyVolumeCurve fade{{0,0},{0,0},{0,0},{0.4f,0.2f}};
    if (!Check(!sound.SetVolumeEnvelope(fade,1000) && RoundTrip(sound,restoredSound,file,context),
        "active envelope snapshot")) return false;
    const auto savedEndpoint=restoredSound.Volume();
    if (!Check(sound.IsPendingEnvelope() && !restoredSound.IsPendingEnvelope() &&
        std::abs(savedEndpoint.left-0.4f)<0.0001f && std::abs(savedEndpoint.right-0.2f)<0.0001f,
        "save keeps current fade running and restores its endpoint per GLS3")) return false;
    sound.CancelVolumeEnvelope();

    auto *globalSource=new ECSResource;
    if (!Check(!globalSource->LoadSoundFile(L"se517.mio",UINT32_MAX,&context),"attachment source file")) {delete globalSource;return false;}
    image.m_csgGlobal.AddVariable(L"savedAudioSource",globalSource);
    image.m_csgGlobal.IndexAllMember();
    StateResource attached,restoredAttached;
    if (!Check(!attached.AttachSound(globalSource,&context) && RoundTrip(attached,restoredAttached,file,context) &&
        restoredAttached.GetSound() && restoredAttached.GetSound()->GetTotalLength()==1504 &&
        ECSObject::GetEntity(restoredAttached.GetVariableAt(-1))==globalSource,
        "serialized attachment resolves through the actual global object graph")) return false;
    StateResource privateAttachment,restoredPrivateAttachment;
    {
        ECSReference temporary;
        auto *privateSource=new ECSResource;
        temporary.SetOwnObject(privateSource);
        if (!Check(!privateSource->LoadSoundFile(L"se517.mio",UINT32_MAX,&context) &&
            !privateAttachment.AttachSound(privateSource,&context),"private owned audio source")) return false;
        temporary.SetReference(nullptr);
    }
    if (!Check(RoundTrip(privateAttachment,restoredPrivateAttachment,file,context) &&
        restoredPrivateAttachment.GetSound() && restoredPrivateAttachment.GetSound()->GetTotalLength()==1504 &&
        ESLTypeCast<ECSReference>(restoredPrivateAttachment.GetVariableAt(-1))->m_pOwnObj,
        "owned source survives release and reattachment during graph restore")) return false;

    std::unique_ptr<ESLFileObject> anonymousFile(environment.OpenFileObject("se517.mio"));
    StateResource anonymous;
    if (!Check(anonymousFile && !anonymous.ReadSoundFile(*anonymousFile) && !file.Create(128) &&
        anonymous.Save(file,context)!=eslErrSuccess && file.GetLength()==0,
        "anonymous media fails before writing a misleading save record")) return false;
    sound.SetSourcePathForFailureTest(L"missing-resource-save-probe.mio");
    if (!Check(!file.Create(128) && !sound.Save(file,context) && Rewind(file) &&
        restoredSound.Load(file,context)!=eslErrSuccess,"missing original media fails restoration")) return false;
    uint8_t truncated[20] = {};
    if (!Check(!file.Open(truncated,sizeof(truncated)) && restoredEmpty.Load(file,context)!=eslErrSuccess,
        "truncated media header reports failure")) return false;
    int destroyed=0;
    {
        auto destination=std::make_unique<ECSResource>();
        ECSReference temporary;
        temporary.SetOwnObject(new OwnedSource(destroyed));
        auto *hidden=ESLTypeCast<ECSReference>(destination->GetVariableAt(-1));
        hidden->SetReference(temporary.m_pRef);
        temporary.SetReference(nullptr);
        if (!Check(hidden->m_pOwnObj && !destroyed,"hidden attachment takes ownership from a temporary")) return false;
    }
    if (!Check(destroyed==1,"standalone Resource destruction releases hidden owned source exactly once")) return false;
    study::platform::LogWrite(study::platform::LogPriority::Info,"StudySteady",
        "Legacy resource state probe PASS: LE wire, actual ERI/MIO reopen, stereo state, loop restoration, fade endpoint, attachment graph and unsupported/missing/truncated errors");
    return true;
}
