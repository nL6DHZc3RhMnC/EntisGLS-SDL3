#pragma once
#include <sakuragl/sakuragl.h>
#include <sakuragl/media/sgl_audio_player.h>
#include <memory>
#include <atomic>
#include <mutex>
#include "runtime/cotopha_port/legacy_volume_curve.h"

class LegacyVolumeEnvelope;

// Resource's script ABI remains Cotopha; media is owned by the current native
// Android SDK. Borrowed image/player pointers must not be deleted by callers.
class ECSResource : public ECSObject {
public:
    DECLARE_CLASS_INFO(ECSResource, ECSObject)
    enum PlayTypeFlag : int {
        ptfDevice = (-2147483647 - 1), ptfNothing = -1,
        ptfMusic, ptfSound, ptfVoice, ptfSystem, ptfMovie,
        ptfUser0, ptfUser1, ptfUser2, ptfUser3, ptfMax
    };
    ECSResource();
    ~ECSResource() override;
    const wchar_t *GetTypeName() const override;
    ECSObject *GetTypeOf(const wchar_t *) override;
    ECSObject *Duplicate() override;
    ESLError Move(ECSContext &, ECSObject *) override;
    ESLError UnaryOperate(ECSContext &, CSUnaryOperatorType) override;
    ESLError Operate(ECSContext &, CSOperatorType, ECSObject *) override;
    ESLError Compare(ECSContext &, int &, CSCompareType, ECSObject &) override;
    ECSObject *GetVariableAt(int) override;
    void IndexAllMember() override;
    void CleanupAllReference(ECSContext &) override;
    ESLError CommitAllReference(ECSContext &) override;
    ESLError GetFunction(ECSContext &, int &, const wchar_t *) override;
    ESLError CallFunction(ECSContext &, int, ECSObjArray<ECSObject> &) override;
    ESLError Save(ESLFileObject &, ECSContext &) override;
    ESLError Load(ESLFileObject &, ECSContext &) override;
    void OnDestruction(ECSContext &) override;

    virtual SakuraGL::SGLImageObject *GetImage() const { return m_image.get(); }
    SakuraGL::SGLAudioPlayerInterface *GetSound() const { return m_sound.get(); }
    ESLObject *GetResource() const;
    virtual ESLError LoadImageFile(const wchar_t *, ECSContext * = nullptr);
    virtual ESLError ReadImageFile(ESLFileObject &);
    ESLError SaveImageFile(const wchar_t *,const wchar_t *,int,ECSContext * = nullptr) const;
    ESLError WriteImageFile(ESLFileObject &,const wchar_t *,int) const;
    virtual ESLError LoadSoundFile(const wchar_t *, unsigned = unsigned(-1), ECSContext * = nullptr);
    virtual ESLError ReadSoundFile(ESLFileObject &, unsigned = unsigned(-1));
    ESLError AttachSound(ECSResource *, ECSContext * = nullptr);
    ESLError AttachNativeResource(SSystem::SObject* resource, std::shared_ptr<void> owner);
    virtual ESLError Release();
    virtual ESLError Play(unsigned introSamples = unsigned(-1), int playType = 0);
    virtual ESLError PlayFrom(unsigned start = 0, unsigned end = unsigned(-1),
                             bool repeat = false, unsigned rewind = unsigned(-1), int type = 0);
    virtual ESLError Stop();
    virtual ESLError Pause();
    virtual ESLError Restart();
    virtual bool IsPlaying();
    virtual ESLError SetVolume(float, float);
    ESLError SetVolumeEnvelope(const LegacyVolumeCurve &, unsigned milliseconds);
    void CancelVolumeEnvelope();
    bool IsPendingEnvelope() const;
    ESLError GetVolumeEnvelopeError() const;
    static ECSStrTagArray *m_staFuncName;
    static const wchar_t *m_pwszFuncName[29];
protected:
    std::shared_ptr<SakuraGL::SGLImageObject> m_image;
    std::shared_ptr<SSystem::SFileInterface> m_imageStream;
    std::shared_ptr<SakuraGL::SGLAudioPlayer> m_sound;
    EWideString m_wstrFileName;
    std::atomic<float> m_volume[2] = {1, 1};
    std::atomic<int> m_nPlayType{ptfNothing};
    ECSReference m_refAttachSound;
    uint32_t m_nThreshold = UINT32_MAX, m_nRewindPos = UINT32_MAX;
    uint32_t m_nStartPos = 0, m_nEndPos = UINT32_MAX, m_nRepeatPlaying = 2;
    bool m_restorePlayback = false;
    uint32_t m_restoreResourceKind = 0;
    bool m_resourceStateCommitted = false, m_committingResourceState = false;
    static std::atomic<float> m_totalVolumes[9];
    // SDK AudioPlayer::SetVolume mutates an unsynchronized array. Serialize
    // envelope ticks, direct volume changes and total-volume reflection.
    mutable std::mutex m_volumeMutex;
    ESLError PushMediaInfo(ECSContext &, bool sound);
    ESLError ApplyVolume();
    ESLError ApplyVolumeUnlocked();
private:
    mutable std::mutex m_envelopeMutex;
    std::unique_ptr<LegacyVolumeEnvelope> m_envelope;
};
