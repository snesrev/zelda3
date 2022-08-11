#include <stddef.h>
struct DspRegWriteHistory;
struct Dsp;

struct Channel {
  uint16 pattern_order_ptr_for_chan;
  uint8 note_ticks_left;
  uint8 note_keyoff_ticks_left;
  uint8 subroutine_num_loops;
  uint8 volume_fade_ticks;
  uint8 pan_num_ticks;
  uint8 pitch_slide_length;
  uint8 pitch_slide_delay_left;
  uint8 vibrato_hold_count;
  uint8 vib_depth;
  uint8 tremolo_hold_count;
  uint8 tremolo_depth;
  uint8 vibrato_change_count;
  uint8 note_length;
  uint8 note_gate_off_fixedpt;
  uint8 channel_volume_master;
  uint8 instrument_id;
  uint16 instrument_pitch_base;
  uint16 saved_pattern_ptr;
  uint16 pattern_start_ptr;
  uint8 pitch_envelope_num_ticks;
  uint8 pitch_envelope_delay;
  uint8 pitch_envelope_direction;
  uint8 pitch_envelope_slide_value;
  uint8 vibrato_count;
  uint8 vibrato_rate;
  uint8 vibrato_delay_ticks;
  uint8 vibrato_fade_num_ticks;
  uint8 vibrato_fade_add_per_tick;
  uint8 vibrato_depth_target;
  uint8 tremolo_count;
  uint8 tremolo_rate;
  uint8 tremolo_delay_ticks;
  uint8 channel_transposition;
  uint16 channel_volume;
  uint16 volume_fade_addpertick;
  uint8 volume_fade_target;
  uint8 final_volume;
  uint16 pan_value;
  uint16 pan_add_per_tick;
  uint8 pan_target_value;
  uint8 pan_flag_with_phase_invert;
  uint16 pitch;
  uint16 pitch_add_per_tick;
  uint8 pitch_target;
  uint8 fine_tune;
  uint16 sfx_sound_ptr;
  uint8 sfx_which_sound;
  uint8 sfx_arr_countdown;
  uint8 sfx_note_length_left;
  uint8 sfx_note_length;
  uint8 sfx_pan;
  uint8 index;
};
struct SpcPlayer {
  DspRegWriteHistory *reg_write_history;
  uint8 timer_cycles;
  Dsp *dsp;
  uint8 new_value_from_snes[4];
  uint8 port_to_snes[4];
  uint8 last_value_from_snes[4];
  uint8 counter_sf0c;
  uint16 _always_zero;
  uint16 temp_accum;
  uint8 ttt;
  uint8 did_affect_volumepitch_flag;
  uint16 addr0;
  uint16 addr1;
  uint16 lfsr_value;
  uint8 is_chan_on;
  uint8 fast_forward;
  uint8 sfx_start_arg_pan;
  uint16 sfx_sound_ptr_cur;
  uint16 music_ptr_toplevel;
  uint8 block_count;
  uint8 sfx_timer_accum;
  uint8 chn;
  uint8 key_ON;
  uint8 key_OFF;
  uint8 cur_chan_bit;
  uint8 reg_FLG;
  uint8 reg_NON;
  uint8 reg_EON;
  uint8 reg_PMON;
  uint8 echo_stored_time;
  uint8 echo_parameter_EDL;
  uint8 reg_EFB;
  uint8 global_transposition;
  uint8 main_tempo_accum;
  uint16 tempo;
  uint8 tempo_fade_num_ticks;
  uint8 tempo_fade_final;
  uint16 tempo_fade_add;
  uint16 master_volume;
  uint8 master_volume_fade_ticks;
  uint8 master_volume_fade_target;
  uint16 master_volume_fade_add_per_tick;
  uint8 vol_dirty;
  uint8 percussion_base_id;
  uint16 echo_volume_left;
  uint16 echo_volume_right;
  uint16 echo_volume_fade_add_left;
  uint16 echo_volume_fade_add_right;
  uint8 echo_volume_fade_ticks;
  uint8 echo_volume_fade_target_left;
  uint8 echo_volume_fade_target_right;
  uint8 sfx_channel_index;
  uint8 current_bit;
  uint8 dsp_register_index;
  uint8 echo_channels;
  uint8 byte_3C4;
  uint8 byte_3C5;
  uint8 echo_fract_incr;
  uint8 sfx_channel_index2; 
  uint8 sfx_channel_bit;
  uint8 pause_music_ctr;
  uint8 port2_active;
  uint8 port2_current_bit;
  uint8 port3_active;
  uint8 port3_current_bit;
  uint8 port1_active;
  uint8 port1_current_bit;
  uint8 byte_3E1;
  uint8 sfx_play_echo_flag;
  uint8 sfx_channels_echo_mask2;
  uint8 port1_counter;
  uint8 channel_67_volume;
  uint8 cutk_always_zero;
  uint8 last_written_edl;
  uint8 input_ports[4];
  Channel channel[8];
  uint8 ram[65536]; // rest of ram
};
struct MemMap {
uint16 off, org_off;
};
struct MemMap2 {
uint16 off, org_off, size;
};
const MemMap kChannel_Maps[] = {
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
const MemMap2 kSpcPlayer_Maps[] = {
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

SpcPlayer *SpcPlayer_Create();
void SpcPlayer_GenerateSamples(SpcPlayer *p);
void SpcPlayer_Initialize(SpcPlayer *p);
void SpcPlayer_Upload(SpcPlayer *p, const uint8_t *data);
void SpcPlayer_CopyVariablesFromRam(SpcPlayer *p);
void SpcPlayer_CopyVariablesToRam(SpcPlayer *p);
