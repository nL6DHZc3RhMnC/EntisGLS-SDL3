#include "../../legacy_device_volume.h"
#include "sdl_pcm_stream.h"

#include <algorithm>
#include <cmath>

bool LegacyGetDeviceVolume(double& volume) {
    float value = 0;
    if (!studysteady::platform::GetSdlMasterVolume(value)) return false;
    volume = value;
    return true;
}

bool LegacySetDeviceVolume(double volume) {
    if (!std::isfinite(volume)) return false;
    // Keep the legacy Android setter's clamp contract, while the portable
    // internal master API rejects invalid ranges to catch misuse elsewhere.
    return studysteady::platform::SetSdlMasterVolume(float(std::clamp(volume, 0.0, 1.0)));
}
