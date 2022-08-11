#include "tagalong.h"
#include "sprite.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "snes_regs.h"
#include "dungeon.h"
#include "overworld.h"
#include "ancilla.h"
#include "player.h"
#include "misc.h"

void Tagalong_Func1();
void Tagalong_Func2();
void Tagalong_Type_2_4();
void Tagalong_Type_3_11();
void Tagalong_Draw();
void Tagalong_Type_Other();
void Tagalong_DrawInner(uint8 a, uint16 x, uint16 y);
void Tagalong_HandleMessages();
bool Tagalong_CheckPlayerProximity();
void Tagalong_Draw3();
void Kiki_AbandonDamagedPlayer(int k);

static const uint8 kTagalongFlags[4] = {0x20, 0x10, 0x30, 0x20};
static const uint8 kTagalong_Tab5[15] = {0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8 kTagalong_Tab4[15] = {0, 0, 3, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void Tagalong_Init() {
  tagalong_y_lo[0] = link_y_coord;
  tagalong_y_hi[0] = link_y_coord >> 8;

  tagalong_x_lo[0] = link_x_coord;
  tagalong_x_hi[0] = link_x_coord >> 8;

  tagalong_layerbits[0] = kTagalongFlags[link_is_on_lower_level] >> 2 | (link_direction_facing >> 1);
  timer_tagalong_reacquire = 64;
  tagalong_var2 = 0;
  tagalong_var1 = 0;
  tagalong_var3 = 0;
  tagalong_var4 = 0;
  link_speed_setting = 0;
}

void Tagalong_SpawnFromSprite(int k) {
  tagalong_var5 = 0;
  int y = Sprite_GetY(k) - 6;
  tagalong_y_lo[0] = y, tagalong_y_hi[0] = (y >> 8);
  int x = Sprite_GetX(k) + 1;
  tagalong_x_lo[0] = x, tagalong_x_hi[0] = (x >> 8);
  tagalong_layerbits[0] = kTagalongFlags[link_is_on_lower_level] >> 2 | 1;
  timer_tagalong_reacquire = 64;
  tagalong_var1 = 0;
  tagalong_var2 = 0;
  tagalong_var3 = 0;
  tagalong_var4 = 0;
  link_speed_setting = 0;
  tagalong_var5 = 0;
  super_bomb_going_off = 0;
  Tagalong_GetCloseToPlayer();
}

bool Tagalong_CanWeDisplayMessage() {
  uint8 ps = link_player_handler_state;
  if (ps != kPlayerState_Ground && ps != kPlayerState_Swimming && ps != kPlayerState_StartDash)
    return false;
  uint8 t = button_mask_b_y & 0x80 | link_unk_master_sword | link_item_in_hand | 
            link_position_mode | flag_is_ancilla_to_pick_up | flag_is_sprite_to_pick_up |
            link_state_bits | link_grabbing_wall;
  return t == 0;
}

static const uint8 kTagalong_Tab0[3] = {5, 9, 0xa};
static const uint16 kTagalong_Tab1[3] = {0xdf3, 0x6f9, 0xdf3};
static const uint16 kTagalong_Msg[3] = {0x20, 0x108, 0x11d};

void Tagalong_MainB();

void Tagalong_Main() {
  if (!savegame_tagalong)
    return;
  if (savegame_tagalong == 0xe) {
    Tagalong_HandleMessages();
    return;
  }
  int j = FindInByteArray(kTagalong_Tab0, savegame_tagalong, 3);
  if (j >= 0 && submodule_index == 0 && !(j == 2 && overworld_screen_index & 0x40) && sign16(--word_7E02CD)) {
    if (!Tagalong_CanWeDisplayMessage()) {
      word_7E02CD = 0;
    } else {
      word_7E02CD = kTagalong_Tab1[j];
      dialogue_message_index = kTagalong_Msg[j];
      Main_ShowTextMessage();
    }
  }
  if (j != 0)
    Tagalong_MainB();
}

bool Tagalong_IsFollowing() {
  uint8 main = main_module_index;
  uint8 sub = submodule_index;
  return !flag_is_link_immobilized && sub != 10 && !(main == 9 && sub == 0x23) && !(main == 14 && (sub == 1 || sub == 2));
}

void Tagalong_MainB() {
  int k;

  if (super_bomb_going_off) {
    Tagalong_Func2();
    return;
  }

  if (savegame_tagalong == 12) {
    if (link_auxiliary_state != 0)
      goto label_a;

  } else if (savegame_tagalong == 13) {
    if (link_auxiliary_state == 2 || player_near_pit_state == 2)
      goto label_c;
  } else {
    goto label_a;
  }

  if (submodule_index != 0 || link_auxiliary_state == 1 || (link_state_bits & 0x80) || tagalong_var5 || tagalong_var3 ||
    (int8)tagalong_z[tagalong_var2] > 0 || !(filtered_joypad_L & 0x80))
    goto label_a;

label_c:

  if (savegame_tagalong == 13 && !player_is_indoors) {
    if (link_player_handler_state == kPlayerState_Ether ||
        link_player_handler_state == kPlayerState_Bombos ||
        link_player_handler_state == kPlayerState_Quake)
      goto label_a;
    super_bomb_indicator_unk2 = 3;
    super_bomb_indicator_unk1 = 0xbb;
  }

  super_bomb_going_off = 128;
  timer_tagalong_reacquire = 64;

  k = tagalong_var2;
  saved_tagalong_y = tagalong_y_lo[k] | tagalong_y_hi[k] << 8;
  saved_tagalong_x = tagalong_x_lo[k] | tagalong_x_hi[k] << 8;
  saved_tagalong_floor = link_is_on_lower_level;
  saved_tagalong_indoors = player_is_indoors;
  Tagalong_Func2();
  return;

label_a:
  Tagalong_Func1();
}

void Tagalong_Func1() {
  if (Tagalong_IsFollowing() && (link_x_vel | link_y_vel)) {
    int k = tagalong_var1 + 1;
    if (k == 20)
      k = 0;
    tagalong_var1 = k;
    uint8 z = link_z_coord;
    if (z >= 0xf0)
      z = 0;
    tagalong_z[k] = z;
    uint16 y = link_y_coord - z;
    tagalong_y_lo[k] = y;
    tagalong_y_hi[k] = y >> 8;
    uint16 x = link_x_coord;
    tagalong_x_lo[k] = x;
    tagalong_x_hi[k] = x >> 8;
    uint8 layerbits = link_direction_facing >> 1 | kTagalongFlags[link_is_on_lower_level] >> 2;
    if (link_player_handler_state == kPlayerState_Swimming) {
      layerbits |= 0x20;
    } else {
      if (link_player_handler_state == kPlayerState_Hookshot && related_to_hookshot)
        layerbits |= 0x10;
      if (draw_water_ripples_or_grass)
        layerbits |= (draw_water_ripples_or_grass == 1) ? 0x80 : 0x40;
    }
    tagalong_layerbits[k] = layerbits;
  }

  switch (savegame_tagalong) {
  case 2: case 4:
    Tagalong_Type_2_4();
    return;
  case 3: case 11:
    Tagalong_Type_3_11();
    return;
  case 5: case 14:
    assert(0); // Y is unknown here...
    return;
  default:
    Tagalong_Type_Other();
    return;
  }
}


void Blind_SpawnFromMaidenTagalong(uint16 x, uint16 y) {
  int k = 0;
  sprite_state[k] = 9;
  sprite_type[k] = 206;
  Sprite_SetX(k, x);
  Sprite_SetY(k, y - 16);
  Sprite_LoadProperties(k);
  sprite_delay_aux2[k] = 192;
  sprite_graphics[k] = 21;
  sprite_D[k] = 2;
  sprite_ignore_projectile[k] = 2;
  dung_savegame_state_bits |= 0x2000;
  byte_7E0B69 = 0;
}

bool Tagalong_CheckBlindTriggerRegion() {
  int k = tagalong_var2;
  uint16 x = tagalong_x_hi[k] << 8 | tagalong_x_lo[k];
  uint16 y = tagalong_y_hi[k] << 8 | tagalong_y_lo[k];
  uint16 z = (int8)tagalong_z[k];
  y += z + 12;
  x += 8;
  return abs16(0x1568 - y) < 24 && abs16(0x1980 - x) < 24;
}


void Tagalong_Type_Other() {
  int k;
  if (!Tagalong_IsFollowing()) {
    Tagalong_Draw();
    return;
  }

  Tagalong_HandleMessages();

  if (savegame_tagalong == 10 && link_auxiliary_state && countdown_for_blink) {
    int k = tagalong_var2 + 1 == 20 ? 0 : tagalong_var2 + 1;
    Kiki_AbandonDamagedPlayer(k);
    savegame_tagalong = 0;
    return;
  }

  if (savegame_tagalong == 6 && dungeon_room_index == 0xac && (save_dung_info[101] & 0x100) && Tagalong_CheckBlindTriggerRegion()) {
    int k = tagalong_var2;
    uint16 x = tagalong_x_lo[k] | tagalong_x_hi[k] << 8;
    uint16 y = tagalong_y_lo[k] | tagalong_y_hi[k] << 8;
    savegame_tagalong = 0;
    Blind_SpawnFromMaidenTagalong(x, y);
    BYTE(dung_flag_trapdoors_down)++;
    BYTE(dung_cur_door_pos) = 0;
    BYTE(door_animation_step_indicator) = 0;
    submodule_index = 5;
    music_control = 21;
    return;
  }

  if (!tagalong_var3) {
    if (link_player_handler_state == kPlayerState_Hookshot && related_to_hookshot) {
      tagalong_var3 = 1;
      goto label_e;
    }
  } else {
    if (link_player_handler_state == kPlayerState_Hookshot)
      goto label_e;
    if (tagalong_var7 != tagalong_var2)
      goto label_d;
    tagalong_var3 = 0;
  }

  k = tagalong_var2;
  if ((int8)tagalong_z[k] > 0) {
    if (tagalong_var1 != k)
      goto label_d;
    tagalong_z[k] = 0;
    uint16 y = link_y_coord, x = link_x_coord;
    tagalong_y_lo[k] = y;
    tagalong_y_hi[k] = y >> 8;
    tagalong_x_lo[k] = x;
    tagalong_x_hi[k] = x >> 8;
  }

  if (link_x_vel | link_y_vel) {
label_e:
    uint8 t;
    t = tagalong_var1 - 15;
    if (sign8(t))
      t += 20;
    if (t == tagalong_var2) {
label_d:
      tagalong_var2 = (tagalong_var2 + 1 == 20) ? 0 : tagalong_var2 + 1;
    }
  }
  Tagalong_Draw();
}



void Tagalong_GetCloseToPlayer() {
  for(;;) {
    int k = 9;
    int j = tagalong_var1;
    ancilla_y_lo[k] = tagalong_y_lo[j];
    ancilla_y_hi[k] = tagalong_y_hi[j];
    ancilla_x_lo[k] = tagalong_x_lo[j];
    ancilla_x_hi[k] = tagalong_x_hi[j];

    ProjectSpeedRet pt = Ancilla_ProjectSpeedTowardsPlayer(k, 24);
    ancilla_x_vel[k] = pt.x;
    ancilla_y_vel[k] = pt.y;
    Ancilla_MoveY(k);
    Ancilla_MoveX(k);

    uint16 x = Ancilla_GetX(k);
    uint16 y = Ancilla_GetY(k);
    if (abs16(x - link_x_coord) < 2 && abs16(y - link_y_coord) < 2)
      return;
    k = ++tagalong_var1;
    if (k == 18)
      return;
    tagalong_y_lo[k] = y;
    tagalong_y_hi[k] = y >> 8;
    tagalong_x_lo[k] = x;
    tagalong_x_hi[k] = x >> 8;
    static const uint8 kTagalongLayerBits[4] = {0x20, 0x10, 0x30, 0x20};
    tagalong_layerbits[k] = kTagalongLayerBits[link_is_on_lower_level] >> 2 | 1;
  }
}

void Tagalong_Func2() {
  if (saved_tagalong_indoors != player_is_indoors)
    return;
  if (!link_is_running && !Tagalong_CheckPlayerProximity()) {
    Tagalong_Init();
    saved_tagalong_indoors = player_is_indoors;
    if (savegame_tagalong == 13) {
      super_bomb_indicator_unk2 = 254;
      super_bomb_indicator_unk1 = 0;
    }
    super_bomb_going_off = 0;
    Tagalong_Draw();
  } else {
    if (savegame_tagalong == 13 && !player_is_indoors && !super_bomb_indicator_unk2) {
      AddSuperBombExplosion(0x3a, 0);
      super_bomb_going_off = 0;
    }
    Tagalong_Draw3();
  }
}

void Tagalong_Type_2_4() {
  uint8 t;

  if (!Tagalong_IsFollowing()) {
    Tagalong_Draw();
    return;
  }

  if (link_speed_setting != 4)
    link_speed_setting = 12;

  Tagalong_HandleMessages();

  if (savegame_tagalong == 0) {
    return;
  } else if (savegame_tagalong == 4) {
    if ((int8)tagalong_z[tagalong_var2] > 0 && tagalong_var1 != tagalong_var2) {
      tagalong_var2 = (tagalong_var2 + 1 >= 20) ? 0 : tagalong_var2 + 1;
      Tagalong_Draw();
      return;
    }
  } else {
    // tagalong type 2
    if ((link_auxiliary_state & 1) && link_player_handler_state == kPlayerState_RecoilOther) {
      if (tagalong_var1 != tagalong_var2)
        goto transform;
      assert(0); // X is undefined here
    }

    if (link_auxiliary_state & 2) {
transform:
      savegame_tagalong = kTagalong_Tab4[savegame_tagalong];
      timer_tagalong_reacquire = 64;
      int k = tagalong_var2;
      saved_tagalong_y = tagalong_y_lo[k] | tagalong_y_hi[k] << 8;
      saved_tagalong_x = tagalong_x_lo[k] | tagalong_x_hi[k] << 8;
      saved_tagalong_floor = link_is_on_lower_level;
      Tagalong_Type_3_11();
      return;
    }
  }

  if (link_x_vel | link_y_vel) {
    t = tagalong_var1 - 20;
    if (sign8(t))
      t += 20;
    if (t == tagalong_var2) {
      tagalong_var2 = (tagalong_var2 + 1 >= 20) ? 0 : tagalong_var2 + 1;
    }
  } else if ((frame_counter & 3) == 0 && tagalong_var1 != tagalong_var2) {
    t = tagalong_var1 - 9;
    if (sign8(t))
      t += 20;
    if (t != tagalong_var2) {
      tagalong_var2 = (tagalong_var2 + 1 >= 20) ? 0 : tagalong_var2 + 1;
    }
  }
  Tagalong_Draw();
}

void Tagalong_Type_3_11() {
  link_speed_setting = 16;
  if (!link_is_running && !link_auxiliary_state && link_player_handler_state != kPlayerState_Swimming) {
    link_speed_setting = 0;
    if (link_player_handler_state != kPlayerState_Hookshot && !Tagalong_CheckPlayerProximity()) {
      Tagalong_Init();
      savegame_tagalong = kTagalong_Tab5[savegame_tagalong];
      return;
    }
  }
  Tagalong_Draw3();
}

void Tagalong_Draw3() {
  oam_priority_value = kTagalongFlags[saved_tagalong_floor] << 8;
  uint8 a = (savegame_tagalong == 12 || savegame_tagalong == 13) ? 2 : 1;
  Tagalong_DrawInner(a, saved_tagalong_x, saved_tagalong_y);
}

bool Tagalong_CheckPlayerProximity() {
  if (!sign8(--timer_tagalong_reacquire))
    return true;
  timer_tagalong_reacquire = 0;
  if ((uint16)(saved_tagalong_y - 1) >= link_y_coord ||
      (uint16)(saved_tagalong_y + 19) < link_y_coord ||
      (uint16)(saved_tagalong_x - 1) >= link_x_coord ||
      (uint16)(saved_tagalong_x + 19) < link_x_coord)
    return true;
  return false;
}

struct TagalongMessageInfo {
  uint16 y, x, bit, msg, tagalong;
};
static const TagalongMessageInfo kTagalong_IndoorInfos[12] = {
  {0x1ef0, 0x288, 1, 0x99, 4},
  {0x1e58, 0x2f0, 2, 0x9a, 4},
  {0x1ea8, 0x3b8, 4, 0x9b, 4},
  {0xcf8, 0x25b, 1, 0x21, 1},
  {0xcf8, 0x39d, 2, 0x21, 1},
  {0xc78, 0x238, 4, 0x21, 1},
  {0xa30, 0x2f8, 1, 0x22, 1},
  {0x178, 0x550, 1, 0x23, 1},
  {0x168, 0x4f8, 2, 0x2a, 1},
  {0x1bd8, 0x16fc, 1, 0x124, 6},
  {0x1520, 0x167c, 1, 0x124, 6},
  {0x5ac, 0x4fc, 1, 0x29, 1},
};

static const TagalongMessageInfo kTagalong_OutdoorInfos[5] = {
  {0x3c0, 0x730, 1, 0x9d, 4},
  {0x648, 0xf50, 0, 0xffff, 0xa},
  {0x6c8, 0xd78, 1, 0xffff, 0xa},
  {0x688, 0xc78, 2, 0xffff, 0xa},
  {0xe8, 0x90, 0, 0x28, 0xe},
};

static const uint8 kTagalong_IndoorOffsets[8] = {0, 3, 6, 7, 9, 10, 11, 12};
static const uint8 kTagalong_OutdoorOffsets[4] = {0, 1, 4, 5};

static const uint16 kTagalong_IndoorRooms[7] = {0xf1, 0x61, 0x51, 2, 0xdb, 0xab, 0x22};
static const uint16 kTagalong_OutdoorRooms[3] = {3, 0x5e, 0};

bool Tagalong_CheckTextTriggerProximity(const TagalongMessageInfo *info) {
  uint16 x = link_x_coord + 12 - (info->x + 8);
  uint16 y = link_y_coord + 12 - (info->y + 8);
  if (sign16(x))
    x = -x;
  if (sign16(y))
    y = -y;
  return x < 24 && y < 28;
}


int Kiki_TransitionFromTagalong(int k) {
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xb6, &info);
  if (j >= 0) {
    sprite_head_dir[j] = tagalong_layerbits[k] & 3;
    sprite_D[j] = tagalong_layerbits[k] & 3;
    int x = tagalong_x_hi[k] << 8 | tagalong_x_lo[k];
    int y = tagalong_y_hi[k] << 8 | tagalong_y_lo[k];
    Sprite_SetX(j, x + 2);
    Sprite_SetY(j, y + 2);
    sprite_floor[j] = link_is_on_lower_level;
    sprite_ignore_projectile[j] = 1;
    sprite_floor[j] = 2;
    link_speed_setting = 0;
  }
  return j;
}


void Kiki_InitiatePalaceOpeningProposal(int k) {
  int j = Kiki_TransitionFromTagalong(k);
  sprite_subtype2[j] = 1;
  savegame_tagalong = 0;
}

void Kiki_InitiateFirstBeggingSequence(int k) {
  int j = Kiki_TransitionFromTagalong(k);
  sprite_subtype2[j] = 2;
}

void Kiki_AbandonDamagedPlayer(int k) {
  int j = Kiki_TransitionFromTagalong(k);
  sprite_z[j] = 1;
  sprite_z_vel[j] = 16;
  sprite_subtype2[j] = 3;
  savegame_tagalong = 0;
}



void Tagalong_HandleMessages() {
  if (submodule_index)
    return;

  const TagalongMessageInfo *tmi, *tmi_end;
  if (player_is_indoors) {
    int j = FindInWordArray(kTagalong_IndoorRooms, dungeon_room_index, 7);
    if (j < 0)
      return;
    tmi = kTagalong_IndoorInfos + kTagalong_IndoorOffsets[j];
    tmi_end = kTagalong_IndoorInfos + kTagalong_IndoorOffsets[j + 1];
  } else {
    int j = FindInWordArray(kTagalong_OutdoorRooms, overworld_screen_index, 3);
    if (j < 0)
      return;
    tmi = kTagalong_OutdoorInfos + kTagalong_OutdoorOffsets[j];
    tmi_end = kTagalong_OutdoorInfos + kTagalong_OutdoorOffsets[j + 1];
  }
  int st = (tagalong_var2 + 1 >= 20) ? 0 : tagalong_var2 + 1;
  do {
    if (tmi->tagalong == savegame_tagalong && Tagalong_CheckTextTriggerProximity(tmi)) {
      if (tmi->bit & tagalong_event_flags)
        return;
      tagalong_event_flags |= tmi->bit;
      dialogue_message_index = tmi->msg;
      if (tmi->msg == 0xffff) {
        if (!(tmi->bit & 3))
          Kiki_InitiatePalaceOpeningProposal(st);
        else if (!(save_ow_event_info[BYTE(overworld_screen_index)] & 1))
          Kiki_InitiateFirstBeggingSequence(st);
        return;
      }
      if (tmi->msg == 0x9d) {
        OldMountainMan_TransitionFromTagalong(st);
      } else if (tmi->msg == 0x28) {
        savegame_tagalong = 0;
      }
      Main_ShowTextMessage();
      return;
    }
  } while (++tmi != tmi_end);
}


void Tagalong_Draw() {
  if (tagalong_var5)
    return;
  oam_priority_value =
    ((tagalong_z[tagalong_var2] && !player_is_indoors) ? 0x20 :
    (submodule_index == 14) ? kTagalongFlags[link_is_on_lower_level] :
                              (tagalong_layerbits[tagalong_var2] & 0xc) << 2) << 8;
  int k = tagalong_var2;
  if (sign8(k))
    k = 0;
  uint16 x = tagalong_x_lo[k] | tagalong_x_hi[k] << 8;
  uint16 y = tagalong_y_lo[k] | tagalong_y_hi[k] << 8;
  uint8 a = tagalong_layerbits[k];
  Tagalong_DrawInner(a, x, y);
}

struct TagalongSprXY {
  int8 y1, x1, y2, x2;
};

struct TagalongDmaFlags {
  uint8 dma6, dma7;
  uint8 flags;
};

static const TagalongSprXY kTagalongDraw_SprXY[56] = {
  {-2, 0, 0, 0},
  {-2, 0, 0, 0},
  {-2, 0, 0, 0},
  {-2, 0, 0, 0},
  {-1, 0, 1, 0},
  {-1, 0, 1, 0},
  {-1, 0, 1, 0},
  {-1, 0, 1, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {1, 0, 0, 0},
  {1, 0, 0, 0},
  {1, 0, 0, 0},
  {1, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, -3, 0, 0},
  {0, 3, 0, 0},
  {1, 0, 0, 0},
  {1, 0, 0, 0},
  {1, -3, 1, 0},
  {1, 3, 1, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {-1, 0, 0, 0},
  {-1, 0, 0, 0},
  {-1, 0, 0, 0},
  {-1, 0, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {2, 0, 0, 0},
  {2, 0, 0, 0},
  {2, -1, 0, 0},
  {2, 1, 0, 0},
  {3, 0, 1, 0},
  {3, 0, 1, 0},
  {3, -1, 1, 0},
  {3, 1, 1, 0},
};

static const TagalongDmaFlags kTagalongDmaAndFlags[16] = {
  {0x20, 0xc0, 0x00},
  {0x00, 0xa0, 0x00},
  {0x40, 0x60, 0x00},
  {0x40, 0x60, 0x44},
  {0x20, 0xc0, 0x04},
  {0x00, 0xa0, 0x04},
  {0x40, 0x80, 0x00},
  {0x40, 0x80, 0x44},
  {0x20, 0xe0, 0x00},
  {0x00, 0xe0, 0x00},
  {0x40, 0xe0, 0x00},
  {0x40, 0xe0, 0x44},
  {0x20, 0xe0, 0x04},
  {0x00, 0xe0, 0x04},
  {0x40, 0xe0, 0x04},
  {0x40, 0xe0, 0x40},
};

static const uint8 kTagalongDraw_Pals[14] = {0, 4, 4, 4, 4, 0, 7, 4, 4, 3, 4, 4, 4, 4};
static const uint16 kTagalongDraw_Offs[14] = {0, 0, 0x80, 0x80, 0x80, 0, 0, 0xc0, 0xc0, 0x100, 0x180, 0x180, 0x140, 0x140};
static const uint8 kTagalongDraw_SprInfo0[24] = {
  0xd8, 0x24, 0xd8, 0x64, 0xd9, 0x24, 0xd9, 0x64, 0xda, 0x24, 0xda, 0x64, 0xc8, 0x22, 0xc8, 0x62, 
  0xc9, 0x22, 0xc9, 0x62, 0xca, 0x22, 0xca, 0x62, 
};
static const uint16 kTagalongDraw_SprOffs0[2] = {0x170, 0xc0};
static const uint16 kTagalongDraw_SprOffs1[2] = {0x1c0, 0x110};
void Tagalong_DrawInner(uint8 ain, uint16 xin, uint16 yin) {

  uint8 yt = 0, av = 0;
  uint8 sc = 0;

  if ((ain >> 2 & 8) && (savegame_tagalong == 6 || savegame_tagalong == 1)) {
    yt = 8;
    if (swimcoll_var7[0] | swimcoll_var7[1])
      av = (frame_counter >> 1) & 4;
    else
      av = (frame_counter >> 2) & 4;
  } else if (submodule_index == 8 || submodule_index == 14 || submodule_index == 16) {
    av = link_is_running ? (frame_counter & 4) : ((frame_counter >> 1) & 4);
  } else if (savegame_tagalong == 11) {
    av = (frame_counter >> 1) & 4;
  } else if ((savegame_tagalong == 12 || savegame_tagalong == 13) && super_bomb_going_off || flag_is_link_immobilized || 
             submodule_index == 10 || main_module_index == 9 && submodule_index == 0x23 ||
             main_module_index == 14 && (submodule_index == 1 || submodule_index == 2) ||
             (link_y_vel | link_x_vel) == 0) {
    av = sc = 4;
  } else {
    av = link_is_running ? (frame_counter & 4) : ((frame_counter >> 1) & 4);
  }
  uint8 frame = (ain & 3) + av + yt;

  int spr_offs = 
    (link_y_coord == yin && !(ain & 3) || link_y_coord < yin) ? 
      kTagalongDraw_SprOffs0[sort_sprites_setting] >> 2: 
      kTagalongDraw_SprOffs1[sort_sprites_setting] >> 2;
  oam_ext_cur_ptr = 0xa20 + spr_offs;
  oam_cur_ptr = 0x800 + spr_offs * 4;
  
  OamEnt *oam = GetOamCurPtr();
  uint16 scrolly = yin - BG2VOFS_copy2;
  uint16 scrollx = xin - BG2HOFS_copy2;

  const uint8 *sk = kTagalongDraw_SprInfo0;
  if (savegame_tagalong == 1 || savegame_tagalong == 6 || !(ain & 0x20)) {
    if (!(ain & 0xc0))
      goto skip_first_sprites;
    if ((ain & 0x80) || (sk += 12, sc == 0))
      goto incr;
    byte_7E02D7 = 0;
  } else {
incr:
    if (!(frame_counter & 7) && ++byte_7E02D7 == 3)
      byte_7E02D7 = 0;
  }
  sk += byte_7E02D7 * 4;
  {
    uint16 y = scrolly + 16, x = scrollx, ext = 0;
    oam->x = x;
    oam->y = (uint16)(x + 0x80) < 0x180 && (ext = x >> 8 & 1, (uint16)(y + 0x10) < 0x100) ? y : 0xf0;
    oam->charnum = sk[0];
    oam->flags = sk[1];
    bytewise_extended_oam[oam - oam_buf] = ext;
    oam++;

    ext = 0, x += 8;
    oam->x = x;
    oam->y = (uint16)(x + 0x80) < 0x180 && (ext = x >> 8 & 1, (uint16)(y + 0x10) < 0x100) ? y : 0xf0;
    oam->charnum = sk[2];
    oam->flags = sk[3];
    bytewise_extended_oam[oam - oam_buf] = ext;
    oam++;
  }
skip_first_sprites:

  uint8 pal = kTagalongDraw_Pals[savegame_tagalong];
  if (pal == 7 && overworld_palette_swap_flag)
    pal = 0;

  if (savegame_tagalong == 13 && super_bomb_indicator_unk2 == 1)
    pal = (frame_counter & 7);

  const TagalongSprXY *sprd = kTagalongDraw_SprXY + frame + (kTagalongDraw_Offs[savegame_tagalong] >> 3);
  const TagalongDmaFlags *sprf = kTagalongDmaAndFlags + frame;

  if (savegame_tagalong != 12 && savegame_tagalong != 13) {
    uint16 y = scrolly + sprd->y1, x = scrollx + sprd->x1, ext = 0;
    oam->x = x;
    oam->y = (uint16)(x + 0x80) < 0x180 && (ext = x >> 8 & 1, (uint16)(y + 0x10) < 0x100) ? y : 0xf0;
    oam->charnum = 0x20;
    oam->flags = (sprf->flags & 0xf0) | pal << 1 | (oam_priority_value >> 8);
    bytewise_extended_oam[oam - oam_buf] = 2 | ext;
    oam++;
    BYTE(dma_var6) = sprf->dma6;
  }
  {
    uint16 y = scrolly + sprd->y2 + 8, x = scrollx + sprd->x2, ext = 0;
    oam->x = x;
    oam->y = (uint16)(x + 0x80) < 0x180 && (ext = x >> 8 & 1, (uint16)(y + 0x10) < 0x100) ? y : 0xf0;
    oam->charnum = 0x22;
    oam->flags = ((sprf->flags & 0xf) << 4) | pal << 1 | (oam_priority_value >> 8);
    bytewise_extended_oam[oam - oam_buf] = 2 | ext;
    BYTE(dma_var7) = sprf->dma7;
  }
}

void Tagalong_Disable() {
  if (savegame_tagalong == 9 || savegame_tagalong == 10)
    savegame_tagalong = 0;
}