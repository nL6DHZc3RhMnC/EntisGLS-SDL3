#include "sdl_sound_player.h"

using namespace SakuraGL;
using studysteady::platform::SdlPcmStream;

ESL_IMPLEMENT_CLASS_INFO(SakuraGL::SGLSDLSoundPlayer, SGLSoundPlayerInterface)

namespace {
SGLError Result(bool success) { return success ? sglErrSuccess : sglErrFailed; }
SGLSoundPlayerInterface* NewPlayer(void*) { return new SGLSDLSoundPlayer; }
struct ReleaseCallerLocks {
    atomic_int_t count = SSystem::g_mutexGlobal ? SSystem::UnlockAll() : 0;
    ~ReleaseCallerLocks() { if (count) SSystem::Relock(count); }
};
}

SGLSDLSoundPlayer::~SGLSDLSoundPlayer() { Close(); }

SGLError SGLSDLSoundPlayer::Open(const SGLSoundFormat& format) {
    SdlPcmStream::Format pcm;
    pcm.frequency = format.frequency;
    pcm.channels = format.channels;
    if (format.format == formatSoundLinearPCM) {
        switch (format.bitsPerSample) {
        case 8: pcm.type = SdlPcmStream::SampleType::unsigned8; break;
        case 16: pcm.type = SdlPcmStream::SampleType::signed16; break;
        case 32: pcm.type = SdlPcmStream::SampleType::signed32; break;
        default: return sglErrNotSupported;
        }
    } else if (format.format == formatSoundIEEEFloat && format.bitsPerSample == 32) {
        pcm.type = SdlPcmStream::SampleType::float32;
    } else return sglErrNotSupported;
    if (format.channels < 1 || format.channels > 2) return sglErrNotSupported;
    ReleaseCallerLocks unlock;
    return Result(output_.Open(pcm, [] { SSystem::SThread::ReleaseLocalStorage(); }));
}

SGLError SGLSDLSoundPlayer::Close() {
    ReleaseCallerLocks unlock;
    return Result(output_.Close());
}
SGLError SGLSDLSoundPlayer::WriteStatic(const void* data, size_t bytes) { return Result(output_.WriteStatic(data, bytes)); }
SGLError SGLSDLSoundPlayer::PrepareStream(size_t bytes) { return Result(output_.PrepareStream(bytes)); }
size_t SGLSDLSoundPlayer::Write(const void* data, size_t bytes) { return output_.Write(data, bytes); }
SGLError SGLSDLSoundPlayer::Play(uint64_t flags) {
    if (flags & ~uint64_t(flagPlayLoop)) return sglErrNotSupported;
    return Result(output_.Play((flags & flagPlayLoop) != 0));
}
SGLError SGLSDLSoundPlayer::Stop() { return Result(output_.Stop()); }
SGLError SGLSDLSoundPlayer::Pause() { return Result(output_.Pause()); }
SGLError SGLSDLSoundPlayer::Restart() { return Result(output_.Restart()); }
SGLError SGLSDLSoundPlayer::GetVolume(float32_t* volumes, size_t channels) { return Result(output_.GetVolume(volumes, channels)); }
SGLError SGLSDLSoundPlayer::SetVolume(const float32_t* volumes, size_t channels) { return Result(output_.SetVolume(volumes, channels)); }
bool SGLSDLSoundPlayer::IsPlaying() const { return output_.IsPlaying(); }
bool SGLSDLSoundPlayer::IsPaused() const { return output_.IsPaused(); }
uint64_t SGLSDLSoundPlayer::GetPlayingPosition() { return output_.GetPlayingPosition(); }
SGLError SGLSDLSoundPlayer::SeekPosition(uint64_t byteOffset) { return Result(output_.SeekPosition(byteOffset)); }

SGLSoundPlayerListener* SGLSDLSoundPlayer::SetListener(SGLSoundPlayerListener* listener) {
    // Quiesce the previous listener before returning it to the owner. Refill
    // invokes Write/Stop without holding the SDL stream or player state lock.
    {
        ReleaseCallerLocks unlock;
        output_.SetRefillCallback(listener ? std::function<void()>([this, listener] { listener->OnStreaming(this); })
                                         : std::function<void()>{});
    }
    auto* previous = m_pListener;
    m_pListener = listener;
    return previous;
}

void SakuraGL::RegisterSDLSoundPlayer() {
    SGLSoundPlayer::SetPlayerCreator(&NewPlayer, nullptr);
}
