#pragma once

#include <SDL3/SDL_audio.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace studysteady::platform {

// Application master gain, multiplied with each player's channel volumes.
// Applies to all currently open streams and is inherited by later players;
// this does not change an operating-system or hardware volume setting.
bool GetSdlMasterVolume(float& volume);
bool SetSdlMasterVolume(float volume);

// Device-facing PCM output, independent of the SDK and its decoder classes.
// Input PCM is little endian, matching the game archives. Positions count
// source sample frames consumed by SDL, not elapsed wall-clock time. The
// device's final hardware buffer adds its normal output latency.
class SdlPcmStream {
public:
    enum class SampleType { unsigned8, signed16, signed32, float32 };
    struct Format {
        SampleType type = SampleType::signed16;
        unsigned frequency = 0;
        unsigned channels = 0;
    };

    SdlPcmStream();
    ~SdlPcmStream();
    SdlPcmStream(const SdlPcmStream&) = delete;
    SdlPcmStream& operator=(const SdlPcmStream&) = delete;

    // Optional SDK/host cleanup runs once on the feeder thread after its last
    // callback, before Close's join returns. The PCM layer itself is SDK-free.
    bool Open(Format format, std::function<void()> workerFinalizer = {});
    // Close and callback replacement quiesce an in-flight refill callback.
    // The caller must not hold locks needed by that callback. Close cannot be
    // called by the refill callback itself; Stop can, including at decoder EOF.
    bool Close();
    bool WriteStatic(const void* data, std::size_t bytes);
    bool PrepareStream(std::size_t bufferBytes = 0);
    std::size_t Write(const void* data, std::size_t bytes);
    bool Play(bool loop);
    bool Stop();
    bool Pause();
    bool Restart();
    bool SetVolume(const float* volumes, std::size_t channels);
    bool GetVolume(float* volumes, std::size_t channels) const;
    bool IsPlaying() const;
    bool IsPaused() const;
    std::uint64_t GetPlayingPosition() const;
    bool SeekPosition(std::uint64_t byteOffset);
    void SetRefillCallback(std::function<void()> callback);
    SDL_AudioDeviceID GetDeviceId() const;

private:
    struct State;
    std::unique_ptr<State> state_;
};

} // namespace studysteady::platform
