#pragma once

// The legacy ptfDevice control maps to Android's media-stream volume in the
// original JNI backend and to this application's master PCM gain in SDL builds.
// SDL never changes the operating-system-wide volume. Values are normalized.
bool LegacyGetDeviceVolume(double& volume);
bool LegacySetDeviceVolume(double volume);
