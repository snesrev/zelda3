#ifndef ZELDA3_PLATFORM_WIN32_VOLUME_CONTROL_H_
#define ZELDA3_PLATFORM_WIN32_VOLUME_CONTROL_H_

#include <stdbool.h>

#ifndef SYSTEM_VOLUME_MIXER_AVAILABLE
#define SYSTEM_VOLUME_MIXER_AVAILABLE 1
#endif  // SYSTEM_VOLUME_MIXER_AVAILABLE

int GetApplicationVolume();
bool SetApplicationVolume(int volume_level);
bool SetApplicationMuted(bool muted);

#endif // ZELDA3_PLATFORM_WIN32_VOLUME_CONTROL_H_
