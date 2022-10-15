#ifndef ZELDA3_AUDIO_H_
#define ZELDA3_AUDIO_H_

#include "types.h"

// Things for msu
bool ZeldaIsPlayingMusicTrack(uint8 track);
bool ZeldaIsPlayingMusicTrackWithBug(uint8 track);
void ZeldaPlayMsuAudioTrack(uint8 track);
bool ZeldaIsMusicPlaying();

void ZeldaEnableMsu(uint8 enable);

void ZeldaRenderAudio(int16 *audio_buffer, int samples, int channels);
void ZeldaDiscardUnusedAudioFrames();
void ZeldaRestoreMusicAfterLoad_Locked(bool is_reset);
void ZeldaSaveMusicStateToRam_Locked();
void ZeldaPushApuState();

#endif  // ZELDA3_AUDIO_H_
