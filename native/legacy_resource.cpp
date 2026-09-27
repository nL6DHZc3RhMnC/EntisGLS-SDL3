#include "legacy_compat/gls.h"
#include "legacy_resource.h"
#include "legacy_device_volume.h"
#include "legacy_image_export.h"
#include "legacy_atomic_file.h"
#include "../tools/legacy_serialization.h"
#include <algorithm>
#include <mutex>
#include <set>
#include <cmath>
#include <chrono>
#include <condition_variable>
#include <functional>
#include "platform/log.h"

// A real SDK thread supplies the TLS context required by the media decoders
// (and the JNI attachment used by the original Android backend).
// No script lock is taken here: Cancel() must be able to join from a native
// method while its calling ECSContext holds the interpreter lock.
class LegacyVolumeEnvelope final : public SSystem::SProcedure {
public:
    ~LegacyVolumeEnvelope() override { Cancel(); }
    ESLError Start(const LegacyVolumeCurve &curve, unsigned milliseconds,
                   std::function<ESLError(LegacyVolumePoint)> apply) {
        Cancel();
        curve_ = curve;
        duration_ = milliseconds;
        apply_ = std::move(apply);
        error_.store(eslErrSuccess);
        {
            std::lock_guard<std::mutex> lock(waitMutex_);
            cancelled_ = false;
        }
        pending_.store(true);
        if (thread_.BeginThread(this) != SSystem::errSuccess) {
            pending_.store(false);
            error_.store(eslErrGeneral);
            thread_.Delete();
            return eslErrGeneral;
        }
        return eslErrSuccess;
    }
    void Cancel() {
        {
            std::lock_guard<std::mutex> lock(waitMutex_);
            cancelled_ = true;
        }
        wake_.notify_all();
        thread_.Delete();
        pending_.store(false);
    }
    bool Pending() const { return pending_.load(); }
    ESLError Error() const { return error_.load(); }
    LegacyVolumePoint Endpoint() const { return curve_.back(); }
    void Run() override {
        using Clock = std::chrono::steady_clock;
        using Milliseconds = std::chrono::milliseconds;
        const auto start = Clock::now(), end = start + Milliseconds(duration_);
        auto next = start;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(waitMutex_);
                if (wake_.wait_until(lock, next, [this] { return cancelled_; })) break;
            }
            const auto now = Clock::now();
            const auto elapsed = std::chrono::duration_cast<Milliseconds>(now - start).count();
            // EGLSTime quantizes the full interval to 0x10000 before evaluating
            // the curve; preserve that sampling, including the exact endpoint.
            const uint64_t offset = std::min<uint64_t>(0x10000,
                uint64_t(elapsed) * 0x10000 / duration_);
            const auto result = apply_(LegacyEvaluateVolumeCurve(curve_, double(offset) / 0x10000));
            if (result) {
                error_.store(result);
                study::platform::LogPrint(study::platform::LogPriority::Error, "StudySteady",
                    "Legacy volume envelope stopped: audio backend error %d", int(result));
                break;
            }
            if (offset == 0x10000) break;
            next = std::min(now + Milliseconds(66), end);
        }
        pending_.store(false);
    }
    void Finalize() override {
        // Release SDK TLS before this short-lived worker exits. ThreadProc's
        // later TLS release is harmless.
        SSystem::SThread::ReleaseLocalStorage();
#if defined(__ANDROID__) && !defined(STUDYSTEADY_PLATFORM_SDL3)
        // The original SDK leaves JavaVM::DetachCurrentThread commented out.
        if (JNI::g_JavaVM) JNI::g_JavaVM->DetachCurrentThread();
#endif
    }
private:
    SSystem::SThread thread_;
    std::mutex waitMutex_;
    std::condition_variable wake_;
    bool cancelled_ = true;
    std::atomic<bool> pending_{false};
    std::atomic<ESLError> error_{eslErrSuccess};
    LegacyVolumeCurve curve_;
    unsigned duration_ = 0;
    std::function<ESLError(LegacyVolumePoint)> apply_;
};

namespace {
std::mutex resourceMutex;
std::set<ECSResource *> resources;
ESLError mediaError(SakuraGL::SGLError error) { return error ? eslErrGeneral : eslErrSuccess; }
}
IMPLEMENT_CLASS_INFO(ECSResource, ECSObject)
ECSStrTagArray *ECSResource::m_staFuncName = nullptr;
std::atomic<float> ECSResource::m_totalVolumes[9] = {1,1,1,1,1,1,1,1,1};
const wchar_t *ECSResource::m_pwszFuncName[29] = {
    L"LoadImage", L"LoadSound", L"LoadMidi", L"SaveImage", L"Release",
    L"AttachSound", L"Play", L"PlayFrom", L"SetRewindingPortion", L"Stop", L"Pause",
    L"Restart", L"GetVolume", L"SetVolume", L"GetTotalVolume", L"SetTotalVolume",
    L"IsPlaying", L"GetPlayingPosition", L"GetInfo", L"GetImageInfo", L"GetSoundInfo",
    L"SetVolumeEnvelope", L"CancelVolumeEnvelope", L"IsPendingEnvelope", L"GetPixel",
    L"GetPixelRect", L"SetPixelRect", L"GetWaveData", nullptr
};
ECSResource::ECSResource() {
    m_vtType = csvtObject;
    std::lock_guard<std::mutex> lock(resourceMutex);
    resources.insert(this);
}
ECSResource::~ECSResource() {
    CancelVolumeEnvelope();
    // A temporary source's ownership may have transferred into this hidden
    // reference. Release it before taking the registry lock: deleting another
    // Resource also removes that object from the same registry.
    m_refAttachSound.SetReference(nullptr, nullptr);
    std::lock_guard<std::mutex> lock(resourceMutex);
    resources.erase(this);
}
const wchar_t *ECSResource::GetTypeName() const { return L"Resource"; }
ESLError ECSResource::AttachNativeResource(SSystem::SObject* object, std::shared_ptr<void> owner) {
    if (auto* image = ESLTypeCast<SakuraGL::SGLImageObject>(object)) {
        Release();
        m_image = std::shared_ptr<SakuraGL::SGLImageObject>(std::move(owner), image);
        return eslErrSuccess;
    }
    if (auto* audio = ESLTypeCast<SakuraGL::SGLAudioPlayer>(object)) {
        Release();
        std::lock_guard<std::mutex> lock(m_volumeMutex);
        m_sound = std::make_shared<SakuraGL::SGLAudioPlayer>(audio->ClonePlayer(), true);
        return eslErrSuccess;
    }
    return eslErrNotSupported;
}
ECSObject *ECSResource::GetTypeOf(const wchar_t *name) {
    return !EWideString::Compare(name, L"Resource") ? this : ECSObject::GetTypeOf(name);
}
ECSObject *ECSResource::Duplicate() {
    auto *result = new ECSResource;
    result->m_image = m_image;
    result->m_imageStream = m_imageStream;
    { std::lock_guard<std::mutex> lock(m_volumeMutex);
      if (m_sound) result->m_sound = std::make_shared<SakuraGL::SGLAudioPlayer>(m_sound->ClonePlayer(), true); }
    result->m_wstrFileName = m_wstrFileName;
    return result;
}
ESLError ECSResource::Move(ECSContext &context, ECSObject *object) {
    auto *source = ESLTypeCast<ECSResource>(ECSObject::GetEntity(object));
    if (!source) return ESLErrorMsg("Resource assignment requires a Resource");
    if (source != this) {
        Release();
        m_image = source->m_image;
        m_imageStream = source->m_imageStream;
        { std::scoped_lock lock(m_volumeMutex, source->m_volumeMutex);
          if (source->m_sound) m_sound = std::make_shared<SakuraGL::SGLAudioPlayer>(source->m_sound->ClonePlayer(), true); }
        m_wstrFileName = source->m_wstrFileName;
    }
    context.delete_CSObject(object);
    return eslErrSuccess;
}
ESLError ECSResource::UnaryOperate(ECSContext &, CSUnaryOperatorType) { return ESLErrorMsg("Resource has no unary operator"); }
ESLError ECSResource::Operate(ECSContext &, CSOperatorType, ECSObject *) { return ESLErrorMsg("Resource has no arithmetic operator"); }
ESLError ECSResource::Compare(ECSContext &, int &, CSCompareType, ECSObject &) { return ESLErrorMsg("Resource has no value comparison"); }
ESLObject *ECSResource::GetResource() const { return m_image ? static_cast<ESLObject *>(m_image.get()) : static_cast<ESLObject *>(m_sound.get()); }
ESLError ECSResource::LoadImageFile(const wchar_t *path, ECSContext *context) {
    if (!context || !path) return eslErrInvalidParam;
    std::unique_ptr<ESLFileObject> file(context->OpenFileOnScript(path));
    if (!file) return eslErrGeneral;
    const ESLError result = ReadImageFile(*file);
    if (!result) m_wstrFileName = path;
    return result;
}
ESLError ECSResource::ReadImageFile(ESLFileObject &file) {
    auto *copy = file.Duplicate();
    if (!copy) return eslErrGeneral;
    auto stream = std::make_shared<SESLFileInterface>(copy, true);
    auto image = std::make_shared<SakuraGL::SGLImage>();
    const auto result = image->ReadImage(stream.get());
    if (result) return eslErrGeneral;
    Release();
    m_image = std::move(image);
    m_imageStream = std::move(stream);
    return eslErrSuccess;
}
ESLError ECSResource::WriteImageFile(ESLFileObject &file,const wchar_t *mime,int quality) const {
    auto *image=GetImage();if(!image)return eslErrGeneral;
    EMemoryFile encoded;
    if(const auto error=LegacyEncodeImage(*image,encoded,mime,quality))return error;
    return file.Write(encoded.GetBuffer(),encoded.GetLength())==encoded.GetLength()?eslErrSuccess:eslErrGeneral;
}
ESLError ECSResource::SaveImageFile(const wchar_t *path,const wchar_t *mime,int quality,ECSContext *context) const {
    if(!path||!*path)return eslErrInvalidParam;
    auto *image=GetImage();if(!image)return eslErrGeneral;
    EMemoryFile encoded;
    if(const auto error=LegacyEncodeImage(*image,encoded,mime,quality))return error;
    bool candidate=false;
    std::unique_ptr<LegacyAtomicSaveFile> atomic(LegacyAtomicSaveFile::TryOpen(
        context?context->GetEnvironment():nullptr,path,ESLFileObject::modeCreate,candidate));
    if(candidate) {
        if(!atomic)return eslErrGeneral;
        atomic->BeginSave();
        return atomic->Replace(encoded.GetBuffer(),encoded.GetLength(),0);
    }
    std::unique_ptr<ESLFileObject> file(context?context->OpenFileOnScript(path,ESLFileObject::modeCreate):nullptr);
    if(!context) {
        auto *native=SSystem::SFileOpener::DefaultNewOpenFile(path,ESLFileObject::modeCreate);
        if(native)file.reset(new LegacyFileAdapter(native,ESLFileObject::modeCreate));
    }
    if(!file)return eslErrGeneral;
    return file->Write(encoded.GetBuffer(),encoded.GetLength())==encoded.GetLength()?eslErrSuccess:eslErrGeneral;
}
ESLError ECSResource::LoadSoundFile(const wchar_t *path, unsigned threshold, ECSContext *context) {
    if (!context || !path) return eslErrInvalidParam;
    auto player = std::make_shared<SakuraGL::SGLAudioPlayer>();
    const auto result = player->Open(path, SakuraGL::SGLAudioPlayerInterface::modeOpenAuto,
                                    context->GetEnvironment());
    if (result) return eslErrGeneral;
    Release();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound = std::move(player); }
    m_wstrFileName = path;
    m_nThreshold = threshold;
    return ApplyVolume();
}
ESLError ECSResource::ReadSoundFile(ESLFileObject &file, unsigned threshold) {
    auto *copy = file.Duplicate();
    if (!copy) return eslErrGeneral;
    auto player = std::make_shared<SakuraGL::SGLAudioPlayer>();
    auto *stream = new SESLFileInterface(copy, true);
    const auto result = player->Create(stream, true, SakuraGL::SGLAudioPlayerInterface::modeOpenAuto);
    if (result) return eslErrGeneral;
    Release();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound = std::move(player); }
    m_nThreshold = threshold;
    return ApplyVolume();
}
ESLError ECSResource::AttachSound(ECSResource *resource, ECSContext *context) {
    if (!resource) return eslErrInvalidParam;
    // Reattaching a loaded, privately owned source must keep it alive while
    // Release clears the old hidden reference. Cotopha transfers its ownership
    // to this temporary and then back to the newly established hidden link.
    ECSReference sourceLifetime(resource);
    std::shared_ptr<SakuraGL::SGLAudioPlayer> sound;
    {
        std::lock_guard<std::mutex> lock(resource->m_volumeMutex);
        if (!resource->m_sound) return eslErrInvalidParam;
        sound = std::make_shared<SakuraGL::SGLAudioPlayer>(resource->m_sound->ClonePlayer(), true);
        if (!sound->GetPlayer()) return eslErrGeneral;
    }
    Release();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound = std::move(sound); }
    m_refAttachSound.SetReference(resource, context);
    return ApplyVolume();
}
ESLError ECSResource::Release() {
    CancelVolumeEnvelope();
    { std::lock_guard<std::mutex> lock(m_volumeMutex); m_sound.reset(); }
    m_image.reset();
    m_imageStream.reset();
    m_wstrFileName = L"";
    m_refAttachSound.SetReference(nullptr, nullptr);
    m_nPlayType = ptfNothing;
    m_nThreshold = m_nRewindPos = m_nEndPos = UINT32_MAX;
    m_nStartPos = 0; m_nRepeatPlaying = 2;
    m_restorePlayback = false; m_restoreResourceKind = 0;
    m_resourceStateCommitted = false;
    return eslErrSuccess;
}
ESLError ECSResource::ApplyVolume() {
    std::lock_guard<std::mutex> lock(m_volumeMutex);
    return ApplyVolumeUnlocked();
}
ESLError ECSResource::ApplyVolumeUnlocked() {
    if (!m_sound) return eslErrSuccess;
    const int type = m_nPlayType.load(std::memory_order_relaxed);
    const float total = (type >= 0 && type < 9) ? m_totalVolumes[type].load(std::memory_order_relaxed) : 1;
    const float values[2] = {m_volume[0].load(std::memory_order_relaxed) * total, m_volume[1].load(std::memory_order_relaxed) * total};
    return mediaError(m_sound->SetVolume(values, 2));
}
ESLError ECSResource::SetVolume(float left, float right) {
    if (!std::isfinite(left) || !std::isfinite(right) || left < 0 || right < 0) return eslErrInvalidParam;
    std::lock_guard<std::mutex> lock(m_volumeMutex);
    m_volume[0] = left; m_volume[1] = right;
    return ApplyVolumeUnlocked();
}
ESLError ECSResource::SetVolumeEnvelope(const LegacyVolumeCurve &curve, unsigned milliseconds) {
    if (!LegacyValidVolumeCurve(curve)) return eslErrInvalidParam;
    std::lock_guard<std::mutex> lock(m_envelopeMutex);
    if (!m_sound) return eslErrGeneral;
    if (m_envelope) m_envelope->Cancel();
    // GLS3 specifically indexes point 3 for zero-duration curves, even when
    // several segments were supplied. Do not silently substitute curve.back().
    if (!milliseconds) {
        m_envelope.reset();
        return SetVolume(curve[3].left, curve[3].right);
    }
    if (const auto error = SetVolume(curve.front().left, curve.front().right)) return error;
    if (!m_envelope) m_envelope = std::make_unique<LegacyVolumeEnvelope>();
    return m_envelope->Start(curve, milliseconds, [this](LegacyVolumePoint point) {
        return SetVolume(point.left, point.right);
    });
}
void ECSResource::CancelVolumeEnvelope() {
    std::lock_guard<std::mutex> lock(m_envelopeMutex);
    if (m_envelope) m_envelope->Cancel();
}
bool ECSResource::IsPendingEnvelope() const {
    std::lock_guard<std::mutex> lock(m_envelopeMutex);
    return m_envelope && m_envelope->Pending();
}
ESLError ECSResource::GetVolumeEnvelopeError() const {
    std::lock_guard<std::mutex> lock(m_envelopeMutex);
    return m_envelope ? m_envelope->Error() : eslErrSuccess;
}
ESLError ECSResource::Play(unsigned intro, int type) {
    if (!m_sound) return eslErrGeneral;
    if (const auto error = Stop()) return error;
    bool repeat = false;
    int64_t loopStart = 0, loopEnd = -1;
    auto *stream = m_sound->GetAudioStream();
    if (stream) {
        SakuraGL::SGLMediaOptionalInfo info;
        if (!stream->GetAudioOptinalInfo(info)) {
            repeat = (info.m_nFlags & SakuraGL::SGLMediaOptionalInfo::flagLoopStart) != 0;
            if (repeat) loopStart = info.m_nLoopStart;
            if (info.m_nFlags & SakuraGL::SGLMediaOptionalInfo::flagLoopEnd) loopEnd = info.m_nLoopEnd;
        }
        m_sound->ReleaseAudioStream(stream);
    }
    const int32_t signedIntro = static_cast<int32_t>(intro);
    if (signedIntro >= 0) { repeat = true; loopStart = intro; }
    else if (signedIntro == -2) repeat = false;
    return PlayFrom(0, loopEnd < 0 ? unsigned(-1) : static_cast<unsigned>(loopEnd),
                    repeat, static_cast<unsigned>(loopStart), type);
}
ESLError ECSResource::PlayFrom(unsigned start, unsigned end, bool repeat, unsigned rewind, int type) {
    if (!m_sound) return eslErrGeneral;
    if (const auto error = Stop()) return error;
    const uint64_t total = m_sound->GetTotalLength();
    const uint64_t stop = end == unsigned(-1) ? total : end;
    const uint64_t loopStart = rewind == unsigned(-1) ? start : rewind;
    if (!total || start >= stop || stop > total || (repeat && loopStart >= stop)) return eslErrInvalidParam;
    // This SDK's one-shot interface has no end-position control. Its static
    // loop player plays only the loop slice, so it cannot reproduce an intro
    // before a nonzero loop point. Report these limits instead of changing audio.
    if (!repeat && stop != total) return eslErrNotSupported;
    auto *reader = ESLTypeCast<SakuraGL::SGLAudioBufferReader>(m_sound->GetPlayer());
    if (repeat && loopStart && reader && reader->GetStaticBufferSize()) return eslErrNotSupported;
    const auto result = m_sound->SetLoop(true, repeat ? loopStart : 0, repeat ? stop : total);
    if (result) return eslErrGeneral;
    // Resetting the full range before disabling also restores a previously
    // truncated static loop buffer; SetLoop(false) alone leaves that buffer.
    if (!repeat && m_sound->SetLoop(false)) return eslErrGeneral;
    m_sound->SeekPosition(start);
    m_nPlayType = type;
    if (const auto volumeError = ApplyVolume()) return volumeError;
    std::lock_guard<std::mutex> lock(m_volumeMutex);
    const auto playError = mediaError(m_sound->Play());
    if (!playError) {
        m_nStartPos = start; m_nEndPos = end; m_nRewindPos = rewind;
        m_nRepeatPlaying = repeat ? 1 : 0;
    }
    return playError;
}
ESLError ECSResource::Stop() {
    const auto error = m_sound ? mediaError(m_sound->Stop()) : eslErrSuccess;
    if (!error) m_nPlayType = ptfNothing;
    return error;
}
ESLError ECSResource::Pause() { return m_sound ? mediaError(m_sound->Pause()) : eslErrGeneral; }
ESLError ECSResource::Restart() { return m_sound ? mediaError(m_sound->Restart()) : eslErrGeneral; }
bool ECSResource::IsPlaying() { return m_sound && m_sound->IsPlaying(); }
ESLError ECSResource::GetFunction(ECSContext &, int &index, const wchar_t *name) {
    index = m_staFuncName ? m_staFuncName->FindIndex(name) : -1;
    return index < 0 ? ESLErrorMsg("Unknown Resource method") : eslErrSuccess;
}
ECSObject *ECSResource::GetVariableAt(int index) {
    return index == -1 ? &m_refAttachSound : ECSObject::GetVariableAt(index);
}
void ECSResource::IndexAllMember() {
    ECSObject::IndexAllMember();
    m_refAttachSound.IndexAllMember();
    m_refAttachSound.m_pParent = this;
    m_refAttachSound.m_nIndex = -1;
}
void ECSResource::CleanupAllReference(ECSContext &context) {
    m_refAttachSound.CleanupAllReference(context);
    ECSObject::CleanupAllReference(context);
}
ESLError ECSResource::CommitAllReference(ECSContext &context) {
    if (m_resourceStateCommitted) return ECSObject::CommitAllReference(context);
    if (m_committingResourceState) return ESLErrorMsg("Saved Resource attachments form a cycle without a media source");
    struct CommitScope {
        bool &flag;
        explicit CommitScope(bool &state) : flag(state) { flag = true; }
        ~CommitScope() { flag = false; }
    } committing(m_committingResourceState);
    if (const auto error = m_refAttachSound.CommitAllReference(context)) return error;
    if (m_restorePlayback) {
        const int type = m_nPlayType.load();
        const uint32_t kind = m_restoreResourceKind, threshold = m_nThreshold;
        const uint32_t start = m_nStartPos, end = m_nEndPos, rewind = m_nRewindPos, repeat = m_nRepeatPlaying;
        auto *source = ESLTypeCast<ECSResource>(ECSObject::GetEntity(&m_refAttachSound));
        if (source) {
            if (kind != 2) return ESLErrorMsg("Saved Resource attachment conflicts with its media type");
            if (source->m_restorePlayback) {
                if (const auto error = source->ECSResource::CommitAllReference(context)) return error;
            }
            if (const auto error = AttachSound(source, &context)) return error;
            m_nPlayType = type; m_nThreshold = threshold; m_nStartPos = start;
            m_nEndPos = end; m_nRewindPos = rewind; m_nRepeatPlaying = repeat;
            m_restorePlayback = true; m_restoreResourceKind = kind;
        }
        if (kind == 2 && !m_sound)
            return ESLErrorMsg("Saved Resource sound has neither a reopenable file nor a resolved attachment");
        // GLS3 resumes looping media and music after all references resolve;
        // one-shot effects/voices are intentionally not restarted on load.
        if (type != ptfNothing && (repeat || type == ptfMusic)) {
            if (const auto error = PlayFrom(start, end, repeat != 0, rewind, type))
                return error;
        }
        m_restorePlayback = false;
        m_resourceStateCommitted = true;
    }
    return ECSObject::CommitAllReference(context);
}
ESLError ECSResource::PushMediaInfo(ECSContext &context, bool sound) {
    if (sound && m_sound) {
        auto *stream = m_sound->GetAudioStream();
        if (!stream) return eslErrGeneral;
        SakuraGL::SGLSoundFormat format;
        const auto error = stream->GetAudioFormat(format);
        m_sound->ReleaseAudioStream(stream);
        if (error) return eslErrGeneral;
        auto *info = context.CreateUserStructure(L"SoundInfo");
        if (!info) return eslErrGeneral;
        info->SetMemberAsInt(L"nSamplesPerSec", format.frequency);
        info->SetMemberAsInt(L"nChannelCount", format.channels);
        info->SetMemberAsInt(L"nBitsPerSample", format.bitsPerSample);
        info->SetMemberAsInt(L"nResourceBytes", format.SamplesToBytes(m_sound->GetTotalLength()));
        info->SetMemberAsInt(L"nSampleCount", m_sound->GetTotalLength());
        info->SetMemberAsInt(L"nRewoundPosition", 0);
        return context.PushObject(*info);
    }
    if (!sound && GetImage()) {
        auto* image = GetImage();
        SakuraGL::SGLImageInfo native;
        if (image->GetImageInfo(native)) return eslErrGeneral;
        auto *info = context.CreateUserStructure(L"ImageInfo");
        if (!info) return eslErrGeneral;
        info->SetMemberAsInt(L"nFormatType", native.format);
        info->SetMemberAsInt(L"nImageWidth", native.width);
        info->SetMemberAsInt(L"nImageHeight", native.height);
        info->SetMemberAsInt(L"nBitsPerPixel", native.depth);
        info->SetMemberAsInt(L"nFrameCount", image->GetFrameCount());
        info->SetMemberAsInt(L"xHotSpot", native.ptOrigin.x);
        info->SetMemberAsInt(L"yHotSpot", native.ptOrigin.y);
        info->SetMemberAsInt(L"nResourceBytes", std::abs(native.pitchLine) * native.height * image->GetFrameCount());
        return context.PushObject(*info);
    }
    return context.PushObject(context.new_CSReference());
}

ESLError ECSResource::CallFunction(ECSContext &context, int index, ECSObjArray<ECSObject> &args) {
    static const unsigned counts[28][2] = {{2,2},{2,3},{2,2},{2,4},{1,1},{2,2},{1,3},{1,6},{1,4},
        {1,1},{1,1},{1,1},{3,3},{3,3},{2,2},{3,3},{1,1},{1,1},{1,1},{1,1},{1,1},{3,3},{1,1},{1,1},{3,3},{6,6},{6,6},{1,8}};
    if (index < 0 || index >= 28) return ESLErrorMsg("Unknown Resource method index");
    if (const auto error = context.VerifyArgumentCount(args, counts[index][0], counts[index][1])) return error;
    auto push = [&](INT64 value) { return context.PushObject(context.new_CSInteger(value)); };
    ESLError error = eslErrSuccess;
    auto integer = [&](int argument, int fallback = 0) { int value = fallback; if (!error) error = context.GetArgumentAsInt(value, args, argument, fallback); return value; };
    auto real = [&](int argument, double fallback = 1.) { double value = fallback; if (!error) error = context.GetArgumentAsReal(value, args, argument, fallback); return value; };
    switch (index) {
    case 0: case 1: {
        EWideString path;
        auto *file = ESLTypeCast<ECSFile>(context.GetArgumentObjectAs(args, 1, L"File"));
        const int threshold = index == 1 ? integer(2, -1) : -1;
        if (error) return error;
        if (file && file->GetFileInterface()) error = index == 0 ? ReadImageFile(*file->GetFileInterface()) : ReadSoundFile(*file->GetFileInterface(), threshold);
        else {
            if (const auto argError = context.GetArgumentAsStr(path, args, 1, nullptr)) return argError;
            error = index == 0 ? LoadImageFile(path, &context) : LoadSoundFile(path, threshold, &context);
        }
        return push(error);
    }
    case 2: return ESLErrorMsg("Resource.LoadMidi is not implemented by the current media backend");
    case 3: {
        EWideString path,mime;
        if((error=context.GetArgumentAsStr(path,args,1,nullptr))||
           (error=context.GetArgumentAsStr(mime,args,2,L"image/x-eri")))return error;
        const int quality=integer(3,-1);if(error)return error;
        return push(SaveImageFile(path,mime,quality,&context));
    }
    case 4: return push(Release());
    case 5: {
        auto *resource = ESLTypeCast<ECSResource>(context.GetArgumentObjectAs(args, 1, L"Resource"));
        return push(AttachSound(resource, &context));
    }
    case 6: { const int intro = integer(1,-1), type = integer(2); return error ? error : push(Play(intro,type)); }
    case 7: { const int start = integer(1), end = integer(2,-1), repeat = integer(3), rewind = integer(4,-1), type = integer(5); return error ? error : push(PlayFrom(start,end,repeat,rewind,type)); }
    case 8: {
        const int rewind = integer(1,-1), end = integer(2,-1), repeat = integer(3);
        if (error) return error;
        const auto loopError = m_sound ? mediaError(m_sound->SetLoop(repeat,rewind,end)) : eslErrGeneral;
        if (!loopError) { m_nRewindPos = rewind; m_nEndPos = end; m_nRepeatPlaying = repeat ? 1 : 0; }
        return push(loopError);
    }
    case 9: CancelVolumeEnvelope(); return push(Stop());
    case 10: return push(Pause());
    case 11: return push(Restart());
    case 12: {
        auto *left = ESLTypeCast<ECSReal>(context.GetArgumentObjectAs(args,1,L"Real"));
        auto *right = ESLTypeCast<ECSReal>(context.GetArgumentObjectAs(args,2,L"Real"));
        if (!left || !right) return eslErrInvalidParam;
        std::lock_guard<std::mutex> lock(m_volumeMutex);
        left->m_varReal = m_volume[0]; right->m_varReal = m_volume[1]; return push(0);
    }
    case 13: { const double left = real(1), right = real(2); return error ? error : push(SetVolume(left,right)); }
    case 14: {
        const int type = integer(1); if (error) return error;
        double volume = 1;
        if (type == ptfDevice) {
            if (!LegacyGetDeviceVolume(volume)) return ESLErrorMsg("Media master volume query failed");
        } else if (type >= 0 && type < 9) volume = m_totalVolumes[type].load(std::memory_order_relaxed);
        return context.PushObject(context.new_CSReal(volume));
    }
    case 15: {
        const int type = integer(1); const double value = real(2);
        if (error) return error;
        if (type == ptfDevice) return push(LegacySetDeviceVolume(value) ? 0 : eslErrGeneral);
        if (type < 0 || type >= 9 || !std::isfinite(value) || value < 0) return eslErrInvalidParam;
        std::lock_guard<std::mutex> lock(resourceMutex);
        m_totalVolumes[type].store(value,std::memory_order_relaxed);
        for (auto *resource : resources) if (resource->m_nPlayType == type) {
            if (const auto volumeError = resource->ApplyVolume()) return volumeError;
        }
        return push(0);
    }
    case 16: return push(IsPlaying());
    case 17: return push(m_sound ? m_sound->GetPosition() : 0);
    case 18: return PushMediaInfo(context, bool(m_sound));
    case 19: return PushMediaInfo(context, false);
    case 20: return PushMediaInfo(context, true);
    case 21: {
        auto *array = ESLTypeCast<ECSArray>(context.GetArgumentObjectAs(args, 1, L"Array"));
        const int milliseconds = integer(2, 1000);
        if (error) return error;
        if (!array || milliseconds < 0) return eslErrInvalidParam;
        LegacyVolumeCurve curve;
        curve.reserve(array->m_varArray.GetSize());
        for (unsigned i = 0; i < array->m_varArray.GetSize(); ++i) {
            auto *point = ESLTypeCast<ECSStructureInterface>(ECSObject::GetEntity(array->m_varArray.GetAt(i)));
            if (!point) return ESLErrorMsg("Resource.SetVolumeEnvelope requires an array of Vector2D-compatible structures");
            curve.push_back({float(point->GetMemberAsReal(L"x", 1)), float(point->GetMemberAsReal(L"y", 1))});
        }
        return push(SetVolumeEnvelope(curve, unsigned(milliseconds)));
    }
    case 22: CancelVolumeEnvelope(); return push(0);
    case 23: return push(IsPendingEnvelope() ? -1 : 0);
    case 24: {
        const int x = integer(1), y = integer(2);
        if (error) return error;
        if (!GetImage()) return eslErrGeneral;
        SakuraGL::SGLPalette pixel;
        if (GetImage()->GetPixelRGBA(pixel,x,y)) return eslErrGeneral;
        return push(pixel.ui32);
    }
    default: return ESLErrorMsg("This Resource pixel/wave buffer operation is not ported yet");
    }
}
#include "legacy_resource_state.inc"
void ECSResource::OnDestruction(ECSContext &) { Release(); }
