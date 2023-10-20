from ast import literal_eval as make_tuple
import sys
import text_compression
import util
from util import get_bytes, get_words, get_byte, get_word, get_int8, get_int16, cache
import tables
import yaml
import extract_music
import os
import sprite_sheets

def print_map32_to_map16(f):
  for i in range(2218):
    def getit(ea):
      ov = [get_byte(ea + j) for j in range(6)]
      res=[0]*4
      res[0] = ov[0] | (ov[4] >> 4) << 8
      res[1] = ov[1] | (ov[4] & 0xf) << 8
      res[2] = ov[2] | (ov[5] >> 4) << 8
      res[3] = ov[3] | (ov[5] & 0xf) << 8
      return res
    t0 = getit(0x838000 + i * 6)
    t1 = getit(0x83b400 + i * 6)
    t2 = getit(0x848000 + i * 6)
    t3 = getit(0x84b400 + i * 6)
    for j in range(4):
      print('%5d: %4d, %4d, %4d, %4d' % (i * 4 + j, t0[j], t1[j], t2[j], t3[j]), file = f)

@cache
def get_exit_datas():
  r = {}
  for i in range(79):
    room = get_word(0x82dd8a + i * 2)
    screen_index = get_byte(0x82DE28 + i)
    load_offs = get_word(0x82DE77 + i * 2)
    scroll_y = get_word(0x82DF15 + i * 2)
    scroll_x = get_word(0x82DFB3 + i * 2)
    pos_y = get_word(0x82E051 + i * 2)
    pos_x = get_word(0x82E0EF + i * 2)
    camera_y = get_word(0x82E18D + i * 2)
    camera_x = get_word(0x82E22B + i * 2)
    unk1 = get_int8(0x82E2C9 + i)
    unk3 = get_int8(0x82E318 + i)
    ndoor = get_word(0x82E367 + i * 2)
    fdoor = get_word(0x82E405 + i * 2)
    base_x = (screen_index & 7) << 9
    base_y = (screen_index & 56) << 6
    y = {
      'index' : i,
      'room' : room,
      'xy' : [pos_x - base_x, pos_y - base_y],
      'scroll_xy' : [scroll_x - base_x, scroll_y - base_y],
      'camera_xy' : [camera_x - base_x, camera_y - base_y],
    }
    y['load_xy'] = [((load_offs >> 1) - (y['scroll_xy'][0] >> 4)) & 0x3f, (load_offs >> 7) - (y['scroll_xy'][1] >> 4) & 0x3f]
    y['unk'] = [unk1, unk3]
    def get_special_exit_info(room_index):
      return {
        'dir' : get_byte(0x82E801  + room_index - 0x180) >> 1,
        'spr_gfx' : get_byte(0x82E811  + room_index - 0x180),
        'aux_gfx' : get_byte(0x82E821  + room_index - 0x180),
        'pal_bg' : get_byte(0x82E831  + room_index - 0x180),
        'pal_spr' : get_byte(0x82E841  + room_index - 0x180),
        'top' : get_word(0x82e6e1 + (room_index - 0x180) * 2),
        'bottom' : get_word(0x82e701 + (room_index - 0x180) * 2),
        'left' : get_word(0x82e721 + (room_index - 0x180) * 2),
        'right' : get_word(0x82e741 + (room_index - 0x180) * 2),
        'left_edge_of_map' : get_word(0x82E7E1 + (room_index - 0x180) * 2),
        'unk4' : get_int16(0x82e761 + (room_index - 0x180) * 2),
        'unk6' : get_int16(0x82e781 + (room_index - 0x180) * 2),
        'unk5' : get_int16(0x82e7a1 + (room_index - 0x180) * 2),
        'unk7' : get_int16(0x82e7c1 + (room_index - 0x180) * 2),
      }

    if room >= 0x180 and room < 0x190:
      y['special_exit'] = get_special_exit_info(room)

    if ndoor != 0:
      assert fdoor == 0
      y['door'] = ['bombable' if ndoor & 0x8000 else 'wooden', (ndoor & 0x7e) >> 1, (ndoor & 0x3f80) >> 7]
    if fdoor != 0:
      y['door'] = ['palace' if fdoor & 0x8000 else 'sanctuary', (fdoor & 0x7e) >> 1, (fdoor & 0x3f80) >> 7]
    r.setdefault(screen_index, []).append(y)
  return r

def get_loadoffs(c, d):
  x, y = c[0] >> 4, c[1] >> 4
  x += d[0]
  y += d[1]
  return (y&0x3f) << 7 | (x&0x3f) << 1

@cache
def get_ow_travel_infos():
  r = {}
  for i in range(17):
    screen_index = get_word(0x82EAE5 + i * 2)
    load_offs = get_word(0x82EB07 + i * 2)
    scroll_y = get_word(0x82EB29 + i * 2)
    scroll_x = get_word(0x82EB4B + i * 2)
    pos_y = get_word(0x82EB6D + i * 2)
    pos_x = get_word(0x82EB8F + i * 2)
    camera_y = get_word(0x82EBB1 + i * 2)
    camera_x = get_word(0x82EBD3 + i * 2)
    unk1 = get_int8(0x82EBF5 + i * 2)
    unk3 = get_int8(0x82EC17 + i * 2)
    base_x = (screen_index & 7) << 9
    base_y = (screen_index & 56) << 6
    y = {}
    if i < 9:
      y['bird_travel_id'] = i
    else:
      y['whirlpool_src_area'] = get_word(0x82ECF8 + (i - 9) * 2)
    y['xy'] = [pos_x - base_x, pos_y - base_y]
    y['scroll_xy'] = [scroll_x - base_x, scroll_y - base_y]
    y['camera_xy'] = [camera_x - base_x, camera_y - base_y]
    y['load_xy'] = [((load_offs >> 1) - (y['scroll_xy'][0] >> 4)) & 0x3f, (load_offs >> 7) - (y['scroll_xy'][1] >> 4) & 0x3f]

    t0 = get_loadoffs(y['scroll_xy'], y['load_xy'])
    assert t0 == load_offs, (t0 & 0x7f, load_offs & 0x7f)
    
    y['unk'] = [unk1, unk3]
    r.setdefault(screen_index, []).append(y)
  return r

@cache
def get_ow_entrance_info():
  r = {}
  for i in range(129):
    area = get_word(0x9BB96F + i * 2)
    pos = get_word(0x9BBA71 + i * 2)
    entrance_id = get_byte(0x9BBB73 + i)
    r.setdefault(area, []).append({'index' : i, 'x' : (pos >> 1) & 0x3f, 'y' : (pos >> 7) & 0x3f, 'entrance_id' : entrance_id})
  return r

@cache
def get_hole_infos():
  r = {}
  for i in range(19):
    pos = get_word(0x9BB800 + i * 2) + 0x400
    area = get_word(0x9BB826 + i * 2)
    entrance_id = get_byte(0x9BB84C + i)
    r.setdefault(area, []).append({'x' : (pos >> 1) & 0x3f, 'y' : (pos >> 7) & 0x3f, 'entrance_id' : entrance_id})
  return r

def print_overworld_area(overworld_area):
  is_small = get_bytes(0x82F88D, 192)
  y = {}

  def get_music(ambient):
    def fn(x):
      if ambient:
        return tables.kAmbientSoundName[x >> 4]
      else:
        return tables.kMusicNames[x & 0xf]
    if overworld_area < 64:
      return {
        'beginning' : fn(get_byte(0x82C303 + overworld_area)),
        'zelda' : fn(get_byte(0x82C303 + overworld_area + 64)),
        'sword' : fn(get_byte(0x82C303 + overworld_area + 128)),
        'agahnim' : fn(get_byte(0x82C303 + overworld_area + 192)),
      }
    else:
      assert overworld_area < 64 + 96
      return {'agahnim' : fn(get_byte(0x82C403 + overworld_area - 64)) }

  def get_items():
    if overworld_area >= 128: return []
    ea = 0x9b0000 | get_word(0x9BC2F9 + overworld_area * 2)
    xs = []
    while get_word(ea) != 0xffff:
      pos = get_word(ea)
      assert pos%2==0
      x, y = pos//2%64, pos//2//64 
      xs.append([x, y, tables.kSecretNames[get_byte(ea+2)]])
      ea += 3
    return xs

  header = {
    'name' : tables.kAreaNames[overworld_area],
    'size' : 'small' if is_small[overworld_area] else 'big',
    'gfx' : get_byte(0x80FC9C + overworld_area) if overworld_area < 128 else -1,
    'palette' : get_byte(0x80FD1C + overworld_area) if overworld_area < 136 else -1,
    'sign_text' : get_word(0x87F51D + overworld_area * 2) if overworld_area < 128 else -1,
    'music' : get_music(False),
    'ambient' : get_music(True),
  }
      
  y['Header'] = header
  y['Travel'] = get_ow_travel_infos().get(overworld_area, [])
  y['Entrances'] = get_ow_entrance_info().get(overworld_area, [])
  hole_infos = get_hole_infos()
  if overworld_area in hole_infos:
    y['Holes'] = hole_infos[overworld_area]
  y['Exits'] = get_exit_datas().get(overworld_area, [])
  y['Items'] = get_items()
  
  def decode_sprites(base_addr):
    r = []
    ea = 0x890000 + get_word(base_addr + overworld_area * 2)
    while get_byte(ea) != 0xff:
      y, x, w = get_byte(ea), get_byte(ea+1), get_byte(ea+2)
      r.append([x, y, tables.kSpriteNames[w]])
      ea += 3
    return r

  def get_info(stage):
    if overworld_area >= 128: return {}
    if overworld_area >= 64: stage = 3
    return {
      'gfx' : get_byte(0x80FA41 + (overworld_area & 63) + stage * 64),
      'palette' : get_byte(0x80FB41 + (overworld_area & 63) + stage * 64),
    }
  if overworld_area < 64:
    y['Sprites.Beginning'] = {
      'info' : get_info(0),
      'sprites' : decode_sprites(0x89C881)
    }
    y['Sprites.FirstPart'] = {
      'info' : get_info(1),
      'sprites' : decode_sprites(0x89C901)
    }
    y['Sprites.SecondPart'] = {
      'info' : get_info(2),
      'sprites' : decode_sprites(0x89CA21)
    }
  elif overworld_area < 144:
    y['Sprites'] = {
      'info' : get_info(2),
      'sprites' : decode_sprites(0x89CA21)
    }
    
  s = yaml.dump(y, default_flow_style=None, sort_keys=False)
  open('overworld/overworld-%d.yaml' % overworld_area, 'w').write(s)

def print_all_overworld_areas():
  area_heads = get_bytes(0x82A5EC, 64)
  
  for i in range(160):
    if i >= 128 or area_heads[i&63] == (i&63):
      print_overworld_area(i)

def print_dialogue():
  text_compression.print_strings(util.ROM, file = open(text_compression.dialogue_filename(util.ROM.language), 'w', encoding='utf8'))

def decode_room_objects(p):
  objs = []
  j = 0
  while True:
    p0, p1, p2 = get_byte(p), get_byte(p+1), get_byte(p+2)
    A = p0 | p1 << 8
    if A == 0xffff:
      return p + 2, objs, None
    if A == 0xfff0:
      p += 2
      break
    if (A & 0xfc) != 0xfc:
      index = p2
      Dst = (p1 >> 2) << 7
      Dst |= (p0 & 0xfc) >> 1
      X = (Dst >> 1) & 0x3f
      Y = (Dst >> 7) & 0x3f
      W = p0 & 3
      H = p1 & 3
      if index < 0xf8:
        objs.append({'x' : X, 'y' : Y, 's' : '%d*%d' % (W, H), 'n': tables.kType0Names[index]})
      else:
        index2 = (index & 7) << 4 | H << 2 | W
        objs.append({'x' : X, 'y' : Y, 'n': tables.kType1Names[index2]})
    else:
      # subtype 2: 111111xx xxxxyyyy yyiiiiii
      X = ((p0 << 4 | p1 >> 4) & 0x3f)
      Y = (p1 << 2 | p2 >> 6) & 0x3f
      index = p2 & 0x3f
      objs.append({'x' : X, 'y' : Y, 'n' : tables.kType2Names[index]})
    p += 3
    j += 1

  doors = []
  while True:
    A = get_byte(p) | get_byte(p+1) << 8
    if A == 0xffff:
      return p + 2, objs, doors
    doors.append({'type':get_byte(p + 1), 'pos' : get_byte(p) >> 4, 'dir' : A & 3})
    p += 2

@cache
def get_chest_info():
  ea = 0x81e96e
  all = {}
  for i in range(504//3):
    room = get_word(ea + i * 3)
    data = get_byte(ea + i * 3 + 2)
    all.setdefault(room & 0x7fff, []).append((data, (room & 0x8000) != 0))
  return all

def _get_entrance_info_one(i, set):
  def get_exit_door(i):
    x = get_word((0x82D724, 0x82DC32)[set] + i * 2)
    if x == 0:
      return ['none']
    if x == 0xffff:
      return ['none_0xffff']
    return ['bombable' if x & 0x8000 else 'wooden', (x & 0x7e) >> 1, (x & 0x3f80) >> 7]
  kQuadrantNames = { 0 : 'upper_left', 2 : 'lower_left', 16 : 'upper_right', 18 : 'lower_right' }
  room = get_word((0x82C813, 0x82DB6E )[set] + i * 2)
  def get_se(se_base_addr, xy, quds):
    base_x = (room & 0xf) * 2
    base_y = (room >> 4) * 2  
    ym = (xy[1] & 0x100) >> 8
    xm = (xy[0] & 0x100) >> 8
    #if room == 259: ym = 1 # possibly related to none_0xffff
    hu = get_byte(se_base_addr + i * 8 + 0) - base_y - ym
    fu = get_byte(se_base_addr + i * 8 + 1) - base_y
    hd = get_byte(se_base_addr + i * 8 + 2) - base_y - ym
    fd = get_byte(se_base_addr + i * 8 + 3) - base_y - 1
    #assert fu == 0 and fd == 0 and hd == 0 and fd == 0, (room, fu, fd, hd, fd)
    qqq = xm if room >= 242 and quds[0] == 'single_x' else 0
    hl = get_byte(se_base_addr + i * 8 + 4) - base_x - xm
    fl = get_byte(se_base_addr + i * 8 + 5) - base_x - qqq
    hr = get_byte(se_base_addr + i * 8 + 6) - base_x - xm
    fr = get_byte(se_base_addr + i * 8 + 7) - base_x - 1 - qqq
    return [hu, fu, hd, fd, hl, fl, hr, fr]#

  player_x = get_word((0x82D063, 0x82DBDE)[set] + i * 2) - ((room & 0x00f) << 9)
  player_y = get_word((0x82CF59, 0x82DBD0)[set] + i * 2) - ((room & 0x1f0) << 5)

  y = {
    ('entrance_index' if set == 0 else 'starting_point_index'): i,
    'name' : tables.kEntranceNames[i] if set == 0 else 'Starting Location %d' % i,
    'scroll_xy' : [
      get_word((0x82CD45, 0x82DBB4)[set] + i * 2) - ((room & 0x00f) << 9),
      get_word((0x82CE4F, 0x82DBC2)[set] + i * 2) - ((room & 0x1f0) << 5),
    ],
    'player_xy' : [ player_x, player_y ],
    'camera_xy' : [
      get_word((0x82D277, 0x82DBFA)[set] + i * 2),
      get_word((0x82D16D, 0x82DBEC)[set] + i * 2),
    ],
    'blockset' : get_byte((0x82D381, 0x82DC08)[set] + i),
    'music' : tables.kMusicNames[get_byte((0x82D82E, 0x82DC4E)[set] + i)],
    'palace' : tables.kPalaceNames[(get_int8((0x82D48B, 0x82DC16)[set] + i) + 2) >> 1],
    'doorway_orientation' : get_int8(0x82D510 + i) if set == 0 else 0,
    'plane' : get_byte((0x82D595, 0x82DC1D)[set] + i) & 0xf,
    'ladder_level' : get_byte((0x82D595, 0x82DC1D)[set] + i) >> 4,
    'quadrants' : [
      'double_x' if (get_byte((0x82D61a, 0x82DC24)[set] + i) & 0x20) != 0 else 'single_x',
      'double_y' if (get_byte((0x82D61a, 0x82DC24)[set] + i) & 0x2) else 'single_y',
      kQuadrantNames[get_byte((0x82D69F, 0x82DC2B)[set] + i)]
      ],
    'floor' : get_int8((0x82D406, 0x82DC0F)[set] + i),
  }

  se = get_se((0x82C91D, 0x82DB7C)[set], y['player_xy'], y['quadrants'])
  if se != [0, 0, 0, 0, 0, 0, 0, 0]:
    y['repair_scroll_bounds'] = se
  y['house_exit_door'] = get_exit_door(i)

#  quadrant_byte = (2 if player_y & 0x100 else 0) + (16 if player_x & 0x100 else 0)
#  if get_byte((0x82D69F, 0x82DC2B)[set] + i) != quadrant_byte:
#    print('Room %d has incorrect quadrant byte' % room, y['quadrants'])
  if set:
    y['associated_entrance_index'] = get_word(0x82DC40 + i * 2)
  return room, y 

@cache
def get_entrance_info(set):
  r = {}
  for i in range(133 if set == 0 else 7):
    room, y = _get_entrance_info_one(i, set)
    r.setdefault(room, []).append(y)
  return r

@cache 
def pits_hurt_player():
  return set(get_word(0x80990C + i * 2) for i in range(57))

def print_room(room_index):
  p = 0x1f8000 + room_index * 3
  room_addr = get_byte(p) | get_byte(p+1) << 8 | get_byte(p+2) << 16
  p = 0x40000 | get_word(0x4f502 + room_index * 2)
  if p == 0x4FFEF:
    p = 0x82EDC5 # just some place with zeros
  floor, layout = get_byte(room_addr), get_byte(room_addr + 1)

  flags = get_byte(p + 0)
  p7, p8 = get_byte(p + 7), get_byte(p + 8)

  ea = 0x890000 + get_word(0x89D62E + room_index * 2)
  sort_sprites_setting = get_byte(ea)

  header = {
    'floor1': floor & 0xf,
    'floor2' : floor >> 4,
    'layout': layout >> 2,
    'start_quadrant' : layout & 3,
    'bg2' : tables.kBg2[flags >> 5],
    'collision' : tables.kCollisionNames[flags >> 2 & 7],
    'lights_out' : flags & 1,
    'palette' : get_byte(p + 1),
    'blockset' : get_byte(p + 2),
    'enemyblk' : get_byte(p + 3),
    'effect' : tables.kEffectNames[get_byte(p + 4)],
    'tag0' : tables.kTagNames[get_byte(p + 5)],
    'tag1' : tables.kTagNames[get_byte(p + 6)],
    'hole0_dest' : [get_byte(p+9), p7 & 3],
    'stair0_dest' : [get_byte(p+10), p7 >> 2 & 3],
    'stair1_dest' : [get_byte(p+11), p7 >> 4 & 3],
    'stair2_dest' : [get_byte(p+12), p7 >> 6 & 3],
    'stair3_dest' : [get_byte(p+13),p8 & 3],
    'tele_msg' : get_word(0x87F61D+room_index*2),
    'sort_sprites' : sort_sprites_setting,
    'pits_hurt_player' : room_index in pits_hurt_player()
  }

  def get_sprites():
    ea = 0x890000 + get_word(0x89D62E + room_index * 2)
    ea += 1
    r = []
    while get_byte(ea) != 0xff:
      y, x, type = get_byte(ea), get_byte(ea+1), get_byte(ea+2)
      if type == 0xe4:
        if y == 0xfe or y == 0xfd:
          r[-1].append('drop_key' if y == 0xfe else 'drop_big_key')
          ea += 3
          continue
      elif x >= 0xe0:
        floor = y >> 7
        r.append([x & 0x1f, y & 0x1f, 'lower' if floor else 'upper', tables.kSpriteNames[type + 0x100]])
        ea += 3
        continue
      subtype = (x >> 5) | ((y >> 5) & 3) << 3
      floor = y >> 7
      name = tables.kSpriteNames[type]
      if subtype != 0:
        i = name.index('-')
        name = name[:i] + ('.%d' % subtype) + name[i:]
      r.append([x & 0x1f, y & 0x1f, 'lower' if floor else 'upper', name])
      ea += 3
    return r

  def get_secrets():
    ea = 0x810000 | get_word(0x81db69 + room_index * 2)
    xs = []
    while get_word(ea) != 0xffff:
      pos = get_word(ea)
      assert pos%2==0
      x, y = pos//2%64, pos//2//64 
      xs.append([x, y, tables.kSecretNames[get_byte(ea+2)]])
      ea += 3
    return xs

  def get_chests():
    r = []
    for data, big in get_chest_info().get(room_index, []):
      if big:
        r.append('%d!' % data)
      else:
        r.append(data)
    return r
      
  sprites = get_sprites()
  secrets = get_secrets()

  data = {'Header' : header, 'Sprites' : sprites, 'Secrets' : secrets, 'Chests' : get_chests()}
  data['Entrances'] = get_entrance_info(0).get(room_index, [])
  if room_index in get_entrance_info(1):
    data['StartingPoints'] = get_entrance_info(1)[room_index]

  p = room_addr + 2
  p, objs, doors = decode_room_objects(p)
  data['Layer1'] = objs
  if doors: data['Layer1.doors'] = doors

  p, objs, doors = decode_room_objects(p)
  data['Layer2'] = objs
  if doors: data['Layer2.doors'] = doors

  p, objs, doors = decode_room_objects(p)
  data['Layer3'] = objs
  if doors: data['Layer3.doors'] = doors
  
  return yaml.dump(data, default_flow_style=None, sort_keys=False)

def print_all_dungeon_rooms():
  for i in range(320):
    s = print_room(i)
    open( 'dungeon/dungeon-%d.yaml' % i, 'w').write(s)

def print_default_rooms():
  def print_default_room(idx):
    p = 0x84EF2F + idx * 3
    room_addr = get_byte(p) | get_byte(p+1) << 8 | get_byte(p+2) << 16
    p, objs, doors = decode_room_objects(room_addr)
    assert doors == None
    return objs
  default_rooms = {}
  for i in range(8):
    default_rooms['Default%d' % i] = print_default_room(i)
  s = yaml.dump(default_rooms, default_flow_style=None, sort_keys=False)
  open('dungeon/default_rooms.yaml', 'w').write(s)

def print_overlay_rooms():
  def print_overlay_room(idx):
    p = 0x84ECC0 + idx * 3
    room_addr = get_byte(p) | get_byte(p+1) << 8 | get_byte(p+2) << 16
    p, objs, doors = decode_room_objects(room_addr)
    assert doors == None
    return objs
  default_rooms = {}
  for i in range(19):
    default_rooms['Overlay%d' % i] = print_overlay_room(i)
  s = yaml.dump(default_rooms, default_flow_style=None, sort_keys=False)
  open('dungeon/overlay_rooms.yaml', 'w').write(s)

def make_directories():
  os.makedirs('img', exist_ok=True)
  os.makedirs('overworld', exist_ok=True)
  os.makedirs('dungeon', exist_ok=True)
  os.makedirs('sound', exist_ok=True)

def print_all_text_stuff():
  print_all_overworld_areas()
  print_all_dungeon_rooms()
  print_overlay_rooms()
  print_default_rooms()
  print_dialogue()
  print_map32_to_map16(open('map32_to_map16.txt', 'w'))

def main():
  make_directories()
  print_all_text_stuff()
  extract_music.extract_sound_data(util.ROM)
  sprite_sheets.decode_link_sprites()
  sprite_sheets.decode_sprite_sheets()
  sprite_sheets.decode_hud_icons()
  sprite_sheets.decode_font()
 
if __name__ == "__main__":
  util.load_rom(sys.argv[1] if len(sys.argv) >= 2 else None)
  main()

