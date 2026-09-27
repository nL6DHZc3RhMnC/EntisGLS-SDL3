#pragma once

#include <sakuragl/sakuragl.h>
#include <sakuragl/media/sgl_sound_player.h>
#include "platform/sdl/sdl_pcm_stream.h"

namespace SakuraGL {

class SGLSDLSoundPlayer final : public SGLSoundPlayerInterface {
public:
    ESL_DECLARE_CLASS_INFO(SGLSDLSoundPlayer, SGLSoundPlayerInterface)
    ~SGLSDLSoundPlayer() override;
    SGLError Open(const SGLSoundFormat& format) override;
    SGLError Close() override;
    SGLError WriteStatic(const void* data, size_t bytes) override;
    SGLError PrepareStream(size_t bytes = 0) override;
    size_t Write(const void* data, size_t bytes) override;
    SGLError Play(uint64_t flags = 0) override;
    SGLError Stop() override;
    SGLError Pause() override;
    SGLError Restart() override;
    SGLError GetVolume(float32_t* volumes, size_t channels) override;
    SGLError SetVolume(const float32_t* volumes, size_t channels) override;
    bool IsPlaying() const override;
    bool IsPaused() const override;
    uint64_t GetPlayingPosition() override;
    SGLError SeekPosition(uint64_t byteOffset) override;
    SGLSoundPlayerListener* SetListener(SGLSoundPlayerListener* listener) override;

private:
    studysteady::platform::SdlPcmStream output_;
};

// Call after SSystem initialization, before creating any game sound players.
void RegisterSDLSoundPlayer();

} // namespace SakuraGL
