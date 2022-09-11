#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>
#include <assert.h>
#include "types.h"

#include "snes/spc.h"
#include "snes/dsp_regs.h"
#include "tracing.h"

#include "spc_player.h"

typedef struct MemMap {
  uint16 off, org_off;
} MemMap;

typedef struct MemMapSized {
  uint16 off, org_off, size;
} MemMapSized;

static const MemMap kChannel_Maps[] = {
  {offsetof(Channel, pattern_order_ptr_for_chan), 0x8030},
  {offsetof(Channel, note_ticks_left), 0x70},
  {offsetof(Channel, note_keyoff_ticks_left), 0x71},
  {offsetof(Channel, subroutine_num_loops), 0x80},
  {offsetof(Channel, volume_fade_ticks), 0x90},
  {offsetof(Channel, pan_num_ticks), 0x91},
  {offsetof(Channel, pitch_slide_length), 0xa0},
  {offsetof(Channel, pitch_slide_delay_left), 0xa1},
  {offsetof(Channel, vibrato_hold_count), 0xb0},
  {offsetof(Channel, vib_depth), 0xb1},
  {offsetof(Channel, tremolo_hold_count), 0xc0},
  {offsetof(Channel, tremolo_depth), 0xc1},
  {offsetof(Channel, vibrato_change_count), 0x100},
  {offsetof(Channel, note_length), 0x200},
  {offsetof(Channel, note_gate_off_fixedpt), 0x201},
  {offsetof(Channel, channel_volume_master), 0x210},
  {offsetof(Channel, instrument_id), 0x211},
  {offsetof(Channel, instrument_pitch_base), 0x8220},
  {offsetof(Channel, saved_pattern_ptr), 0x8230},
  {offsetof(Channel, pattern_start_ptr), 0x8240},
  {offsetof(Channel, pitch_envelope_num_ticks), 0x280},
  {offsetof(Channel, pitch_envelope_delay), 0x281},
  {offsetof(Channel, pitch_envelope_direction), 0x290},
  {offsetof(Channel, pitch_envelope_slide_value), 0x291},
  {offsetof(Channel, vibrato_count), 0x2a0},
  {offsetof(Channel, vibrato_rate), 0x2a1},
  {offsetof(Channel, vibrato_delay_ticks), 0x2b0},
  {offsetof(Channel, vibrato_fade_num_ticks), 0x2b1},
  {offsetof(Channel, vibrato_fade_add_per_tick), 0x2c0},
  {offsetof(Channel, vibrato_depth_target), 0x2c1},
  {offsetof(Channel, tremolo_count), 0x2d0},
  {offsetof(Channel, tremolo_rate), 0x2d1},
  {offsetof(Channel, tremolo_delay_ticks), 0x2e0},
  {offsetof(Channel, channel_transposition), 0x2f0},
  {offsetof(Channel, channel_volume), 0x8300},
  {offsetof(Channel, volume_fade_addpertick), 0x8310},
  {offsetof(Channel, volume_fade_target), 0x320},
  {offsetof(Channel, final_volume), 0x321},
  {offsetof(Channel, pan_value), 0x8330},
  {offsetof(Channel, pan_add_per_tick), 0x8340},
  {offsetof(Channel, pan_target_value), 0x350},
  {offsetof(Channel, pan_flag_with_phase_invert), 0x351},
  {offsetof(Channel, pitch), 0x8360},
  {offsetof(Channel, pitch_add_per_tick), 0x8370},
  {offsetof(Channel, pitch_target), 0x380},
  {offsetof(Channel, fine_tune), 0x381},
  {offsetof(Channel, sfx_sound_ptr), 0x8390},
  {offsetof(Channel, sfx_which_sound), 0x3a0},
  {offsetof(Channel, sfx_arr_countdown), 0x3a1},
  {offsetof(Channel, sfx_note_length_left), 0x3b0},
  {offsetof(Channel, sfx_note_length), 0x3b1},
  {offsetof(Channel, sfx_pan), 0x3d0},
};

static const MemMapSized kSpcPlayer_Maps[] = {
  {offsetof(SpcPlayer, new_value_from_snes), 0x0, 4},
  {offsetof(SpcPlayer, port_to_snes), 0x4, 4},
  {offsetof(SpcPlayer, last_value_from_snes), 0x8, 4},
  {offsetof(SpcPlayer, counter_sf0c), 0xc, 1},
  {offsetof(SpcPlayer, _always_zero), 0xe, 2},
  {offsetof(SpcPlayer, temp_accum), 0x10, 2},
  {offsetof(SpcPlayer, ttt), 0x12, 1},
  {offsetof(SpcPlayer, did_affect_volumepitch_flag), 0x13, 1},
  {offsetof(SpcPlayer, addr0), 0x14, 2},
  {offsetof(SpcPlayer, addr1), 0x16, 2},
  {offsetof(SpcPlayer, lfsr_value), 0x18, 2},
  {offsetof(SpcPlayer, is_chan_on), 0x1a, 1},
  {offsetof(SpcPlayer, fast_forward), 0x1b, 1},
  {offsetof(SpcPlayer, sfx_start_arg_pan), 0x20, 1},
  {offsetof(SpcPlayer, sfx_sound_ptr_cur), 0x2c, 2},
  {offsetof(SpcPlayer, music_ptr_toplevel), 0x40, 2},
  {offsetof(SpcPlayer, block_count), 0x42, 1},
  {offsetof(SpcPlayer, sfx_timer_accum), 0x43, 1},
  {offsetof(SpcPlayer, chn), 0x44, 1},
  {offsetof(SpcPlayer, key_ON), 0x45, 1},
  {offsetof(SpcPlayer, key_OFF), 0x46, 1},
  {offsetof(SpcPlayer, cur_chan_bit), 0x47, 1},
  {offsetof(SpcPlayer, reg_FLG), 0x48, 1},
  {offsetof(SpcPlayer, reg_NON), 0x49, 1},
  {offsetof(SpcPlayer, reg_EON), 0x4a, 1},
  {offsetof(SpcPlayer, reg_PMON), 0x4b, 1},
  {offsetof(SpcPlayer, echo_stored_time), 0x4c, 1},
  {offsetof(SpcPlayer, echo_parameter_EDL), 0x4d, 1},
  {offsetof(SpcPlayer, reg_EFB), 0x4e, 1},
  {offsetof(SpcPlayer, global_transposition), 0x50, 1},
  {offsetof(SpcPlayer, main_tempo_accum), 0x51, 1},
  {offsetof(SpcPlayer, tempo), 0x52, 2},
  {offsetof(SpcPlayer, tempo_fade_num_ticks), 0x54, 1},
  {offsetof(SpcPlayer, tempo_fade_final), 0x55, 1},
  {offsetof(SpcPlayer, tempo_fade_add), 0x56, 2},
  {offsetof(SpcPlayer, master_volume), 0x58, 2},
  {offsetof(SpcPlayer, master_volume_fade_ticks), 0x5a, 1},
  {offsetof(SpcPlayer, master_volume_fade_target), 0x5b, 1},
  {offsetof(SpcPlayer, master_volume_fade_add_per_tick), 0x5c, 2},
  {offsetof(SpcPlayer, vol_dirty), 0x5e, 1},
  {offsetof(SpcPlayer, percussion_base_id), 0x5f, 1},
  {offsetof(SpcPlayer, echo_volume_left), 0x60, 2},
  {offsetof(SpcPlayer, echo_volume_right), 0x62, 2},
  {offsetof(SpcPlayer, echo_volume_fade_add_left), 0x64, 2},
  {offsetof(SpcPlayer, echo_volume_fade_add_right), 0x66, 2},
  {offsetof(SpcPlayer, echo_volume_fade_ticks), 0x68, 1},
  {offsetof(SpcPlayer, echo_volume_fade_target_left), 0x69, 1},
  {offsetof(SpcPlayer, echo_volume_fade_target_right), 0x6a, 1},
  {offsetof(SpcPlayer, sfx_channel_index), 0x3c0, 1},
  {offsetof(SpcPlayer, current_bit), 0x3c1, 1},
  {offsetof(SpcPlayer, dsp_register_index), 0x3c2, 1},
  {offsetof(SpcPlayer, echo_channels), 0x3c3, 1},
  {offsetof(SpcPlayer, byte_3C4), 0x3c4, 1},
  {offsetof(SpcPlayer, byte_3C5), 0x3c5, 1},
  {offsetof(SpcPlayer, echo_fract_incr), 0x3c7, 1},
  {offsetof(SpcPlayer, sfx_channel_index2), 0x3c8, 1},
  {offsetof(SpcPlayer, sfx_channel_bit), 0x3c9, 1},
  {offsetof(SpcPlayer, pause_music_ctr), 0x3ca, 1},
  {offsetof(SpcPlayer, port2_active), 0x3cb, 1},
  {offsetof(SpcPlayer, port2_current_bit), 0x3cc, 1},
  {offsetof(SpcPlayer, port3_active), 0x3cd, 1},
  {offsetof(SpcPlayer, port3_current_bit), 0x3ce, 1},
  {offsetof(SpcPlayer, port1_active), 0x3cf, 1},
  {offsetof(SpcPlayer, port1_current_bit), 0x3e0, 1},
  {offsetof(SpcPlayer, byte_3E1), 0x3e1, 1},
  {offsetof(SpcPlayer, sfx_play_echo_flag), 0x3e2, 1},
  {offsetof(SpcPlayer, sfx_channels_echo_mask2), 0x3e3, 1},
  {offsetof(SpcPlayer, port1_counter), 0x3e4, 1},
  {offsetof(SpcPlayer, channel_67_volume), 0x3e5, 1},
  {offsetof(SpcPlayer, cutk_always_zero), 0x3ff, 1},
};

static void PlayNote(SpcPlayer *p, Channel *c, uint8 note);

static void Dsp_Write(SpcPlayer *p, uint8_t reg, uint8 value) {
  DspRegWriteHistory *hist = p->reg_write_history;
  if (hist) {
    if (hist->count < 256) {
      hist->addr[hist->count] = reg;
      hist->val[hist->count] = value;
      hist->count++;
    }
  }
  if (p->dsp)
    dsp_write(p->dsp, reg, value);
}

static void Not_Implemented() {
  assert(0);
  printf("Not Implemented\n");
}

static uint16 SpcDivHelper(int a, uint8 b) {
  int org_a = a;
  if (a & 0x100)
    a = -a;
  int q = b ? (a & 0xff) / b : 0xff;
  int r = b ? (a & 0xff) % b : (a & 0xff);
  int t = (q << 8) + (b ? ((r << 8) / b & 0xff) : 0xff);
  return (org_a & 0x100) ? -t : t;
}

static inline void Chan_DoAnyFade(uint16 *p, uint16 add, uint8 target, uint8 cont) {
  if (!cont)
    *p = target << 8;
  else
    *p += add;
}

static void SetupEchoParameter_EDL(SpcPlayer *p, uint8 a) {
  p->echo_parameter_EDL = a;
  if (a != p->last_written_edl) {
    a = (p->last_written_edl & 0xf) ^ 0xff;
    if (p->echo_stored_time & 0x80)
      a += p->echo_stored_time;
    p->echo_stored_time = a;

    Dsp_Write(p, EON, 0);
    Dsp_Write(p, EFB, 0);
    Dsp_Write(p, EVOLR, 0);
    Dsp_Write(p, EVOLL, 0);
    Dsp_Write(p, FLG, p->reg_FLG | 0x20);

    p->last_written_edl = p->echo_parameter_EDL;
    Dsp_Write(p, EDL, p->echo_parameter_EDL);
  }
  Dsp_Write(p, ESA, (p->echo_parameter_EDL * 8 ^ 0xff) + 0xd1);
}

static void WriteVolumeToDsp(SpcPlayer *p, Channel *c, uint16 volume) {
  static const uint8 kVolumeTable[22] = {0, 1, 3, 7, 13, 21, 30, 41, 52, 66, 81, 94, 103, 110, 115, 119, 122, 124, 125, 126, 127, 127};
  if (p->is_chan_on & p->cur_chan_bit)
    return;
  for (int i = 0; i < 2; i++) {
    int j = volume >> 8;
    uint8 t;
    if (j >= 21) {
      t = p->ram[j + 0x1178] + ((p->ram[j + 0x1179] - p->ram[j + 0x1178]) * (uint8)volume >> 8);
    } else {
      t = kVolumeTable[j] + ((kVolumeTable[j + 1] - kVolumeTable[j]) * (uint8)volume >> 8);
    }
    
     
    t = t * c->final_volume >> 8;
    if ((c->pan_flag_with_phase_invert << i) & 0x80)
      t = -t;
    Dsp_Write(p, V0VOLL + i + c->index * 16, t);
    volume = 0x1400 - volume;
  }
}

static void WritePitch(SpcPlayer *p, Channel *c, uint16 pitch) {
  static const uint16 kBaseNoteFreqs[13] = {2143, 2270, 2405, 2548, 2700, 2860, 3030, 3211, 3402, 3604, 3818, 4045, 4286};
  if ((pitch >> 8) >= 0x34) {
    pitch += (pitch >> 8) - 0x34;
  } else if ((pitch >> 8) < 0x13) {
    pitch += (uint8)(((pitch >> 8) - 0x13) * 2) - 256;
  }

  uint8 pp = (pitch >> 8) & 0x7f;
  uint8 q = pp / 12, r = pp % 12;
  uint16 t = kBaseNoteFreqs[r] + ((uint8)(kBaseNoteFreqs[r + 1] - kBaseNoteFreqs[r]) * (uint8)pitch >> 8);
  t *= 2;
  while (q != 6)
    t >>= 1, q++;

  t = c->instrument_pitch_base * t >> 8;
  if (!(p->cur_chan_bit & p->is_chan_on)) {
    uint8 reg = c->index * 16;
    Dsp_Write(p, reg + V0PITCHL, t & 0xff);
    Dsp_Write(p, reg + V0PITCHH, t >> 8);
  }
}

static void Music_ResetChan(SpcPlayer *p) {
  Channel *c = &p->channel[7];
  p->cur_chan_bit = 0x80;
  do {
    HIBYTE(c->channel_volume) = 0xff;
    c->pan_flag_with_phase_invert = 10;
    c->pan_value = 10 << 8;
    c->instrument_id = 0;
    c->fine_tune = 0;
    c->channel_transposition = 0;
    c->pitch_envelope_num_ticks = 0;
    c->vib_depth = 0;
    c->tremolo_depth = 0;
  } while (c--, p->cur_chan_bit >>= 1);
  p->master_volume_fade_ticks = 0;
  p->echo_volume_fade_ticks = 0;
  p->tempo_fade_num_ticks = 0;
  p->global_transposition = 0;
  p->block_count = 0;
  p->percussion_base_id = 0;
  HIBYTE(p->master_volume) = 0xc0;
  HIBYTE(p->tempo) = 0x20;
}

static void Channel_SetInstrument(SpcPlayer *p, Channel *c, uint8 instrument) {
  c->instrument_id = instrument;
  if (instrument & 0x80)
    instrument = instrument + 54 + p->percussion_base_id;
  const uint8 *ip = p->ram + instrument * 6 + 0x3d00;
  if (p->is_chan_on & p->cur_chan_bit)
    return;
  uint8 reg = c->index * 16;
  if (ip[0] & 0x80) {
    // noise
    p->reg_FLG = (p->reg_FLG & 0x20) | ip[0] & 0x1f;
    p->reg_NON |= p->cur_chan_bit;
    Dsp_Write(p, reg + V0SRCN, 0);
  } else {
    Dsp_Write(p, reg + V0SRCN, ip[0]);
  }
  Dsp_Write(p, reg + V0ADSR1, ip[1]);
  Dsp_Write(p, reg + V0ADSR2, ip[2]);
  Dsp_Write(p, reg + V0GAIN, ip[3]);
  c->instrument_pitch_base = ip[4] << 8 | ip[5];
}

static void ComputePitchAdd(Channel *c, uint8 pitch) {
  c->pitch_target = pitch & 0x7f;
  c->pitch_add_per_tick = SpcDivHelper(c->pitch_target - (c->pitch >> 8), c->pitch_slide_length);
}

static void PitchSlideToNote_Check(SpcPlayer *p, Channel *c) {
  if (c->pitch_slide_length || p->ram[c->pattern_order_ptr_for_chan] != 0xf9)
    return;

  if (p->cur_chan_bit & p->is_chan_on) {
    c->pattern_order_ptr_for_chan += 4;
    return;
  }
  c->pattern_order_ptr_for_chan++;
  c->pitch_slide_delay_left = p->ram[c->pattern_order_ptr_for_chan++];
  c->pitch_slide_length = p->ram[c->pattern_order_ptr_for_chan++];
  ComputePitchAdd(c, p->ram[c->pattern_order_ptr_for_chan++] + p->global_transposition + c->channel_transposition);
}

static const uint8 kEffectByteLength[27] = {1, 1, 2, 3, 0, 1, 2, 1, 2, 1, 1, 3, 0, 1, 2, 3, 1, 3, 3, 0, 1, 3, 0, 3, 3, 3, 1};

static void HandleEffect(SpcPlayer *p, Channel *c, uint8 effect) {
  uint8 arg = kEffectByteLength[effect - 0xe0] ? p->ram[c->pattern_order_ptr_for_chan++] : 0;

  switch (effect) {
  case 0xe0:
    Channel_SetInstrument(p, c, arg);
    break;
  case 0xe1:
    c->pan_flag_with_phase_invert = arg;
    c->pan_value = (arg & 0x1f) << 8;
    break;
  case 0xe2:
    c->pan_num_ticks = arg;
    c->pan_target_value = p->ram[c->pattern_order_ptr_for_chan++];
    c->pan_add_per_tick = SpcDivHelper(c->pan_target_value - (c->pan_value >> 8), arg);
    break;
  case 0xe3: // vibrato on
    c->vibrato_delay_ticks = arg;
    c->vibrato_rate = p->ram[c->pattern_order_ptr_for_chan++];
    c->vibrato_depth_target = c->vib_depth = p->ram[c->pattern_order_ptr_for_chan++];
    c->vibrato_fade_num_ticks = 0;
    break;
  case 0xe4: // vibrato off
    c->vibrato_depth_target = c->vib_depth = 0;
    c->vibrato_fade_num_ticks = 0;
    break;
  case 0xe5:
    if (!p->pause_music_ctr && !p->byte_3E1)
      p->master_volume = arg << 8;
    break;
  case 0xe6:
    p->master_volume_fade_ticks = arg;
    p->master_volume_fade_target = p->ram[c->pattern_order_ptr_for_chan++];
    p->master_volume_fade_add_per_tick = SpcDivHelper(p->master_volume_fade_target - (p->master_volume >> 8), arg);
    break;
  case 0xe7:
    p->tempo = arg << 8;
    break;
  case 0xe8:
    p->tempo_fade_num_ticks = arg;
    p->tempo_fade_final = p->ram[c->pattern_order_ptr_for_chan++];
    p->tempo_fade_add = SpcDivHelper(p->tempo_fade_final - (p->tempo >> 8), arg);
    break;
  case 0xe9:
    p->global_transposition = arg;
    break;
  case 0xea:
    c->channel_transposition = arg;
    break;
  case 0xeb:
    c->tremolo_delay_ticks = arg;
    c->tremolo_rate = p->ram[c->pattern_order_ptr_for_chan++];
    c->tremolo_depth = p->ram[c->pattern_order_ptr_for_chan++];
    break;
  case 0xec:
    c->tremolo_depth = 0;
    break;
  case 0xed:
    c->channel_volume = arg << 8;
    break;
  case 0xee:
    c->volume_fade_ticks = arg;
    c->volume_fade_target = p->ram[c->pattern_order_ptr_for_chan++];
    c->volume_fade_addpertick = SpcDivHelper(c->volume_fade_target - (c->channel_volume >> 8), arg);
    break;
  case 0xef:
    c->pattern_start_ptr = p->ram[c->pattern_order_ptr_for_chan++] << 8 | arg;
    c->subroutine_num_loops = p->ram[c->pattern_order_ptr_for_chan++];
    c->saved_pattern_ptr = c->pattern_order_ptr_for_chan;
    c->pattern_order_ptr_for_chan = c->pattern_start_ptr;
    break;
  case 0xf0:
    c->vibrato_fade_num_ticks = arg;
    c->vibrato_fade_add_per_tick = arg ? c->vib_depth / arg : 0xff;
    break;
  case 0xf4:
    c->fine_tune = arg;
    break;
  case 0xf5:
    p->reg_EON = p->echo_channels = arg;
    p->echo_volume_left = p->ram[c->pattern_order_ptr_for_chan++] << 8;
    p->echo_volume_right = p->ram[c->pattern_order_ptr_for_chan++] << 8;
    p->reg_FLG &= ~0x20;
    break;
  case 0xf6:  // echo off
    p->echo_volume_left = 0;
    p->echo_volume_right = 0;
    p->reg_FLG |= 0x20;
    break;
  case 0xf7: {
    static const int8_t kEchoFirParameters[] = {
      127, 0, 0, 0, 0, 0, 0, 0,
      88, -65, -37, -16, -2, 7, 12, 12,
      12, 33, 43, 43, 19, -2, -13, -7,
      52, 51, 0, -39, -27, 1, -4, -21,
    };
    SetupEchoParameter_EDL(p, arg);
    p->reg_EFB = p->ram[c->pattern_order_ptr_for_chan++];
    const int8_t *ep = kEchoFirParameters + p->ram[c->pattern_order_ptr_for_chan++] * 8;
    for (int i = 0; i < 8; i++)
      Dsp_Write(p, FIR0 + i * 16, *ep++);
    break;
  }
  case 0xf8:
    p->echo_volume_fade_ticks = arg;
    p->echo_volume_fade_target_left = p->ram[c->pattern_order_ptr_for_chan++];
    p->echo_volume_fade_target_right = p->ram[c->pattern_order_ptr_for_chan++];
    p->echo_volume_fade_add_left = SpcDivHelper(p->echo_volume_fade_target_left - (p->echo_volume_left >> 8), arg);
    p->echo_volume_fade_add_right = SpcDivHelper(p->echo_volume_fade_target_right - (p->echo_volume_right >> 8), arg);
    break;
  case 0xf9:
    c->pitch_slide_delay_left = arg;
    c->pitch_slide_length = p->ram[c->pattern_order_ptr_for_chan++];
    ComputePitchAdd(c, p->ram[c->pattern_order_ptr_for_chan++] + p->global_transposition + c->channel_transposition);
    break;
  case 0xfa:
    p->percussion_base_id = arg;
    break;
  default:
    Not_Implemented();
  }
}

static bool WantWriteKof(SpcPlayer *p, Channel *c) {
  int loops = c->subroutine_num_loops;
  int ptr = c->pattern_order_ptr_for_chan;

  for (;;) {
    uint8 cmd = p->ram[ptr++];
    if (cmd == 0) {
      if (loops == 0)
        return true;
      ptr = (--loops == 0) ? c->saved_pattern_ptr : c->pattern_start_ptr;
    } else {
      while (!(cmd & 0x80))
        cmd = p->ram[ptr++];
      if (cmd == 0xc8)
        return false;
      if (cmd == 0xef) {
        ptr = p->ram[ptr + 0] | p->ram[ptr + 1] << 8;
      } else if (cmd >= 0xe0) {
        ptr += kEffectByteLength[cmd - 0xe0];
      } else {
        return true;
      }
    }
  }
}

static void HandleTremolo(SpcPlayer *p, Channel *c) {
  Not_Implemented();
}

static void CalcVibratoAddPitch(SpcPlayer *p, Channel *c, uint16 pitch, uint8 value) {
  int t = value << 2;
  t ^= (t & 0x100) ? 0xff : 0;
  int r = (c->vib_depth >= 0xf1) ?
      (uint8)t * (c->vib_depth & 0xf) :
      (uint8)t * c->vib_depth >> 8;
  WritePitch(p, c, pitch + (value & 0x80 ? -r : r));
}

static void HandlePanAndSweep(SpcPlayer *p, Channel *c) {
  p->did_affect_volumepitch_flag = 0;
  if (c->tremolo_depth) {
    c->tremolo_hold_count = c->tremolo_delay_ticks;
    HandleTremolo(p, c);
  }

  uint16 volume = c->pan_value;

  if (c->pan_num_ticks) {
    p->did_affect_volumepitch_flag = 0x80;
    volume += p->main_tempo_accum * (int16_t)c->pan_add_per_tick / 256;
  }

  if (p->did_affect_volumepitch_flag)
    WriteVolumeToDsp(p, c, volume);

  p->did_affect_volumepitch_flag = 0;
  uint16 pitch = c->pitch;
  if (c->pitch_slide_length && !c->pitch_slide_delay_left) {
    p->did_affect_volumepitch_flag |= 0x80;
    pitch += p->main_tempo_accum * (int16_t)c->pitch_add_per_tick / 256;
  }

  if (c->vib_depth && c->vibrato_delay_ticks == c->vibrato_hold_count) {
    CalcVibratoAddPitch(p, c, pitch, (p->main_tempo_accum * c->vibrato_rate >> 8) + c->vibrato_count);
    return;
  }

  if (p->did_affect_volumepitch_flag)
    WritePitch(p, c, pitch);
}

static void HandleNoteTick(SpcPlayer *p, Channel *c) {
  if (c->note_keyoff_ticks_left != 0 && (--c->note_keyoff_ticks_left == 0 || c->note_ticks_left == 2)) {
    if (WantWriteKof(p, c) && !(p->cur_chan_bit & p->is_chan_on))
      Dsp_Write(p, KOF, p->cur_chan_bit);
  }

  p->did_affect_volumepitch_flag = 0;
  if (c->pitch_slide_length) {
    if (c->pitch_slide_delay_left) {
      c->pitch_slide_delay_left--;
    } else if (!(p->is_chan_on & p->cur_chan_bit)) {
      p->did_affect_volumepitch_flag = 0x80;
      Chan_DoAnyFade(&c->pitch, c->pitch_add_per_tick, c->pitch_target, --c->pitch_slide_length);
    }
  }

  uint16 pitch = c->pitch;

  if (c->vib_depth) {
    if (c->vibrato_delay_ticks == c->vibrato_hold_count) {
      if (c->vibrato_change_count == c->vibrato_fade_num_ticks) {
        c->vib_depth = c->vibrato_depth_target;
      } else {
        c->vib_depth = (c->vibrato_change_count++ == 0 ? 0 : c->vib_depth) + c->vibrato_fade_add_per_tick;
      }
      c->vibrato_count += c->vibrato_rate;
      CalcVibratoAddPitch(p, c, pitch, c->vibrato_count);
      return;
    }
    c->vibrato_hold_count++;
  }
  
  if (p->did_affect_volumepitch_flag)
    WritePitch(p, c, pitch);
}

void CalcFinalVolume(SpcPlayer *p, Channel *c, uint8 vol) {
  int t = (p->master_volume >> 8) * vol >> 8;
  t = t * c->channel_volume_master >> 8;
  t = t * (c->channel_volume >> 8) >> 8;
  c->final_volume = t * t >> 8;
}

void CalcTremolo(SpcPlayer *p, Channel *c) {
  Not_Implemented();
}

static void Chan_HandleTick(SpcPlayer *p, Channel *c) {
  if (c->volume_fade_ticks) {
    c->volume_fade_ticks--;
    p->vol_dirty |= p->cur_chan_bit;
    Chan_DoAnyFade(&c->channel_volume, c->volume_fade_addpertick, c->volume_fade_target, true);
  }
  if (c->tremolo_depth) {
    if (c->tremolo_delay_ticks == c->tremolo_hold_count) {
      p->vol_dirty |= p->cur_chan_bit;
      if (c->tremolo_count & 0x80 && c->tremolo_depth == 0xff) {
        c->tremolo_count = 0x80;
      } else {
        c->tremolo_count += c->tremolo_rate;
      }
      CalcTremolo(p, c);
    } else {
      c->tremolo_hold_count++;
      CalcFinalVolume(p, c, 0xff);
    }
  } else {
    CalcFinalVolume(p, c, 0xff);
  }

  if (c->pan_num_ticks) {
    c->pan_num_ticks--;
    p->vol_dirty |= p->cur_chan_bit;
    Chan_DoAnyFade(&c->pan_value, c->pan_add_per_tick, c->pan_target_value, true);
  }

  if (p->vol_dirty & p->cur_chan_bit)
    WriteVolumeToDsp(p, c, c->pan_value);
}

static void Port0_HandleMusic(SpcPlayer *p) {
  Channel *c;
  uint8 a = p->new_value_from_snes[0];
  int t;

  if (a == 0) {
handle_cmd_00:
    if (p->port_to_snes[0] == 0)
      return;
    if (p->pause_music_ctr != 0 && --p->pause_music_ctr == 0)
      goto HandleCmd_0xf0_PauseMusic;
    if (p->counter_sf0c == 0)
      goto label_a;
    if (--p->counter_sf0c != 0) {
      Music_ResetChan(p);
      return;
    }
next_phrase:
    for (;;) {
      t = WORD(p->ram[p->music_ptr_toplevel]);
      p->music_ptr_toplevel += 2;
      if ((t >> 8) != 0)
        break;
      if (t == 0)
        goto HandleCmd_0xf0_PauseMusic;
      if (t == 0x80) {
        p->fast_forward = 0x80;
      } else if (t == 0x81) {
        p->fast_forward = 0;
      } else {
        if (sign8(--p->block_count))
          p->block_count = t;
        t = WORD(p->ram[p->music_ptr_toplevel]);
        p->music_ptr_toplevel += 2;
        if (p->block_count != 0)
          p->music_ptr_toplevel = t;
      }
    }
    for (int i = 0; i < 8; i++)
      p->channel[i].pattern_order_ptr_for_chan = WORD(p->ram[t]), t += 2;

    c = p->channel, p->cur_chan_bit = 1;
    do {
      if (HIBYTE(c->pattern_order_ptr_for_chan) && c->instrument_id == 0)
        Channel_SetInstrument(p, c, 0);
      c->subroutine_num_loops = 0;
      c->volume_fade_ticks = 0;
      c->pan_num_ticks = 0;
      c->note_ticks_left = 1;
    } while (c++, p->cur_chan_bit <<= 1);
label_a:
    p->vol_dirty = 0;
    c = p->channel, p->cur_chan_bit = 1;
    do {
      if (!HIBYTE(c->pattern_order_ptr_for_chan))
        continue;
      if (!--c->note_ticks_left) {
        for (;;) {
          uint8 cmd = p->ram[c->pattern_order_ptr_for_chan++];
          if (cmd == 0) {
            if (!c->subroutine_num_loops)
              goto next_phrase;
            c->pattern_order_ptr_for_chan = (--c->subroutine_num_loops == 0) ? c->saved_pattern_ptr : c->pattern_start_ptr;
            continue;
          }
          if (!(cmd & 0x80)) {
            static const uint8 kNoteVol[16] = { 25, 50, 76, 101, 114, 127, 140, 152, 165, 178, 191, 203, 216, 229, 242, 252 };
            static const uint8 kNoteGateOffPct[8] = { 50, 101, 127, 152, 178, 203, 229, 252 };
            c->note_length = cmd;
            cmd = p->ram[c->pattern_order_ptr_for_chan++];
            if (!(cmd & 0x80)) {
              c->note_gate_off_fixedpt = kNoteGateOffPct[cmd >> 4 & 7];
              c->channel_volume_master = kNoteVol[cmd & 0xf];
              cmd = p->ram[c->pattern_order_ptr_for_chan++];
            }
          }
          if (cmd >= 0xe0) {
            HandleEffect(p, c, cmd); 
            continue;
          }
          if (!p->fast_forward && !(p->is_chan_on & p->cur_chan_bit))
            PlayNote(p, c, cmd);
          c->note_ticks_left = c->note_length;
          t = c->note_ticks_left * c->note_gate_off_fixedpt >> 8;
          c->note_keyoff_ticks_left = (t != 0) ? t : 1;
          PitchSlideToNote_Check(p, c);
          break;
        }
      } else if (!p->fast_forward) {
        HandleNoteTick(p, c);
        PitchSlideToNote_Check(p, c);
      }
    } while (c++, p->cur_chan_bit <<= 1);
    if (p->tempo_fade_num_ticks)
      p->tempo = (--p->tempo_fade_num_ticks == 0) ? p->tempo_fade_final << 8 : p->tempo + p->tempo_fade_add;
    if (p->echo_volume_fade_ticks) {
      p->echo_volume_left += p->echo_volume_fade_add_left;
      p->echo_volume_right += p->echo_volume_fade_add_right;
      if (--p->echo_volume_fade_ticks == 0) {
        p->echo_volume_left = p->echo_volume_fade_target_left << 8;
        p->echo_volume_right = p->echo_volume_fade_target_right << 8;
      }
    }
    if (p->master_volume_fade_ticks) {
      p->master_volume = (--p->master_volume_fade_ticks == 0) ? p->master_volume_fade_target << 8 : p->master_volume + p->master_volume_fade_add_per_tick;
      p->vol_dirty = 0xff;
    }
    c = p->channel, p->cur_chan_bit = 1;
    do {
      if (HIBYTE(c->pattern_order_ptr_for_chan))
        Chan_HandleTick(p, c);
    } while (c++, p->cur_chan_bit <<= 1);
  } else if (a == 0xff) {
    // Load new music
    Not_Implemented();
  } else if (a == 0xf1) { // continue music
    p->master_volume_fade_ticks = 0x80;
    p->pause_music_ctr = 0x80;
    p->master_volume_fade_target = 0;
    p->master_volume_fade_add_per_tick = SpcDivHelper(0 - (p->master_volume >> 8), 0x80);

    goto handle_cmd_00;
  } else if (a == 0xf2) {
    if (p->byte_3E1 != 0)
      return;
    p->byte_3E1 = HIBYTE(p->master_volume);
    HIBYTE(p->master_volume) = 0x70;
    goto handle_cmd_00;
  } else if (a == 0xf3) {
    if (p->byte_3E1 == 0)
      return;
    HIBYTE(p->master_volume) = p->byte_3E1;
    p->byte_3E1 = 0;
    goto handle_cmd_00;
  } else if (a == 0xf0) {
HandleCmd_0xf0_PauseMusic:
    p->key_OFF = p->is_chan_on ^ 0xff;
    p->port_to_snes[0] = 0;
    p->cur_chan_bit = 0;
  } else {
    p->pause_music_ctr = 0;
    p->byte_3E1 = 0;
    p->port_to_snes[0] = a;
    p->music_ptr_toplevel = WORD(p->ram[0xD000 + (a - 1) * 2]);
    p->counter_sf0c = 2;
    p->key_OFF |= p->is_chan_on ^ 0xff;
  }

}

static inline uint8 Asl(uint8 *p) {
  uint8 old = *p;
  *p <<= 1;
  return old >> 7;
}

static void Sfx_TurnOffChannel(SpcPlayer *p, Channel *c) {
  c->sfx_which_sound = 0;
  p->is_chan_on &= ~p->current_bit;
  p->port1_active &= ~p->current_bit;
  p->port2_active &= ~p->current_bit;
  p->port3_active &= ~p->current_bit;
  Channel_SetInstrument(p, c, c->instrument_id);
  if (p->echo_channels & p->current_bit && !(p->reg_EON & p->current_bit)) {
    p->reg_EON |= p->current_bit;
    Dsp_Write(p, EON, p->reg_EON);
    p->sfx_channels_echo_mask2 &= ~p->current_bit;
  }
}

static void Write_KeyOn(SpcPlayer *p, uint8 bit) {
  Dsp_Write(p, KOF, 0);
  Dsp_Write(p, KON, bit);
}

static void PlayNote(SpcPlayer *p, Channel *c, uint8 note) {
  if (note >= 0xca) {
    Channel_SetInstrument(p, c, note);
    note = 0xa4;
  }

//  if (c->index == 0) {
//    if (note == 0xc8) {
//      printf("-+-\n");
//    } else if (note == 0xc9) {
//      printf("---\n");
//    }
//  }

  if (note >= 0xc8 || p->is_chan_on & p->cur_chan_bit)
    return;

  static const char *const kNoteNames[] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
  //if (c->index==0)
  //  printf("%s%d\n", kNoteNames[(note & 0x7f) % 12], (note & 0x7f) / 12 + 1);

  c->pitch = ((note & 0x7f) + p->global_transposition + c->channel_transposition) << 8 | c->fine_tune;
  c->vibrato_count = c->vibrato_fade_num_ticks << 7;
  c->vibrato_hold_count = 0;
  c->vibrato_change_count = 0;
  c->tremolo_count = 0;
  c->tremolo_hold_count = 0;
  p->vol_dirty |= p->cur_chan_bit;
  p->key_ON |= p->cur_chan_bit;
  c->pitch_slide_length = c->pitch_envelope_num_ticks;
  if (c->pitch_slide_length) {
    c->pitch_slide_delay_left = c->pitch_envelope_delay;
    if (!c->pitch_envelope_direction)
      c->pitch -= c->pitch_envelope_slide_value << 8;
    ComputePitchAdd(c, (c->pitch >> 8) + c->pitch_envelope_slide_value);
  }
  WritePitch(p, c, c->pitch);
}

static void Sfx_MaybeDisableEcho(SpcPlayer *p) {
  if (!(p->port_to_snes[0] & 0x10) || p->current_bit & p->sfx_channels_echo_mask2) {
    if (p->current_bit & p->reg_EON) {
      p->reg_EON ^= p->current_bit;
      Dsp_Write(p, EON, p->reg_EON);
    }
  }
}

static void Sfx_ChannelTick(SpcPlayer *p, Channel *c, bool is_continue) {
  uint8 cmd;

  if (is_continue) {
    Sfx_MaybeDisableEcho(p);
    p->sfx_channel_index = c->index * 2;
    p->sfx_sound_ptr_cur = c->sfx_sound_ptr;
    if (--c->sfx_note_length_left)
      goto note_continue;
    p->sfx_sound_ptr_cur++;
  }

  for (;;) {
    p->dsp_register_index = p->sfx_channel_index * 8;

    cmd = p->ram[p->sfx_sound_ptr_cur];
    if (cmd == 0) {
      Sfx_TurnOffChannel(p, c);
      return;
    }

    if (!(cmd & 0x80)) {
      c->sfx_note_length = cmd;
      cmd = p->ram[++p->sfx_sound_ptr_cur];
      if (!(cmd & 0x80)) {
        if (p->port1_active & p->current_bit) {
          if (cmd == 0 || !p->channel_67_volume) {
            uint8 volume = cmd;
            Dsp_Write(p, p->dsp_register_index + V0VOLL, cmd);
            cmd = p->ram[++p->sfx_sound_ptr_cur];
            if (cmd & 0x80) {
              Dsp_Write(p, p->dsp_register_index + V0VOLR, volume);
            } else {
              Dsp_Write(p, p->dsp_register_index + V0VOLR, cmd);
              cmd = p->ram[++p->sfx_sound_ptr_cur];
            }
          } else {
            cmd = p->ram[++p->sfx_sound_ptr_cur];
          }
        } else {
          c->final_volume = cmd * 2;
          c->pan_flag_with_phase_invert = 10;
          WriteVolumeToDsp(p, c, (p->sfx_start_arg_pan & 0x80 ? 16 : p->sfx_start_arg_pan & 0x40 ? 4 : 10) << 8);
          cmd = p->ram[++p->sfx_sound_ptr_cur];
        }
      }
    }
    // cmd_parsed
    if (cmd == 0xe0) {
      const uint8 *ip = p->ram + 0x3E00 + (p->ram[++p->sfx_sound_ptr_cur] * 9);
      uint8 reg = c->index * 16;
      Dsp_Write(p, reg + V0VOLL, ip[0]);
      Dsp_Write(p, reg + V0VOLR, ip[1]);
      Dsp_Write(p, reg + V0PITCHL, ip[2]);
      Dsp_Write(p, reg + V0PITCHH, ip[3]);
      Dsp_Write(p, reg + V0SRCN, ip[4]);
      Dsp_Write(p, reg + V0ADSR1, ip[5]);
      Dsp_Write(p, reg + V0ADSR2, ip[6]);
      Dsp_Write(p, reg + V0GAIN, ip[7]);
      c->instrument_pitch_base = ip[8] << 8;
      p->sfx_sound_ptr_cur++;
    } else if (cmd == 0xf9 || cmd == 0xf1) {
      if (cmd == 0xf9) {
        PlayNote(p, c, p->ram[++p->sfx_sound_ptr_cur]);
        Write_KeyOn(p, p->current_bit);
      }
      c->pitch_slide_delay_left = p->ram[++p->sfx_sound_ptr_cur];
      c->pitch_slide_length = p->ram[++p->sfx_sound_ptr_cur];
      ComputePitchAdd(c, p->ram[++p->sfx_sound_ptr_cur]);
      c->sfx_note_length_left = c->sfx_note_length;
      goto note_continue;
    } else if (cmd == 0xff) {
      p->sfx_sound_ptr_cur = c->sfx_sound_ptr = WORD(p->ram[0x17C0 + (c->sfx_which_sound - 1) * 2]);
    } else {
      PlayNote(p, c, cmd);
      Write_KeyOn(p, p->current_bit);
      c->sfx_note_length_left = c->sfx_note_length;
note_continue:
      p->did_affect_volumepitch_flag = 0;
      if (c->pitch_slide_length) {
        p->did_affect_volumepitch_flag = 0x80;
        Chan_DoAnyFade(&c->pitch, c->pitch_add_per_tick, c->pitch_target, --c->pitch_slide_length);
        p->cur_chan_bit = 0;  // force change through
        WritePitch(p, c, c->pitch);
      } else if (c->sfx_note_length_left == 2) {
        Dsp_Write(p, KOF, p->current_bit);
      }
      break;
    }
  }
  c->sfx_sound_ptr = p->sfx_sound_ptr_cur;
}

static void Port1_Play_Inner(SpcPlayer *p) {
  p->port1_counter = 0;
  Channel *c = &p->channel[7];
  c->sfx_which_sound = p->new_value_from_snes[1];
  c->sfx_arr_countdown = 3;
  c->pitch_envelope_num_ticks = 0;
  p->port1_active = 0x80;
  p->is_chan_on |= 0x80;
  Dsp_Write(p, KOF, 0x80);
  p->new_value_from_snes[1] = p->ram[0x1800 + p->new_value_from_snes[1] - 1];
  if (!p->new_value_from_snes[1])
    return;
  c--;
  c->sfx_which_sound = p->new_value_from_snes[1];
  c->sfx_arr_countdown = 3;
  c->pitch_envelope_num_ticks = 0;
  p->port1_active = 0x40;
  p->is_chan_on |= 0x40;
  Dsp_Write(p, KOF, 0x40);
  p->port1_active = 0xc0;
  p->sfx_channels_echo_mask2 |= 0xc0;
  p->port2_active &= 0x3f;
  p->port3_active &= 0x3f;
}

static void Port1_StartNewSound(SpcPlayer *p) {
  if (p->port1_counter != 0) {
    if (--p->port1_counter == 0) {
      p->new_value_from_snes[1] = 5;
      Port1_Play_Inner(p);
      p->new_value_from_snes[1] = 0;
      return;
    }
    p->channel_67_volume = p->port1_counter >> 1;
    Dsp_Write(p, V7VOLL, p->channel_67_volume);
    Dsp_Write(p, V7VOLR, p->channel_67_volume);
    Dsp_Write(p, V6VOLL, p->channel_67_volume);
    Dsp_Write(p, V6VOLR, p->channel_67_volume);
  }
  p->port1_current_bit = p->port1_active;
  if (!p->port1_current_bit)
    return;
  Channel *c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (Asl(&p->port1_current_bit)) {
      p->sfx_channel_index = c->index * 2;
      p->dsp_register_index = c->index * 16;
      p->sfx_start_arg_pan = c->sfx_pan;
      if (!c->sfx_arr_countdown) {
        if (c->sfx_which_sound)
          Sfx_ChannelTick(p, c, true);
      } else {
        p->sfx_channel_index = c->index * 2;
        if (!--c->sfx_arr_countdown) {
          p->sfx_sound_ptr_cur = c->sfx_sound_ptr = WORD(p->ram[0x17C0 + (c->sfx_which_sound - 1) * 2]);
          Sfx_ChannelTick(p, c, false);
        }
      }
    }
  } while (c--, (p->current_bit >>= 1) != 0x10);
}

static void Port1_HandleCmd(SpcPlayer *p) {
  uint8 a = p->new_value_from_snes[1];
  if (!(a & 0x80)) {
    if (a != 0) {
      p->port_to_snes[1] = a;
      if (a != 5 || p->port1_active)
        Port1_Play_Inner(p);
    }
  } else {  
    p->port_to_snes[1] = a;
    if (p->port1_active)
      p->port1_counter = 0x78;
  }
}

static void Port2_StartNewSound(SpcPlayer *p) {
  p->port2_current_bit = p->port2_active;
  if (!p->port2_current_bit)
    return;
  Channel *c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (Asl(&p->port2_current_bit)) {
      p->sfx_channel_index = c->index * 2;
      p->dsp_register_index = c->index * 16;
      p->sfx_start_arg_pan = c->sfx_pan;
      if (!c->sfx_arr_countdown) {
        if (c->sfx_which_sound)
          Sfx_ChannelTick(p, c, true);
      } else {
        p->sfx_channel_index = c->index * 2;
        if (!--c->sfx_arr_countdown) {
          p->sfx_sound_ptr_cur = c->sfx_sound_ptr = WORD(p->ram[0x1820 + (c->sfx_which_sound - 1) * 2]);
          Sfx_ChannelTick(p, c, false);
        }
      }
    }
  } while (c--, p->current_bit >>= 1);
}

static Channel *Port2_AllocateChan(SpcPlayer *p) {
  p->sfx_play_echo_flag = p->ram[0x18dd + (p->new_value_from_snes[2] & 0x3f) - 1];
  Channel *c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (p->port2_active & p->current_bit && c->sfx_which_sound + c->sfx_pan == p->new_value_from_snes[2])
      goto found_channel;
  } while (c--, p->current_bit >>= 1);
  c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (!(p->is_chan_on & p->current_bit))
      goto found_channel;
  } while (c--, p->current_bit >>= 1);
  assert(0);  // unreachable
found_channel:
  p->sfx_channel_index2 = p->sfx_channel_index = c->index * 2;
  p->sfx_channel_bit = p->current_bit;
  p->is_chan_on |= p->current_bit;
  if (p->sfx_play_echo_flag)
    p->sfx_channels_echo_mask2 |= p->current_bit;
  Sfx_MaybeDisableEcho(p);
  return c;
}

static void Port2_HandleCmd(SpcPlayer *p) {
  while (p->new_value_from_snes[2] != 0 && p->is_chan_on != 0xff) {
    Channel *c = Port2_AllocateChan(p);
    c->sfx_pan = p->new_value_from_snes[2] & 0xc0;
    c->sfx_which_sound = p->new_value_from_snes[2] & 0x3f;
    c->sfx_arr_countdown = 3;
    c->pitch_envelope_num_ticks = 0;
    p->port2_active |= p->current_bit;
    Dsp_Write(p, KOF, p->current_bit);
    p->new_value_from_snes[2] = p->ram[0x189e + c->sfx_which_sound - 1];
  }
}


static void Port3_StartNewSound(SpcPlayer *p) {
  p->port3_current_bit = p->port3_active;
  if (!p->port3_current_bit)
    return;
  Channel *c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (Asl(&p->port3_current_bit)) {
      p->sfx_channel_index = c->index * 2;
      p->dsp_register_index = c->index * 16;
      p->sfx_start_arg_pan = c->sfx_pan;
      if (!c->sfx_arr_countdown) {
        if (c->sfx_which_sound)
          Sfx_ChannelTick(p, c, true);
      } else {
        p->sfx_channel_index = c->index * 2;
        if (!--c->sfx_arr_countdown) {
          p->sfx_sound_ptr_cur = c->sfx_sound_ptr = WORD(p->ram[0x191C + (c->sfx_which_sound - 1) * 2]);
          Sfx_ChannelTick(p, c, false);
        }
      }
    }
  } while (c--, p->current_bit >>= 1);
}

static Channel *Port3_AllocateChan(SpcPlayer *p) {
  p->sfx_play_echo_flag = p->ram[0x19d8 + (p->new_value_from_snes[3] & 0x3f)];
  Channel *c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (p->port3_active & p->current_bit && c->sfx_which_sound + c->sfx_pan == p->new_value_from_snes[3])
      goto found_channel;
  } while (c--, p->current_bit >>= 1);
  c = &p->channel[7];
  p->current_bit = 0x80;
  do {
    if (!(p->is_chan_on & p->current_bit))
      goto found_channel;
  } while (c--, p->current_bit >>= 1);
  assert(0);  // unreachable
found_channel:
  p->sfx_channel_index2 = p->sfx_channel_index = c->index * 2;
  p->sfx_channel_bit = p->current_bit;
  p->is_chan_on |= p->current_bit;
  if (p->sfx_play_echo_flag)
    p->sfx_channels_echo_mask2 |= p->current_bit;
  Sfx_MaybeDisableEcho(p);
  return c;
}

static void Port3_HandleCmd(SpcPlayer *p) {
  while (p->new_value_from_snes[3] != 0 && p->is_chan_on != 0xff) {
    Channel *c = Port3_AllocateChan(p);
    c->sfx_pan = p->new_value_from_snes[3] & 0xc0;
    c->sfx_which_sound = p->new_value_from_snes[3] & 0x3f;
    c->sfx_arr_countdown = 3;
    c->pitch_envelope_num_ticks = 0;
    p->port3_active |= p->current_bit;
    Dsp_Write(p, KOF, p->current_bit);
    p->new_value_from_snes[3] = p->ram[0x199a + c->sfx_which_sound - 1];
  }
}

static void ReadPortFromSnes(SpcPlayer *p, int port) {
  uint8 old = p->last_value_from_snes[port];
  p->last_value_from_snes[port] = p->input_ports[port];
  if (p->input_ports[port] != old)
    p->new_value_from_snes[port] = p->input_ports[port];
  else
    p->new_value_from_snes[port] = 0;
}

static void Spc_Loop_Part1(SpcPlayer *p) {
  static const uint8 kRegAddrs0[10] = {EVOLL, EVOLR, EFB, EON, FLG, KON, KOF, NON, PMON, KOF};

  Dsp_Write(p, KOF, p->key_OFF);
  Dsp_Write(p, PMON, p->reg_PMON);
  Dsp_Write(p, NON, p->reg_NON);
  Dsp_Write(p, KOF, 0);
  Dsp_Write(p, KON, p->key_ON);
  if (!(p->echo_stored_time & 0x80)) {
    Dsp_Write(p, FLG, p->reg_FLG);
    if (p->echo_stored_time == p->echo_parameter_EDL) {
      Dsp_Write(p, EON, p->reg_EON);
      Dsp_Write(p, EFB, p->reg_EFB);
      Dsp_Write(p, EVOLR, p->echo_volume_right >> 8);
      Dsp_Write(p, EVOLL, p->echo_volume_left >> 8);
    }
  }
  p->key_OFF = p->key_ON = 0;
}

static void Spc_Loop_Part2(SpcPlayer *p, uint8 ticks) {
  int t = p->sfx_timer_accum + (uint8)(ticks * 0x38);
  p->sfx_timer_accum = t;
  if (t >= 256) {
    Port1_StartNewSound(p);
    Port1_HandleCmd(p);
    ReadPortFromSnes(p, 1);

    Port2_StartNewSound(p);
    Port2_HandleCmd(p);
    ReadPortFromSnes(p, 2);

    Port3_StartNewSound(p);
    Port3_HandleCmd(p);
    ReadPortFromSnes(p, 3);

    if (p->echo_stored_time != p->echo_parameter_EDL && !(++p->echo_fract_incr & 1))
      p->echo_stored_time++;
  }

  t = p->main_tempo_accum + (uint8)(ticks * HIBYTE(p->tempo));
  p->main_tempo_accum = t;
  if (t >= 256) {
    Port0_HandleMusic(p);
    ReadPortFromSnes(p, 0);
  } else if (p->port_to_snes[0]) {
    Channel *c = p->channel;
    for (p->cur_chan_bit = 1; p->cur_chan_bit != 0; p->cur_chan_bit <<= 1, c++) {
      if (HIBYTE(c->pattern_order_ptr_for_chan))
        HandlePanAndSweep(p, c);
    }
  }
}

static void Interrupt_Reset(SpcPlayer *p) {
  dsp_reset(p->dsp);

  memset(&p->new_value_from_snes, 0, sizeof(SpcPlayer) - offsetof(SpcPlayer, new_value_from_snes));
  for (int i = 0; i < 8; i++)
    p->channel[i].index = i;
  SetupEchoParameter_EDL(p, 1);
  p->reg_FLG |= 0x20;
  Dsp_Write(p, MVOLL, 0x60);
  Dsp_Write(p, MVOLR, 0x60);
  Dsp_Write(p, DIR, 0x3c);
  HIBYTE(p->tempo) = 16;
  p->timer_cycles = 0;
}

SpcPlayer *SpcPlayer_Create() {
  SpcPlayer *p = (SpcPlayer *)malloc(sizeof(SpcPlayer));
  p->dsp = dsp_init(p->ram);
  p->reg_write_history = 0;
  return p;
}

void SpcPlayer_Initialize(SpcPlayer *p) {
  Interrupt_Reset(p);
  Spc_Loop_Part1(p);
}

void SpcPlayer_CopyVariablesToRam(SpcPlayer *p) {
  Channel *c = p->channel;
  for (int i = 0; i < 8; i++, c++) {
    for (const MemMap *m = &kChannel_Maps[0]; m != &kChannel_Maps[countof(kChannel_Maps)]; m++)
      memcpy(&p->ram[(m->org_off & 0x7fff) + i * 2], (uint8 *)c + m->off, m->org_off & 0x8000 ? 2 : 1);
  }
  for (const MemMapSized *m = &kSpcPlayer_Maps[0]; m != &kSpcPlayer_Maps[countof(kSpcPlayer_Maps)]; m++)
    memcpy(&p->ram[m->org_off], (uint8 *)p + m->off, m->size);
}

void SpcPlayer_CopyVariablesFromRam(SpcPlayer *p) {
  Channel *c = p->channel;
  for (int i = 0; i < 8; i++, c++) {
    for (const MemMap *m = &kChannel_Maps[0]; m != &kChannel_Maps[countof(kChannel_Maps)]; m++)
      memcpy((uint8 *)c + m->off, &p->ram[(m->org_off & 0x7fff) + i * 2], m->org_off & 0x8000 ? 2 : 1);
  }
  for (const MemMapSized *m = &kSpcPlayer_Maps[0]; m != &kSpcPlayer_Maps[countof(kSpcPlayer_Maps)]; m++)
    memcpy((uint8 *)p + m->off, &p->ram[m->org_off], m->size);
}


void SpcPlayer_GenerateSamples(SpcPlayer *p) {
  assert(p->timer_cycles <= 64);

  assert(p->dsp->sampleOffset <= 534);

  for (;;) {
    if (p->timer_cycles >= 64) {
      Spc_Loop_Part2(p, p->timer_cycles >> 6);
      Spc_Loop_Part1(p);
      p->timer_cycles &= 63;
    }

    // sample rate 32000
    int n = 534 - p->dsp->sampleOffset;
    if (n > (64 - p->timer_cycles))
      n = (64 - p->timer_cycles);

    p->timer_cycles += n;

    for (int i = 0; i < n; i++)
      dsp_cycle(p->dsp);

    if (p->dsp->sampleOffset == 534)
      break;
  }
}

void SpcPlayer_Upload(SpcPlayer *p, const uint8_t *data) {
  Dsp_Write(p, EVOLL, 0);
  Dsp_Write(p, EVOLR, 0);
  Dsp_Write(p, KOF, 0xff);

  for (;;) {
    int numbytes = *(uint16 *)(data);
    if (numbytes == 0)
      break;
    int target = *(uint16 *)(data + 2);
    data += 4;
    do {
      p->ram[target++ & 0xffff] = *data++;
    } while (--numbytes);
  }
  p->pause_music_ctr = 0;
  p->port_to_snes[0] = 0;
  p->port1_active = 0;
  p->port2_active = 0;
  p->port3_active = 0;
  p->is_chan_on = 0;
  p->input_ports[0] = p->input_ports[1] = p->input_ports[2] = p->input_ports[3] = 0;
}

// =======================================

static DspRegWriteHistory my_write_hist;
static SpcPlayer my_spc, my_spc_snapshot;
static int loop_ctr;

bool CompareSpcImpls(SpcPlayer *p, SpcPlayer *p_org, Apu *apu) {
  SpcPlayer_CopyVariablesToRam(p);
  memcpy(p->ram + 0x18, apu->ram + 0x18, 2); //lfsr_value
  memcpy(p->ram + 0x110, apu->ram + 0x110, 256-16);  // stack
  memcpy(p->ram + 0xf1, apu->ram + 0xf1, 15);  // dsp regs
  memcpy(p->ram + 0x10, apu->ram + 0x10, 8);  // temp regs
  p->ram[0x44] = apu->ram[0x44]; // chn
  int errs = 0;
  for (int i = 0; i != 0xc000; i++) {  // skip compare echo etc
    if (p->ram[i] != apu->ram[i]) {
      if (errs < 16) {
        if (errs == 0)
          printf("@%d\n", loop_ctr);
        printf("%.4X: %.2X != %.2X (mine, theirs) orig %.2X\n", i, p->ram[i], apu->ram[i], p_org->ram[i]);
        errs++;
      }
    }
  }

  int n = my_write_hist.count < apu->hist.count ? apu->hist.count : my_write_hist.count;
  for (int i = 0; i != n; i++) {
    if (i >= my_write_hist.count || i >= apu->hist.count || my_write_hist.addr[i] != apu->hist.addr[i] || my_write_hist.val[i] != apu->hist.val[i]) {
      if (errs == 0)
        printf("@%d\n", loop_ctr);
      printf("%d: ", i);
      if (i >= my_write_hist.count) printf("[??: ??]"); else printf("[%.2x: %.2x]", my_write_hist.addr[i], my_write_hist.val[i]);
      printf(" != ");
      if (i >= apu->hist.count) printf("[??: ??]"); else printf("[%.2x: %.2x]", apu->hist.addr[i], apu->hist.val[i]);
      printf("\n");
      errs++;
    }
  }

  if (errs) {
    printf("Total %d errors\n", errs);
    return false;
  }

  apu->hist.count = 0;
  my_write_hist.count = 0;
  loop_ctr++;
  return true;
}

void RunAudioPlayer() {
  if(SDL_Init(SDL_INIT_AUDIO) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return;
  }
  
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID device;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 44100;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL; // use queue
  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(device == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return;
  }
  int16_t* audioBuffer = (int16_t*)malloc(735 * 4); // *2 for stereo, *2 for sizeof(int16)
  SDL_PauseAudioDevice(device, 0);

  memset(&my_spc, 0, sizeof(my_spc));
  FILE *f = fopen("lightworld.spc", "rb");
  fread(my_spc.ram, 1, 65536, f);
  fclose(f);

  my_spc.reg_write_history = &my_write_hist;

  bool run_both = 0;// false;// false;

  if (!run_both) {
    SpcPlayer *p = &my_spc;
    Dsp *dsp = dsp_init(p->ram);
    dsp_reset(dsp);
    p->dsp = dsp;
    SpcPlayer_Initialize(p);

    p->input_ports[0] = 4;

    for (;;) {
      SpcPlayer_GenerateSamples(p);

      int16_t audioBuffer[736 * 2];
      dsp_getSamples(p->dsp, audioBuffer, 736, have.channels);
      SDL_QueueAudio(device, audioBuffer, 736 * 2 * have.channels);
      while (SDL_GetQueuedAudioSize(device) >= 736 * 4 * 3/* 44100 * 4 * 300*/)
        SDL_Delay(1);

    }

  } else {
    SpcPlayer *p = &my_spc;
    Dsp *dsp = dsp_init(p->ram);
    dsp_reset(dsp);
    p->dsp = dsp;

    Apu *apu = apu_init();
    apu_reset(apu);
    apu->spc->pc = 0x800;

    memcpy(apu->ram, my_spc.ram, 65536);

    CompareSpcImpls(&my_spc, &my_spc_snapshot, apu);

    uint64_t cycle_counter = 0;
    int tgt = 0x878;
    uint8 ticks_next = 0;
    bool apu_debug = false;
    bool is_initialize = true;
    for (;;) {
      if (apu_debug && apu->cpuCyclesLeft == 0) {
        char line[80];
        getProcessorStateSpc(apu, line);
        puts(line);
      }

      apu_cycle(apu);

      if (((apu->cycles - 1) & 0x1f) == 0)
        dsp_cycle(p->dsp);


      if (apu->spc->pc == tgt) {
        tgt ^= 0x878 ^ 0x879;
        if (tgt == 0x878) {
          uint8 ticks = ticks_next;
          ticks_next = apu->spc->y;
          my_spc_snapshot = my_spc;
          for (;;) {
            my_write_hist.count = 0;
            if (is_initialize)
              SpcPlayer_Initialize(&my_spc);
            else {
              Spc_Loop_Part2(&my_spc, ticks);
              Spc_Loop_Part1(&my_spc);
            }
            is_initialize = false;
            if (CompareSpcImpls(&my_spc, &my_spc_snapshot,apu))
              break;
            my_spc = my_spc_snapshot;
          }

          if (cycle_counter == 0)
            apu->inPorts[0] = my_spc.input_ports[0] = 2;// 2 + cycle_counter / 1000;
          cycle_counter++;
        }
      }

      if (p->dsp->sampleOffset == 534) {
        int16_t audioBuffer[736 * 2];
        dsp_getSamples(p->dsp, audioBuffer, 736, have.channels);
        SDL_QueueAudio(device, audioBuffer, 736 * 2 * have.channels);
        while (SDL_GetQueuedAudioSize(device) >= 736 * 4 * 3/* 44100 * 4 * 300*/) {
          SDL_Delay(1);
        }
      }
    }
  }
}