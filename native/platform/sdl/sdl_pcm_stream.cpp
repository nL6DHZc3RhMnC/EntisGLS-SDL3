#include "sdl_pcm_stream.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <limits>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace studysteady::platform {
namespace {
struct MasterVolume {
    std::mutex mutex;
    float volume = 1.0f;
    std::set<SDL_AudioStream*> streams;
};
MasterVolume& Master() {
    static MasterVolume master;
    return master;
}
bool RegisterMasterStream(SDL_AudioStream* stream) {
    auto& master = Master();
    std::lock_guard<std::mutex> lock(master.mutex);
    if (!SDL_SetAudioStreamGain(stream, master.volume)) return false;
    master.streams.insert(stream);
    return true;
}
void UnregisterMasterStream(SDL_AudioStream* stream) {
    auto& master = Master();
    std::lock_guard<std::mutex> lock(master.mutex);
    master.streams.erase(stream);
}
}

bool GetSdlMasterVolume(float& volume) {
    auto& master = Master();
    std::lock_guard<std::mutex> lock(master.mutex);
    for (auto* stream : master.streams) {
        const auto actual = SDL_GetAudioStreamGain(stream);
        if (!std::isfinite(actual) || actual < 0 || std::abs(actual - master.volume) > 0.000001f)
            return SDL_SetError("SDL application master gain cannot be verified on every active stream");
    }
    volume = master.volume;
    return true;
}

bool SetSdlMasterVolume(float volume) {
    if (!std::isfinite(volume) || volume < 0 || volume > 1)
        return SDL_SetError("SDL application master volume must be finite and within 0..1");
    auto& master = Master();
    std::lock_guard<std::mutex> lock(master.mutex);
    std::vector<std::pair<SDL_AudioStream*, float>> original;
    original.reserve(master.streams.size());
    for (auto* stream : master.streams) {
        const auto actual = SDL_GetAudioStreamGain(stream);
        if (!std::isfinite(actual) || actual < 0) return false;
        original.emplace_back(stream, actual);
    }
    for (std::size_t i = 0; i < original.size(); ++i) {
        if (!SDL_SetAudioStreamGain(original[i].first, volume)) {
            // Do not publish a master value unless every output accepted it.
            // Rollback failure remains observable in the getter consistency
            // check instead of pretending the devices all share one value.
            for (std::size_t previous = 0; previous < i; ++previous)
                if (!SDL_SetAudioStreamGain(original[previous].first, original[previous].second))
                    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "StudySteady master volume rollback: %s", SDL_GetError());
            return false;
        }
    }
    master.volume = volume;
    return true;
}

struct SdlPcmStream::State {
    mutable std::mutex mutex;
    std::condition_variable wake;
    std::thread worker;
    SDL_AudioStream* stream = nullptr;
    Format format;
    std::size_t sampleBytes = 0, frameBytes = 0, queueFrames = 0;
    std::uint64_t submittedFrames = 0, positionBase = 0, stoppedPosition = 0;
    std::uint64_t generation = 0, callbackGeneration = 0;
    std::vector<float> staticData, scratch;
    std::size_t staticCursor = 0, staticStart = 0;
    std::array<float, 2> volume{1.0f, 1.0f};
    std::function<void()> refill;
    bool ownsAudio = false, closing = false, failed = false;
    bool staticMode = false, prepared = false, playing = false, loop = false;
    bool inCallback = false, flushed = false;
    unsigned pauseCount = 0;

    bool OnWorker() const { return worker.joinable() && worker.get_id() == std::this_thread::get_id(); }

    bool Fail(const char* operation) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "StudySteady SDL audio %s: %s", operation, SDL_GetError());
        failed = true;
        playing = false;
        wake.notify_all();
        return false;
    }

    std::int64_t QueuedFrames() const {
        const int bytes = SDL_GetAudioStreamQueued(stream);
        return bytes < 0 ? -1 : bytes / (sizeof(float) * format.channels);
    }

    std::uint64_t Position() const {
        if (!stream || !playing) return stoppedPosition;
        const auto queued = QueuedFrames();
        if (queued < 0) return stoppedPosition;
        return positionBase + submittedFrames - std::min(submittedFrames, std::uint64_t(queued));
    }

    bool ResetQueue() {
        if (!SDL_ClearAudioStream(stream)) return Fail("clear stream");
        submittedFrames = 0;
        flushed = false;
        return true;
    }

    bool Submit(const float* data, std::size_t frames) {
        if (!frames) return true;
        scratch.resize(frames * format.channels);
        for (std::size_t i = 0; i < scratch.size(); ++i)
            scratch[i] = data[i] * volume[i % format.channels];
        if (!SDL_PutAudioStreamData(stream, scratch.data(), int(scratch.size() * sizeof(float))))
            return Fail("queue PCM");
        submittedFrames += frames;
        flushed = false;
        return true;
    }

    bool Convert(const void* data, std::size_t bytes, std::vector<float>& output) const {
        if (!data || bytes % frameBytes) return false;
        const auto* src = static_cast<const unsigned char*>(data);
        output.resize(bytes / sampleBytes);
        for (std::size_t i = 0; i < output.size(); ++i, src += sampleBytes) {
            if (format.type == SampleType::unsigned8) {
                output[i] = (int(src[0]) - 128) / 128.0f;
            } else if (format.type == SampleType::signed16) {
                const unsigned value = unsigned(src[0]) | (unsigned(src[1]) << 8);
                output[i] = (int(value) - ((value & 0x8000) ? 0x10000 : 0)) / 32768.0f;
            } else {
                const std::uint32_t value = std::uint32_t(src[0]) | (std::uint32_t(src[1]) << 8)
                    | (std::uint32_t(src[2]) << 16) | (std::uint32_t(src[3]) << 24);
                if (format.type == SampleType::signed32) {
                    const auto signedValue = std::int64_t(value) - ((value & 0x80000000u) ? 0x100000000LL : 0);
                    output[i] = float(double(signedValue) / 2147483648.0);
                } else {
                    std::memcpy(&output[i], &value, sizeof(float));
                    if (!std::isfinite(output[i])) return false;
                }
            }
        }
        return true;
    }

    bool FeedStatic() {
        auto queued = QueuedFrames();
        if (queued < 0) return Fail("query stream queue");
        const auto length = staticData.size() / format.channels;
        while (std::size_t(queued) < queueFrames && length) {
            if (staticCursor == length) {
                if (!loop) break;
                staticCursor = staticStart;
            }
            const auto count = std::min(queueFrames - std::size_t(queued), length - staticCursor);
            if (!count) break;
            if (!Submit(staticData.data() + staticCursor * format.channels, count)) return false;
            staticCursor += count;
            queued += count;
        }
        if (!loop && staticCursor == length && !flushed) {
            if (!SDL_FlushAudioStream(stream)) return Fail("flush static PCM");
            flushed = true;
        }
        return true;
    }

    void Run() {
        std::unique_lock<std::mutex> lock(mutex);
        while (!closing) {
            wake.wait_for(lock, std::chrono::milliseconds(2));
            if (closing) break;
            if (!playing || pauseCount || failed) continue;
            if (staticMode) {
                if (!FeedStatic()) continue;
                const auto queued = QueuedFrames();
                if (queued < 0) { Fail("query drained PCM"); continue; }
                if (!loop && staticCursor == staticData.size() / format.channels && queued == 0) {
                    stoppedPosition = positionBase + submittedFrames;
                    playing = false;
                    if (!SDL_PauseAudioStreamDevice(stream)) Fail("pause drained stream");
                }
                continue;
            }
            const auto queued = QueuedFrames();
            if (queued < 0) { Fail("query streaming PCM"); continue; }
            if (!refill || std::size_t(queued) >= queueFrames) continue;
            const auto callback = refill;
            const auto before = submittedFrames;
            callbackGeneration = generation;
            inCallback = true;
            lock.unlock();
            bool callbackFailed = false;
            try { callback(); }
            catch (...) { callbackFailed = true; }
            lock.lock();
            inCallback = false;
            wake.notify_all();
            if (callbackFailed) {
                SDL_SetError("exception in decoder refill callback");
                Fail("refill PCM");
            }
            // The listener has no EOF return value. A callback that writes no
            // data may be a temporary movie underrun or decoder EOF. Flush the
            // tail so the real consumed position reaches EOF; keep requesting
            // refills, allowing the decoder to call Stop once it has drained.
            if (playing && !closing && !failed && generation == callbackGeneration
                && submittedFrames == before && !flushed) {
                if (!SDL_FlushAudioStream(stream)) Fail("flush streaming tail");
                else flushed = true;
            }
        }
    }
};

SdlPcmStream::SdlPcmStream() : state_(std::make_unique<State>()) {}
SdlPcmStream::~SdlPcmStream() { Close(); }

bool SdlPcmStream::Open(Format format, std::function<void()> workerFinalizer) {
    if (!format.frequency || format.frequency > 384000 || !format.channels || format.channels > 2)
        return false;
    if (!Close()) return false;
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return s.Fail("initialize audio");
    s.ownsAudio = true;
    SDL_AudioSpec spec{SDL_AUDIO_F32, int(format.channels), int(format.frequency)};
    s.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!s.stream) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        s.ownsAudio = false;
        return s.Fail("open playback device");
    }
    if (!RegisterMasterStream(s.stream)) {
        SDL_DestroyAudioStream(s.stream);
        s.stream = nullptr;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        s.ownsAudio = false;
        return s.Fail("apply application master volume");
    }
    s.format = format;
    s.sampleBytes = format.type == SampleType::unsigned8 ? 1 : format.type == SampleType::signed16 ? 2 : 4;
    s.frameBytes = s.sampleBytes * format.channels;
    s.queueFrames = std::max<std::size_t>(1, format.frequency / 20); // At most 50 ms gain-update latency.
    s.closing = s.failed = s.prepared = s.playing = s.staticMode = s.flushed = false;
    s.submittedFrames = s.positionBase = s.stoppedPosition = 0;
    s.pauseCount = 0;
    try { s.worker = std::thread([&s, finalizer = std::move(workerFinalizer)] {
        struct FinalizeWorker {
            const std::function<void()>& cleanup;
            ~FinalizeWorker() {
                try { if (cleanup) cleanup(); }
                catch (...) { SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "StudySteady audio worker cleanup threw an exception"); }
            }
        } finish{finalizer};
        try { s.Run(); }
        catch (...) {
            std::lock_guard<std::mutex> lock(s.mutex);
            SDL_SetError("exception in PCM feeder thread"); s.Fail("feeder thread");
        }
    }); }
    catch (...) {
        UnregisterMasterStream(s.stream);
        SDL_DestroyAudioStream(s.stream);
        s.stream = nullptr;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        s.ownsAudio = false;
        SDL_SetError("cannot start PCM refill thread");
        return s.Fail("create refill thread");
    }
    return true;
}

bool SdlPcmStream::Close() {
    auto& s = *state_;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        if (s.OnWorker()) return false;
        s.closing = true;
        s.playing = false;
        ++s.generation;
        s.wake.notify_all();
    }
    if (s.worker.joinable()) s.worker.join();
    std::lock_guard<std::mutex> lock(s.mutex);
    if (s.stream) {
        UnregisterMasterStream(s.stream);
        SDL_DestroyAudioStream(s.stream);
    }
    s.stream = nullptr;
    if (s.ownsAudio) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    s.ownsAudio = false;
    s.staticData.clear();
    s.scratch.clear();
    s.prepared = false;
    s.pauseCount = 0;
    return true;
}

bool SdlPcmStream::WriteStatic(const void* data, std::size_t bytes) {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || s.playing || s.closing || s.failed || !bytes || bytes % s.frameBytes) return false;
    std::vector<float> converted;
    if (!s.Convert(data, bytes, converted) || !s.ResetQueue()) return false;
    s.staticData = std::move(converted);
    s.staticCursor = s.staticStart = 0;
    s.positionBase = s.stoppedPosition = 0;
    s.staticMode = s.prepared = true;
    return true;
}

bool SdlPcmStream::PrepareStream(std::size_t bufferBytes) {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || s.playing || s.closing || s.failed || bufferBytes % s.frameBytes) return false;
    if (!s.ResetQueue()) return false;
    if (bufferBytes) {
        // Keep individual SDL queue operations below INT_MAX and avoid a
        // malformed script allocating an unbounded device queue.
        const auto requested = bufferBytes / s.frameBytes;
        if (requested > std::size_t(s.format.frequency) * 2) return false;
        s.queueFrames = requested;
    }
    s.staticData.clear();
    s.staticMode = false;
    s.prepared = true;
    s.positionBase = s.stoppedPosition = 0;
    return true;
}

std::size_t SdlPcmStream::Write(const void* data, std::size_t bytes) {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || !s.prepared || s.staticMode || s.closing || s.failed || !data || bytes % s.frameBytes
        || (s.OnWorker() && s.inCallback && s.callbackGeneration != s.generation)) return 0;
    const auto queued = s.QueuedFrames();
    if (queued < 0) { s.Fail("query streaming capacity"); return 0; }
    if (std::size_t(queued) >= s.queueFrames) return 0;
    const auto frames = std::min(bytes / s.frameBytes, s.queueFrames - std::size_t(queued));
    std::vector<float> converted;
    if (!frames || !s.Convert(data, frames * s.frameBytes, converted) || !s.Submit(converted.data(), frames)) return 0;
    s.wake.notify_all();
    return frames * s.frameBytes;
}

bool SdlPcmStream::Play(bool loop) {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || !s.prepared || s.playing || s.closing || s.failed || (loop && !s.staticMode)) return false;
    s.loop = loop;
    s.pauseCount = 0;
    if (s.staticMode) {
        if (!s.ResetQueue()) return false;
        s.staticCursor = s.staticStart;
        s.positionBase = s.staticStart;
        s.stoppedPosition = s.positionBase;
        if (!s.FeedStatic()) return false;
    }
    if (!SDL_ResumeAudioStreamDevice(s.stream)) return s.Fail("resume playback");
    s.playing = true;
    s.wake.notify_all();
    return true;
}

bool SdlPcmStream::Stop() {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || s.closing) return false;
    if (!SDL_PauseAudioStreamDevice(s.stream)) return s.Fail("stop playback");
    s.stoppedPosition = s.Position();
    s.playing = false;
    s.pauseCount = 0;
    ++s.generation;
    if (!s.ResetQueue()) return false;
    // A new streaming Play begins at sample zero. The decoder maintains its
    // own seek/loop base independently of this consumed-frame counter.
    s.positionBase = 0;
    s.wake.notify_all();
    return true;
}

bool SdlPcmStream::Pause() {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || !s.playing || s.closing || s.failed || s.pauseCount == std::numeric_limits<unsigned>::max()) return false;
    if (!s.pauseCount && !SDL_PauseAudioStreamDevice(s.stream)) return s.Fail("pause playback");
    ++s.pauseCount;
    return true;
}

bool SdlPcmStream::Restart() {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || !s.playing || !s.pauseCount || s.closing || s.failed) return false;
    if (s.pauseCount == 1 && !SDL_ResumeAudioStreamDevice(s.stream)) return s.Fail("restart playback");
    --s.pauseCount;
    s.wake.notify_all();
    return true;
}

bool SdlPcmStream::SetVolume(const float* volumes, std::size_t channels) {
    if (!volumes || !channels) return false;
    const float left = volumes[0], right = channels == 1 ? left : volumes[1];
    if (!std::isfinite(left) || !std::isfinite(right)) return false;
    std::lock_guard<std::mutex> lock(state_->mutex);
    state_->volume = {std::clamp(left, 0.0f, 1.0f), std::clamp(right, 0.0f, 1.0f)};
    return true;
}

bool SdlPcmStream::GetVolume(float* volumes, std::size_t channels) const {
    if (!volumes || !channels) return false;
    std::lock_guard<std::mutex> lock(state_->mutex);
    for (std::size_t i = 0; i < channels; ++i) volumes[i] = i < 2 ? state_->volume[i] : 1.0f;
    return true;
}

bool SdlPcmStream::IsPlaying() const { std::lock_guard<std::mutex> lock(state_->mutex); return state_->playing && !state_->failed; }
bool SdlPcmStream::IsPaused() const { std::lock_guard<std::mutex> lock(state_->mutex); return state_->pauseCount != 0; }
std::uint64_t SdlPcmStream::GetPlayingPosition() const { std::lock_guard<std::mutex> lock(state_->mutex); return state_->Position(); }

bool SdlPcmStream::SeekPosition(std::uint64_t byteOffset) {
    auto& s = *state_;
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.stream || !s.prepared || !s.staticMode || s.closing || s.failed || byteOffset % s.frameBytes) return false;
    const auto frame = byteOffset / s.frameBytes;
    if (frame >= s.staticData.size() / s.format.channels) return false;
    if (!SDL_PauseAudioStreamDevice(s.stream)) return s.Fail("pause for seek");
    if (!s.ResetQueue()) return false;
    s.staticStart = s.staticCursor = std::size_t(frame);
    s.positionBase = s.stoppedPosition = frame;
    if (s.playing) {
        if (!s.FeedStatic()) return false;
        if (!s.pauseCount && !SDL_ResumeAudioStreamDevice(s.stream)) return s.Fail("resume after seek");
    }
    return true;
}

void SdlPcmStream::SetRefillCallback(std::function<void()> callback) {
    auto& s = *state_;
    std::unique_lock<std::mutex> lock(s.mutex);
    if (!s.OnWorker()) s.wake.wait(lock, [&s] { return !s.inCallback; });
    s.refill = std::move(callback);
    s.wake.notify_all();
}

SDL_AudioDeviceID SdlPcmStream::GetDeviceId() const {
    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->stream ? SDL_GetAudioStreamDevice(state_->stream) : 0;
}

} // namespace studysteady::platform
