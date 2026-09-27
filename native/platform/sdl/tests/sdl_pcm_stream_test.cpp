#include "../sdl_pcm_stream.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

using studysteady::platform::SdlPcmStream;

namespace {
void Require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(std::string(message) + ": " + SDL_GetError());
}

template<class Predicate> void Wait(Predicate predicate, const char* message) {
    const auto until = SDL_GetTicks() + 3000;
    while (!predicate() && SDL_GetTicks() < until) SDL_Delay(2);
    Require(predicate(), message);
}

std::vector<unsigned char> Stereo(std::size_t frames) {
    std::vector<unsigned char> data(frames * 4);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        // Signed 16-bit little endian: +0.5 left, -0.25 right.
        data[frame * 4 + 0] = 0;
        data[frame * 4 + 1] = 0x40;
        data[frame * 4 + 2] = 0;
        data[frame * 4 + 3] = 0xe0;
    }
    return data;
}

struct OutputCapture {
    std::mutex mutex;
    double leftEnergy = 0, rightEnergy = 0;
    float leftPeak = 0;
    unsigned nonzeroFrames = 0;
    static void SDLCALL Mix(void* opaque, const SDL_AudioSpec* spec, float* buffer, int bytes) {
        auto& self = *static_cast<OutputCapture*>(opaque);
        std::lock_guard<std::mutex> lock(self.mutex);
        if (spec->channels != 2) return;
        for (int i = 0; i + 1 < bytes / int(sizeof(float)); i += 2) {
            self.leftEnergy += std::abs(buffer[i]);
            self.rightEnergy += std::abs(buffer[i + 1]);
            self.leftPeak = std::max(self.leftPeak, std::abs(buffer[i]));
            if (buffer[i] != 0) ++self.nonzeroFrames;
        }
    }
};
}

int main() {
    try {
        // Uses SDL's real stream conversion, device scheduling, and postmix
        // path without making noise or requiring a physical output device.
        Require(SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy"), "select dummy device");
        Require(SDL_Init(0), "initialize SDL");
        {
            bool finalized = false;
            std::thread::id finalizerThread;
            const auto caller = std::this_thread::get_id();
            SdlPcmStream lifetime;
            Require(lifetime.Open({SdlPcmStream::SampleType::signed16, 48000, 1}, [&] {
                finalizerThread = std::this_thread::get_id(); finalized = true;
            }), "open stream with worker finalizer");
            Require(lifetime.Close(), "close stream with worker finalizer");
            Require(finalized && finalizerThread != caller, "worker cleanup must finish on feeder before Close returns");
            std::cout << "worker finalizer and TLS cleanup boundary: PASS\n";
        }
        SdlPcmStream player;
        const SdlPcmStream::Format format{SdlPcmStream::SampleType::signed16, 48000, 2};
        Require(!player.Open({SdlPcmStream::SampleType::signed16, 48000, 3}), "reject unsupported channels");
        Require(player.Open(format), "open stereo PCM output");

        OutputCapture capture;
        Require(SDL_SetAudioPostmixCallback(player.GetDeviceId(), &OutputCapture::Mix, &capture), "capture device output");
        const float gains[] = {0.5f, 0.0f};
        Require(player.SetVolume(gains, 2), "set independent channel gains");
        auto shortData = Stereo(8000);
        Require(!player.WriteStatic(shortData.data(), shortData.size() - 1), "reject split sample frame");
        Require(player.WriteStatic(shortData.data(), shortData.size()), "queue static PCM");
        Require(player.Play(false), "play static PCM");
        Wait([&] { return !player.IsPlaying(); }, "static PCM must end");
        Require(player.GetPlayingPosition() == 8000, "static position must count actual frames");
        {
            std::lock_guard<std::mutex> lock(capture.mutex);
            Require(capture.nonzeroFrames >= 7000 && capture.leftEnergy > 1000, "device must receive audible PCM samples");
            Require(capture.rightEnergy == 0, "right-channel mute must reach device output");
            Require(std::abs(capture.leftPeak - 0.25f) < 0.0001f, "left gain must preserve expected sample values");
        }
        Require(SDL_SetAudioPostmixCallback(player.GetDeviceId(), nullptr, nullptr), "remove output capture");
        std::cout << "static output, stereo gains, device consumption: PASS\n";

        auto longData = Stereo(48000);
        Require(player.WriteStatic(longData.data(), longData.size()), "load pause test PCM");
        Require(player.Play(false), "start pause test");
        Wait([&] { return player.GetPlayingPosition() >= 2400; }, "audio must advance before pause");
        Require(player.Pause() && player.Pause(), "nested pauses");
        const auto paused = player.GetPlayingPosition();
        SDL_Delay(70);
        Require(player.GetPlayingPosition() == paused, "pause must freeze consumed position");
        Require(player.Restart() && player.IsPaused(), "one restart must preserve remaining pause");
        Require(player.SeekPosition(6000 * 4), "seek static PCM while paused");
        Require(player.GetPlayingPosition() == 6000, "seek takes bytes, reports frames");
        Require(player.Restart() && !player.IsPaused(), "balanced restart resumes");
        Wait([&] { return player.GetPlayingPosition() > 6000; }, "resumed audio must consume data");
        Require(player.Stop() && !player.IsPlaying(), "stop playback");
        std::cout << "pause nesting, seek units, restart: PASS\n";

        auto loopData = Stereo(1200);
        Require(player.WriteStatic(loopData.data(), loopData.size()), "load loop PCM");
        Require(player.Play(true), "play loop PCM");
        Wait([&] { return player.GetPlayingPosition() >= 6000; }, "loop must consume more than one source buffer");
        Require(player.IsPlaying() && player.Stop(), "loop stays active until stopped");
        std::cout << "static looping and cumulative playback position: PASS\n";

        using studysteady::platform::GetSdlMasterVolume;
        using studysteady::platform::SetSdlMasterVolume;
        float master = 0;
        Require(GetSdlMasterVolume(master) && master == 1.0f, "initial application master gain");
        Require(!SetSdlMasterVolume(-1) && !SetSdlMasterVolume(2), "invalid master gain must fail");
        Require(player.WriteStatic(longData.data(), longData.size()) && player.Play(true), "start existing master-controlled player");
        Wait([&] { return player.GetPlayingPosition() >= 2400; }, "existing player must advance before master update");
        Require(SetSdlMasterVolume(0.25f) && GetSdlMasterVolume(master) && master == 0.25f, "set and verify every active stream gain");
        SDL_Delay(50);
        OutputCapture existingMaster;
        Require(SDL_SetAudioPostmixCallback(player.GetDeviceId(), &OutputCapture::Mix, &existingMaster), "capture updated existing output");
        SDL_Delay(80);
        {
            std::lock_guard<std::mutex> lock(existingMaster.mutex);
            Require(existingMaster.nonzeroFrames > 0 && std::abs(existingMaster.leftPeak - 0.0625f) < 0.0001f,
                    "master change must multiply existing player channel gain in actual output");
        }
        Require(SDL_SetAudioPostmixCallback(player.GetDeviceId(), nullptr, nullptr), "remove existing output capture");

        SdlPcmStream futurePlayer;
        Require(futurePlayer.Open(format), "open future master-controlled player");
        OutputCapture futureMaster;
        Require(SDL_SetAudioPostmixCallback(futurePlayer.GetDeviceId(), &OutputCapture::Mix, &futureMaster), "capture future output");
        Require(futurePlayer.WriteStatic(shortData.data(), shortData.size()) && futurePlayer.Play(false), "play future stream at stored master gain");
        Wait([&] { return !futurePlayer.IsPlaying(); }, "future player must complete");
        {
            std::lock_guard<std::mutex> lock(futureMaster.mutex);
            Require(futureMaster.nonzeroFrames > 0 && std::abs(futureMaster.leftPeak - 0.125f) < 0.0001f,
                    "new players must inherit master gain in actual output");
        }
        Require(SDL_SetAudioPostmixCallback(futurePlayer.GetDeviceId(), nullptr, nullptr), "remove future output capture");
        Require(futurePlayer.Close() && player.Stop() && SetSdlMasterVolume(1), "restore master and player states");
        Require(GetSdlMasterVolume(master) && master == 1, "master restore must update all remaining outputs");
        std::cout << "application master gain on already-playing and future streams: PASS\n";

        Require(player.PrepareStream(1600 * 4), "prepare bounded streaming buffer");
        Require(!player.SeekPosition(0), "stream seeks must fail explicitly");
        Require(!player.Play(true), "decoder-owned streaming loop must not be misrepresented as static loop");
        const auto streamingData = Stereo(7000);
        std::size_t accepted = 0;
        std::atomic<unsigned> callbacks{0};
        std::atomic<bool> stoppedAtEnd{false};
        player.SetRefillCallback([&] {
            ++callbacks;
            if (accepted < streamingData.size()) {
                const auto remaining = streamingData.size() - accepted;
                const auto written = player.Write(streamingData.data() + accepted, remaining);
                Require(written <= remaining && written % 4 == 0, "stream accepts whole frames only");
                accepted += written;
            } else if (player.GetPlayingPosition() >= 7000) {
                // Matches SGLAudioDecodingPlayer::OnStreaming at decoder EOF.
                stoppedAtEnd = player.Stop();
            }
        });
        Require(player.Play(false), "play streaming PCM");
        Wait([&] { return stoppedAtEnd.load(); }, "EOF callback must stop without self-join/deadlock");
        player.SetRefillCallback({});
        Require(accepted == streamingData.size() && callbacks >= 5, "partial writes must preserve all streaming bytes");
        Require(player.GetPlayingPosition() == 7000, "streaming EOF must drain actual frames");
        std::cout << "partial streaming writes, tail drain, callback Stop reentry: PASS\n";

        Require(player.PrepareStream(), "prepare listener lifetime test");
        std::mutex callbackMutex;
        std::condition_variable callbackWake;
        bool entered = false, release = false;
        player.SetRefillCallback([&] {
            std::unique_lock<std::mutex> lock(callbackMutex);
            entered = true;
            callbackWake.notify_all();
            callbackWake.wait(lock, [&] { return release; });
        });
        Require(player.Play(false), "play listener lifetime test");
        {
            std::unique_lock<std::mutex> lock(callbackMutex);
            callbackWake.wait(lock, [&] { return entered; });
        }
        std::atomic<bool> closed{false};
        std::thread closer([&] { closed = player.Close(); });
        SDL_Delay(10);
        const bool closedEarly = closed;
        {
            std::lock_guard<std::mutex> lock(callbackMutex);
            release = true;
            callbackWake.notify_all();
        }
        closer.join();
        Require(!closedEarly && closed, "Close must quiesce in-flight callbacks");
        player.SetRefillCallback({});
        std::cout << "listener lifetime and close synchronization: PASS\n";

        Require(SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "studysteady-invalid-driver"), "select absent output backend");
        Require(!player.Open(format), "unavailable output device must fail instead of silent success");
        player.Close();
        SDL_Quit();
        std::cout << "device failure propagation: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
