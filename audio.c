#include "audio.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "features.h"
#include "snes/snes_regs.h"
#include "spc_player.h"
#include "third_party/opus-1.3.1-stripped/opus.h"
#include "config.h"
#include "assets.h"

// This needs to hold a lot more things than with just PCM
typedef struct MsuPlayerResumeInfo {
  uint32 tag;
  uint32 offset;
  uint32 samples_until_repeat;
  uint16 range_cur, range_repeat;
  uint64 initial_packet_bytes;  // To verify we seeked right
  uint8 orig_track;        // Using the old zelda track numbers
  uint8 actual_track;      // The MSU track index we're playing (Different if using msu deluxe)
} MsuPlayerResumeInfo;

enum {
  kMsuState_Idle = 0,
  kMsuState_FinishedPlaying = 1,
  kMsuState_Resuming = 2,
  kMsuState_Playing = 3,
};

typedef struct MsuPlayer {
  FILE *f;
  uint32 buffer_size, buffer_pos;
  uint32 preskip, samples_until_repeat;
  uint32 total_samples_in_file, repeat_position;
  uint32 cur_file_offs;
  MsuPlayerResumeInfo resume_info;
  uint8 enabled;
  uint8 state;
  float volume, volume_step, volume_target;
  uint16 range_cur, range_repeat;
  OpusDecoder *opus;
  int16 buffer[960 * 2];
} MsuPlayer;

static MsuPlayer g_msu_player;

static void MsuPlayer_Open(MsuPlayer *mp, int orig_track, bool resume_from_snapshot);

static const uint8 kMsuTracksWithRepeat[48] = {
  1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,0,
  1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,
  1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

static const uint8 kIsMusicOwOrDungeon[] = {
  0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, // 1 = ow songs : 2 = lw, 5 = forest, 7 = town, 9 = dark world, 13 = mountain
  2, 2, 2, 0, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, // 2 = indoor songs : 16, 17, 18, 22, 23, 27
};


static const uint8 kMsuDeluxe_OW_Songs[] = {
  37, 37, 42, 38, 38, 38, 38, 39, 37, 37, 42, 38, 38, 38, 38, 41, // lw
  42, 42, 42, 42, 42, 42, 40, 40, 43, 43, 42, 47, 47, 42, 45, 45,
  43, 43, 43, 47, 47, 42, 45, 45, 112, 112, 48, 42, 42, 42, 42, 45,
  44, 44, 48, 48, 48, 46, 46, 46, 44, 44, 44, 48, 48, 46, 46, 46,
  49, 49, 51, 50, 50, 50, 50, 50, 49, 49, 51, 50, 50, 50, 50, 51, // dw
  51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 51, 56, 56, 51, 54, 54,
  52, 52, 52, 56, 56, 51, 54, 54, 58, 52, 57, 51, 51, 51, 51, 54,
  53, 53, 57, 57, 57, 55, 55, 110, 53, 53, 57, 57, 57, 55, 55, 110,
  37, 41, 41, 42, 42, 42, 42, 42, 42, 41, 41, 42, 42, 42, 42, 42, // special
  42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
};

static const uint8 kMsuDeluxe_Entrance_Songs[] = {
   59,  59,  60,  61,  61,  61,  62,  62,  63,  64,  64,  64, 105,  65,  65,  66,
   66,  62,  67,  62,  62,  68,  62,  62,  68,  68,  62,  62,  62,  62,  62,  62,
   62,  62,  62,  62,  69,  70,  71,  72,  73,  73,  73, 106, 102,  74,  62,  62,
   75,  75,  76,  77,  78,  68,  79,  80,  81,  62,  62,  62,  82,  75, 242,  59,
   59,  76, 242, 242, 242,  96,  83,  99,  59, 242, 242, 242,  84,  95, 104,  62,
   85,  62,  62,  86, 242,  67, 103,  83,  83,  87,  76,  88,  81,  98,  81,  88,
   83,  89,  75,  97,  90,  91,  91, 100,  92,  93,  92, 242,  93, 107,  62,  75,
   62,  67,  62, 242, 242, 242,  73,  73,  73,  73, 102, 114,  81,  76,  62,  67,
   62,  61,  94,  62, 103,
};


// Remap an track number into a potentially different track number (used for msu deluxe)
static uint8 RemapMsuDeluxeTrack(MsuPlayer *mp, uint8 track) {
  if (!(mp->enabled & kMsuEnabled_MsuDeluxe) || track >= sizeof(kIsMusicOwOrDungeon))
    return track;
  switch (kIsMusicOwOrDungeon[track]) {
  case 1:
    return BYTE(overworld_area_index) < sizeof(kMsuDeluxe_OW_Songs) ? kMsuDeluxe_OW_Songs[BYTE(overworld_area_index)] : track;
  case 2:
    if (which_entrance >= sizeof(kMsuDeluxe_Entrance_Songs) || kMsuDeluxe_Entrance_Songs[which_entrance] == 242)
      return track;
    return kMsuDeluxe_Entrance_Songs[which_entrance];
  default:
    return track;
  }
}

bool ZeldaIsPlayingMusicTrack(uint8 track) {
  MsuPlayer *mp = &g_msu_player;
  if (mp->state != kMsuState_Idle && mp->enabled & kMsuEnabled_MsuDeluxe)
    return RemapMsuDeluxeTrack(mp, track) == mp->resume_info.actual_track;
  else
    return track == music_unk1;
}

bool ZeldaIsPlayingMusicTrackWithBug(uint8 track) {
  MsuPlayer *mp = &g_msu_player;
  if (mp->state != kMsuState_Idle && mp->enabled & kMsuEnabled_MsuDeluxe)
    return RemapMsuDeluxeTrack(mp, track) == mp->resume_info.actual_track;
  else
    return track == (enhanced_features0 & kFeatures0_MiscBugFixes ? music_unk1 : last_music_control);
}

uint8 ZeldaGetEntranceMusicTrack(int i) {
  MsuPlayer *mp = &g_msu_player;
  uint8 rv = kEntranceData_musicTrack[i];

  // For some entrances the original performs a fade out, while msu deluxe has new tracks.
  if (mp->state != kMsuState_Idle && mp->enabled & kMsuEnabled_MsuDeluxe) {
    if (rv == 242 && kMsuDeluxe_Entrance_Songs[which_entrance] != 242)
      rv = 16;
  }

  return rv;
}

static const uint8 kVolumeTransitionTarget[4] = { 0, 64, 255, 255};
static const uint8 kVolumeTransitionStep[4] = { 7, 3, 3, 24};
// These are precomputed in the config parse
static float kVolumeTransitionStepFloat[4];
static float kVolumeTransitionTargetFloat[4];

void ZeldaPlayMsuAudioTrack(uint8 music_ctrl) {
  MsuPlayer *mp = &g_msu_player;
  if (!mp->enabled) {
    mp->resume_info.tag = 0;
    zelda_apu_write(APUI00, music_ctrl);
    return;
  }
  ZeldaApuLock();
  if ((music_ctrl & 0xf0) != 0xf0)
    MsuPlayer_Open(mp, music_ctrl, false);
  else if (music_ctrl >= 0xf1 && music_ctrl <= 0xf3) {
    mp->volume_target = kVolumeTransitionTargetFloat[music_ctrl - 0xf1];
    mp->volume_step = kVolumeTransitionStepFloat[music_ctrl - 0xf1];
  }

  if (mp->state == 0) {
    zelda_apu_write(APUI00, music_ctrl);
  } else {
    zelda_apu_write(APUI00, 0xf0);  // pause spc player
  }
  ZeldaApuUnlock();
}

static void MsuPlayer_CloseFile(MsuPlayer *mp) {
  if (mp->f)
    fclose(mp->f);
  opus_decoder_destroy(mp->opus);
  mp->opus = NULL;
  mp->f = NULL;
  if (mp->state != kMsuState_FinishedPlaying)
    mp->state = kMsuState_Idle;
  memset(&mp->resume_info, 0, sizeof(mp->resume_info));
}

static void MsuPlayer_Open(MsuPlayer *mp, int orig_track, bool resume_from_snapshot) {
  MsuPlayerResumeInfo resume;
  int actual_track = RemapMsuDeluxeTrack(mp, orig_track);

  if (!resume_from_snapshot) {
    resume.tag = 0;
    // Attempt to resume MSU playback when exiting back to the overworld.
    if (main_module_index == 9 &&
        actual_track == ((MsuPlayerResumeInfo *)msu_resume_info_alt)->actual_track && g_config.resume_msu) {
      memcpy(&resume, msu_resume_info_alt, sizeof(mp->resume_info));
    }
    if (mp->state >= kMsuState_Resuming)
      memcpy(msu_resume_info_alt, &mp->resume_info, sizeof(mp->resume_info));
  } else {
    memcpy(&resume, msu_resume_info, sizeof(mp->resume_info));
  }

  mp->volume_target = kVolumeTransitionTargetFloat[3];
  mp->volume_step = kVolumeTransitionStepFloat[3];

  mp->state = kMsuState_Idle;
  MsuPlayer_CloseFile(mp);
  if (actual_track == 0)
    return;
  char fname[256], buf[8];
  snprintf(fname, sizeof(fname), "%s%d.%s", g_config.msu_path ? g_config.msu_path : "", actual_track, mp->enabled & kMsuEnabled_Opuz ? "opuz" : "pcm");
  printf("Loading MSU %s\n", fname);
  mp->f = fopen(fname, "rb");
  if (mp->f == NULL)
    goto READ_ERROR;
  setvbuf(mp->f, NULL, _IOFBF, 16384);
  if (fread(buf, 1, 8, mp->f) != 8) READ_ERROR: {
    fprintf(stderr, "Unable to read MSU file %s\n", fname);
    MsuPlayer_CloseFile(mp);
    return;
  }
  uint32 file_tag = *(uint32 *)(buf + 0);
  mp->repeat_position = *(uint32 *)(buf + 4);
  mp->state = (resume.actual_track == actual_track && resume.tag == file_tag) ? kMsuState_Resuming : kMsuState_Playing;
  if (mp->state == kMsuState_Resuming) {
    memcpy(&mp->resume_info, &resume, sizeof(mp->resume_info));
  } else {
    mp->resume_info.orig_track = orig_track;
    mp->resume_info.actual_track = actual_track;
    mp->resume_info.tag = file_tag;
    mp->resume_info.range_cur = 8;
  }
  mp->cur_file_offs = mp->resume_info.offset;
  mp->samples_until_repeat = mp->resume_info.samples_until_repeat;
  mp->range_cur = mp->resume_info.range_cur;
  mp->range_repeat = mp->resume_info.range_repeat;
  mp->buffer_size = mp->buffer_pos = 0;
  mp->preskip = 0;
  if (file_tag == (('Z' << 24) | ('U' << 16) | ('P' << 8) | 'O')) {
    mp->opus = opus_decoder_create(48000, 2, NULL);
    if (!mp->opus)
      goto READ_ERROR;
    if (mp->state == kMsuState_Resuming)
      fseek(mp->f, mp->cur_file_offs, SEEK_SET);
  } else if (file_tag == (('1' << 24) | ('U' << 16) | ('S' << 8) | 'M')) {
    fseek(mp->f, 0, SEEK_END);
    mp->total_samples_in_file = (ftell(mp->f) - 8) / 4;
    mp->samples_until_repeat = mp->total_samples_in_file - mp->cur_file_offs;
    fseek(mp->f, mp->cur_file_offs * 4 + 8, SEEK_SET);
  } else {
    goto READ_ERROR;
  }
}

static void MixToBufferWithVolume(int16 *dst, const int16 *src, size_t n, float volume) {
  if (volume == 1.0f) {
    for (size_t i = 0; i < n; i++) {
      dst[i * 2 + 0] += src[i * 2 + 0];
      dst[i * 2 + 1] += src[i * 2 + 1];
    }
  } else {
    uint32 vol = (int32)(65536 * volume);
    for (size_t i = 0; i < n; i++) {
      dst[i * 2 + 0] += src[i * 2 + 0] * vol >> 16;
      dst[i * 2 + 1] += src[i * 2 + 1] * vol >> 16;
    }
  }
}

static void MixToBufferWithVolumeRamp(int16 *dst, const int16 *src, size_t n, float volume, float volume_step, float ideal_target) {
  int64 vol = volume * 281474976710656.0f;
  int64 step = volume_step * 281474976710656.0f;
  for (size_t i = 0; i < n; i++) {
    uint32 v = (vol >> 32);
    dst[i * 2 + 0] += src[i * 2 + 0] * v >> 16;
    dst[i * 2 + 1] += src[i * 2 + 1] * v >> 16;
    vol += step;
  }
}

static void MixToBuffer(MsuPlayer *mp, int16 *dst, const int16 *src, uint32 n) {
  if (mp->volume != mp->volume_target) {
    float step = mp->volume < mp->volume_target ? mp->volume_step : -mp->volume_step;
    float new_vol = mp->volume + step * n;
    uint32 curn = n;
    if (step >= 0 ? new_vol >= mp->volume_target : new_vol < mp->volume_target) {
      uint32 maxn = (uint32)((mp->volume_target - mp->volume) / step);
      curn = UintMin(maxn, curn);
      new_vol = mp->volume_target;
    }
    float vol = mp->volume;
    mp->volume = new_vol;
    MixToBufferWithVolumeRamp(dst, src, curn, vol, step, new_vol);
    dst += curn * 2, src += curn * 2, n -= curn;
  }
  MixToBufferWithVolume(dst, src, n, mp->volume);
}

void MsuPlayer_Mix(MsuPlayer *mp, int16 *audio_buffer, int audio_samples) {
  int r;

  do {
    if (mp->buffer_size - mp->buffer_pos == 0) {
      if (mp->opus != NULL) {
        if (mp->samples_until_repeat == 0) {
          if (mp->range_cur == 0) FINISHED_PLAYING: {
            mp->state = kMsuState_FinishedPlaying;
            MsuPlayer_CloseFile(mp);
            return;
          }
          opus_decoder_ctl(mp->opus, OPUS_RESET_STATE);
          fseek(mp->f, mp->range_cur, SEEK_SET);
          uint8 *file_data = (uint8 *)mp->buffer;
          if (fread(file_data, 1, 10, mp->f) != 10) READ_ERROR: {
            fprintf(stderr, "MSU read/decode error!\n");
            zelda_apu_write(APUI00, mp->resume_info.orig_track);
            MsuPlayer_CloseFile(mp);
            return;
          }
          uint32 file_offs = *(uint32 *)&file_data[0];
          assert((file_offs & 0xF0000000) == 0);
          uint32 samples_until_repeat = *(uint32 *)&file_data[4];
          uint16 preskip = *(uint32 *)&file_data[8];
          mp->samples_until_repeat = samples_until_repeat;
          mp->preskip = preskip & 0x3fff;
          if (preskip & 0x4000)
            mp->range_repeat = mp->range_cur;
          mp->range_cur = (preskip & 0x8000) ? mp->range_repeat : mp->range_cur + 10;
          mp->cur_file_offs = file_offs;
          mp->resume_info.range_repeat = mp->range_repeat;
          mp->resume_info.range_cur = mp->range_cur;
          fseek(mp->f, file_offs, SEEK_SET);
        }
        assert(mp->samples_until_repeat != 0);
        for (;;) {
          uint8 *file_data = (uint8 *)mp->buffer;
          *(uint64 *)file_data = 0;
          if (fread(file_data, 1, 2, mp->f) != 2)
            goto READ_ERROR;
          int size = *(uint16 *)file_data & 0x7fff;
          if (size > 1275)
            goto READ_ERROR;
          int n = (*(uint16 *)file_data >> 15);
          if (fread(&file_data[2], 1, size, mp->f) != size)
            goto READ_ERROR;
          // Verify if the snapshot matches the file on disk.
          uint64 initial_file_data = *(uint64 *)file_data;
          if (mp->state == kMsuState_Resuming) {
            mp->state = kMsuState_Playing;
            if (mp->resume_info.initial_packet_bytes != initial_file_data)
              goto READ_ERROR;
          }
          mp->resume_info.initial_packet_bytes = initial_file_data;
          mp->resume_info.samples_until_repeat = mp->samples_until_repeat + mp->preskip;
          mp->resume_info.offset = mp->cur_file_offs;
          mp->cur_file_offs += 2 + size;
          file_data[1] = 0xfc;
          r = opus_decode(mp->opus, &file_data[2 - n], size + n, mp->buffer, 960, 0);
          if (r <= 0)
            goto READ_ERROR;
          if (r > mp->preskip)
            break;
          mp->preskip -= r;
        }
      } else {
        if (mp->samples_until_repeat == 0) {
          if (mp->resume_info.actual_track < sizeof(kMsuTracksWithRepeat) && !kMsuTracksWithRepeat[mp->resume_info.actual_track])
            goto FINISHED_PLAYING;
          mp->samples_until_repeat = mp->total_samples_in_file - mp->repeat_position;
          if (mp->samples_until_repeat == 0)
            goto READ_ERROR; // impossible to make progress
          mp->cur_file_offs = mp->repeat_position;
          fseek(mp->f, mp->cur_file_offs * 4 + 8, SEEK_SET);
        }
        r = UintMin(960, mp->samples_until_repeat);
        if (fread(mp->buffer, 4, r, mp->f) != r)
          goto READ_ERROR;
        mp->resume_info.offset = mp->cur_file_offs;
        mp->cur_file_offs += r;
      }
      uint32 n = UintMin(r - mp->preskip, mp->samples_until_repeat);
      mp->samples_until_repeat -= n;
      mp->buffer_pos = mp->preskip;
      mp->buffer_size = mp->buffer_pos + n;
      mp->preskip = 0;
    }
#if 0
    if (mp->samples_to_play > 44100 * 5) {
      mp->buffer_pos = mp->buffer_size;
    }
#endif
    int nr = IntMin(audio_samples, mp->buffer_size - mp->buffer_pos);
    int16 *buf = mp->buffer + mp->buffer_pos * 2;
    mp->buffer_pos += nr;

#if 0
    static int t;
    for (int i = 0; i < nr; i++) {
      buf[i * 2 + 0] = buf[i * 2 + 1] = 5000 * sinf(2 * 3.1415 * t++ / 440);
    }
#endif
    MixToBuffer(mp, audio_buffer, buf, nr);

#if 0
    static FILE *f;
    if (!f)f = fopen("out.pcm", "wb");
    fwrite(audio_buffer, 4, nr, f);
    fflush(f);
#endif
    audio_samples -= nr, audio_buffer += nr * 2;
  } while (audio_samples != 0);
}

// Maintain a queue cause the snes and audio callback are not in sync.
struct ApuWriteEnt {
  uint8 ports[4];
};
static struct ApuWriteEnt g_apu_write_ents[16], g_apu_write;
static uint8 g_apu_write_ent_pos, g_apu_write_count, g_apu_total_write;
void zelda_apu_write(uint32_t adr, uint8_t val) {
  g_apu_write.ports[adr & 0x3] = val;
}

void ZeldaPushApuState() {
  ZeldaApuLock();
  g_apu_write_ents[g_apu_write_ent_pos++ & 0xf] = g_apu_write;
  if (g_apu_write_count < 16)
    g_apu_write_count++;
  g_apu_total_write++;
  ZeldaApuUnlock();
}

static void ZeldaPopApuState() {
  if (g_apu_write_count != 0)
    memcpy(g_zenv.player->input_ports, &g_apu_write_ents[(g_apu_write_ent_pos - g_apu_write_count--) & 0xf], 4);
}

void ZeldaDiscardUnusedAudioFrames() {
  if (g_apu_write_count != 0 && memcmp(g_zenv.player->input_ports, &g_apu_write_ents[(g_apu_write_ent_pos - g_apu_write_count) & 0xf], 4) == 0) {
    if (g_apu_total_write >= 16) {
      g_apu_total_write = 14;
      g_apu_write_count--;
    }
  } else {
    g_apu_total_write = 0;
  }
}

static void ZeldaResetApuQueue() {
  g_apu_write_ent_pos = g_apu_total_write = g_apu_write_count = 0;
}

uint8_t zelda_read_apui00() {
  // This needs to be here because the ancilla code reads
  // from the apu and we don't want to make the core code
  // dependent on the apu timings, so relocated this value
  // to 0x648.
  return g_ram[kRam_APUI00];
}

uint8_t zelda_apu_read(uint32_t adr) {
  return g_zenv.player->port_to_snes[adr & 0x3];
}

void ZeldaRenderAudio(int16 *audio_buffer, int samples, int channels) {
  ZeldaApuLock();
  ZeldaPopApuState();
  SpcPlayer_GenerateSamples(g_zenv.player);
  dsp_getSamples(g_zenv.player->dsp, audio_buffer, samples, channels);
  if (g_msu_player.f && channels == 2)
    MsuPlayer_Mix(&g_msu_player, audio_buffer, samples);
  ZeldaApuUnlock();
}

bool ZeldaIsMusicPlaying() {
  if (g_msu_player.state != kMsuState_Idle) {
    return g_msu_player.state != kMsuState_FinishedPlaying;
  } else {
    return g_zenv.player->port_to_snes[0] != 0;
  }
}

void ZeldaRestoreMusicAfterLoad_Locked(bool is_reset) {
  // Restore spc variables from the ram dump.
  SpcPlayer_CopyVariablesFromRam(g_zenv.player);
  // This is not stored in the snapshot
  g_zenv.player->timer_cycles = 0;

  // Restore input ports state
  SpcPlayer *spc_player = g_zenv.player;
  memcpy(spc_player->input_ports, &spc_player->ram[0x410], 4);
  memcpy(g_apu_write.ports, spc_player->input_ports, 4);

  if (is_reset) {
    SpcPlayer_Initialize(g_zenv.player);
  }

  MsuPlayer *mp = &g_msu_player;
  if (mp->enabled) {
    mp->volume = 0.0;
    MsuPlayer_Open(mp, (music_unk1 == 0xf1) ? ((MsuPlayerResumeInfo*)msu_resume_info)->orig_track : 
                   music_unk1, true);

    // If resuming in the middle of a transition, then override
    // the volume with that of the transition.
    if (last_music_control >= 0xf1 && last_music_control <= 0xf3) {
      uint8 target = kVolumeTransitionTarget[last_music_control - 0xf1];
      if (target != msu_volume) {
        float f = kVolumeTransitionTargetFloat[3] * (1.0f / 255);
        mp->volume = msu_volume * f;
        mp->volume_target = target * f;
        mp->volume_step = kVolumeTransitionStepFloat[last_music_control - 0xf1];
      }
    }

    if (g_msu_player.state)
      zelda_apu_write(APUI00, 0xf0);  // pause spc player
  }
  ZeldaResetApuQueue();
}

void ZeldaSaveMusicStateToRam_Locked() {
  SpcPlayer_CopyVariablesToRam(g_zenv.player);
  // SpcPlayer.input_ports is not saved to the SpcPlayer ram by SpcPlayer_CopyVariablesToRam,
  // in any case, we want to save the most recently written data, and that might still
  // be in the queue. 0x410 is a free memory location in the SPC ram, so store it there.
  SpcPlayer *spc_player = g_zenv.player;
  memcpy(&spc_player->ram[0x410], g_apu_write.ports, 4);

  msu_volume = g_msu_player.volume * 255;
  memcpy(msu_resume_info, &g_msu_player.resume_info, sizeof(g_msu_player.resume_info));
}

void ZeldaEnableMsu(uint8 enable) {
  g_msu_player.volume = 1.0f;
  g_msu_player.enabled = enable;
  if (enable & kMsuEnabled_Opuz) {
    if (g_config.audio_freq != 48000)
      fprintf(stderr, "Warning: MSU Opuz requires: AudioFreq = 48000\n");
  } else if (enable) {
    if (g_config.audio_freq != 44100)
      fprintf(stderr, "Warning: MSU requires: AudioFreq = 44100\n");
  }

  float volscale = g_config.msuvolume * (1.0f / 255 / 100);
  float stepscale = g_config.msuvolume * (60.0f / 256 / 100) / g_config.audio_freq;
  for (size_t i = 0; i != countof(kVolumeTransitionStepFloat); i++) {
    kVolumeTransitionStepFloat[i] = kVolumeTransitionStep[i] * stepscale;
    kVolumeTransitionTargetFloat[i] = kVolumeTransitionTarget[i] * volscale;
  }
}

void LoadSongBank(const uint8 *p) {  // 808888
  ZeldaApuLock();
  SpcPlayer_Upload(g_zenv.player, p);
  ZeldaApuUnlock();
}
