#ifndef VOLUME_CONTROL_H
#define VOLUME_CONTROL_H

#include <stdbool.h>

int GetApplicationVolume();
bool SetApplicationVolume(int volume_level);
bool SetApplicationMuted(bool muted);

#endif // VOLUME_CONTROL_H
