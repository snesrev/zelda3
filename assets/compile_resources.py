import sys
import text_compression
import util
from PIL import Image
import yaml
import tables
import compile_music
import array, hashlib, struct
from util import cache
import sprite_sheets
import argparse
import os

def flatten(xss):
    return [x for xs in xss for x in xs]
  
def invert_dict(xs):
  return {s:i for i,s in xs.items()}

assets = {}

def add_asset_uint8(name, data):
  assert name not in assets
  assets[name] = ('uint8', bytes(array.array('B', data)))

def add_asset_uint16(name, data):
  assert name not in assets
  assets[name] = ('uint16', bytes(array.array('H', data)))

def add_asset_int8(name, data):
  assert name not in assets
  assets[name] = ('int8', bytes(array.array('b', data)))

def add_asset_int16(name, data):
  assert name not in assets
  assets[name] = ('int16', bytes(array.array('h', data)))

def add_asset_packed(name, data):
  assert name not in assets
  assets[name] = ('packed', pack_arrays(data))

def print_map32_to_map16():
  tab = {}
  for line in open('map32_to_map16.txt'):
    line = line.strip()
    x, xs = line.split(':', 1)
    tab[int(x)] = [int(t) for t in xs.split(',')]

  def pack(*a):
    res=[0] * 6
    res[0] = a[0] & 0xff
    res[1] = a[1] & 0xff
    res[2] = a[2] & 0xff
    res[3] = a[3] & 0xff
    res[4] = (a[0] >> 8) << 4 | (a[1] >> 8)
    res[5] = (a[2] >> 8) << 4 | (a[3] >> 8)
    return res    

  res = [[],[],[],[]]
  for i in range(0, len(tab), 4):
    for j in range(4):
      res[j].extend(pack(tab[i][j], tab[i+1][j], tab[i+2][j], tab[i+3][j]))

  add_asset_uint8('kMap32ToMap16_0', res[0])
  add_asset_uint8('kMap32ToMap16_1', res[1])
  add_asset_uint8('kMap32ToMap16_2', res[2])
  add_asset_uint8('kMap32ToMap16_3', res[3])

def compress_dialogue(fname, lang):
  lines = []
  for line in open(fname, encoding='utf8').read().splitlines():
    a, b = line.split(': ', 1)
    lines.append(b)
  return text_compression.compress_strings(lines, lang)

def compress_store(r):
  rr = []
  j, jend = 0, len(r)
  while j < jend:
    n = min(jend - j, 1024)
    rr.append(0xe0 | (n - 1) >> 8)
    rr.append((n - 1) & 0xff)
    rr.extend(r[j:j+n])
    j += n
  rr.append(0xff)
  return rr
  
# Pack arrays and determine automatically the index size
def pack_arrays(arr):
  if len(arr) == 0:
    return b''
  all_offs, offs = [], 0
  for i in range(len(arr) - 1):
    offs += len(arr[i])
    all_offs.append(offs)
  if offs < 65536 and len(arr) <= 8192:
    return b''.join([struct.pack('H', i) for i in all_offs] + arr + [struct.pack('H', len(arr) - 1)])
  else:
    return b''.join([struct.pack('I', i) for i in all_offs] + arr + [struct.pack('H', 8192 + len(arr) - 1)])

def print_images(args):
  sprsheet = sprite_sheets.load_sprite_sheets() if args.sprites_from_png else None

  all = []
  for i in range(108):
    if sprsheet != None and i < 103:
      all.append(sprsheet.encode_sheet_in_snes_format(i))
    elif i < 12:
      all.append(bytes(ROM.get_bytes(tables.kCompSpritePtrs[i], 0x600)))   
    else:
      decomp, comp_len = util.decomp(tables.kCompSpritePtrs[i], ROM.get_byte, False, True)
      all.append(bytes(ROM.get_bytes(tables.kCompSpritePtrs[i], comp_len)))
  add_asset_packed('kSprGfx', all)

  all = []
  for i in range(len(tables.kCompBgPtrs)):
    decomp, comp_len = util.decomp(tables.kCompBgPtrs[i], ROM.get_byte, False, True)
    all.append(bytes(ROM.get_bytes(tables.kCompBgPtrs[i], comp_len)))
  add_asset_packed('kBgGfx', all)

def print_dialogue(args):
  from text_compression import dialogue_filename, kLanguages

  languages = ['us']
  if args.languages:
    for a in args.languages.split(','):
      if a in languages or a not in kLanguages:
        raise Exception(f'Language {a} is not valid')
      name = dialogue_filename(a)
      if not os.path.exists(name):
        raise Exception(f'{name} not found. You need to extract it with --extract-dialogue using the ROM of that language.')
      languages.append(a)

  all_langs, all_fonts, mappings = [], [], []
  for i, lang in enumerate(languages):
    dict_packed = pack_arrays(text_compression.encode_dictionary(lang))
    dialogue_packed = pack_arrays(compress_dialogue(dialogue_filename(lang), lang))
    all_langs.append(pack_arrays([dict_packed, dialogue_packed]))
    font_data, font_width = sprite_sheets.encode_font_from_png(lang)
    all_fonts.append(pack_arrays([font_data, font_width]))
    flags = text_compression.uses_new_format(lang)
    if i != 0: flags |= 2 # no us rom match?
    mappings.append(pack_arrays([lang.encode('utf8'), bytearray([i, i, flags])]))
  add_asset_packed('kDialogue', all_langs)
  add_asset_packed('kDialogueFont', all_fonts)
  add_asset_packed('kDialogueMap', mappings)

def print_misc(args):
  add_asset_uint8('kOverworldMapGfx', ROM.get_bytes(0x18c000, 0x4000))
  add_asset_uint8('kLightOverworldTilemap', ROM.get_bytes(0xac727, 4096))
  add_asset_uint8('kDarkOverworldTilemap', ROM.get_bytes(0xaD727, 1024))

  add_asset_uint16('kPredefinedTileData', ROM.get_words(0x9B52, 6438))

  add_asset_uint16('kMap16ToMap8', ROM.get_words(0x8f8000, 3752 * 4))

  add_asset_uint8('kGeneratedWishPondItem', ROM.get_bytes(0x888450, 256))
  add_asset_uint8('kGeneratedBombosArr', ROM.get_bytes(0x8890FC, 256))

  add_asset_uint8('kGeneratedEndSequence15', ROM.get_bytes(0x8ead25, 256))
  add_asset_uint8('kEnding_Credits_Text', ROM.get_bytes(0x8EB178, 1989))
  add_asset_uint16('kEnding_Credits_Offs', ROM.get_words(0x8EB93d, 394))
  add_asset_uint16('kEnding_MapData', ROM.get_words(0x8EB038, 160))
  add_asset_uint16('kEnding0_Offs', ROM.get_words(0x8EC2E1, 17))
  add_asset_uint8('kEnding0_Data', ROM.get_bytes(0x8EBF4C, 917))

  add_asset_uint16('kPalette_DungBgMain', ROM.get_words(0x9BD734, 1800))
  add_asset_uint16('kPalette_MainSpr', ROM.get_words(0x9BD218, 120))

  add_asset_uint16('kPalette_ArmorAndGloves', sprite_sheets.override_armor_palette or ROM.get_words(0x9BD308, 75))
  add_asset_uint16('kPalette_Sword', ROM.get_words(0x9BD630, 12))
  add_asset_uint16('kPalette_Shield', ROM.get_words(0x9BD648, 12))

  add_asset_uint16('kPalette_SpriteAux3', ROM.get_words(0x9BD39E, 84))
  add_asset_uint16('kPalette_MiscSprite_Indoors', ROM.get_words(0x9BD446, 77))
  add_asset_uint16('kPalette_SpriteAux1', ROM.get_words(0x9BD4E0, 168))

  add_asset_uint16('kPalette_OverworldBgMain', ROM.get_words(0x9BE6C8, 210))
  add_asset_uint16('kPalette_OverworldBgAux12', ROM.get_words(0x9BE86C, 420))
  add_asset_uint16('kPalette_OverworldBgAux3', ROM.get_words(0x9BE604, 98))
  add_asset_uint16('kPalette_PalaceMapBg', ROM.get_words(0x9BE544, 96))
  add_asset_uint16('kPalette_PalaceMapSpr', ROM.get_words(0x9BD70A, 21))
  add_asset_uint16('kHudPalData', ROM.get_words(0x9BD660, 64))

  add_asset_uint16('kOverworldMapPaletteData', ROM.get_words(0x8ADB27, 256))

@cache
def load_overworld_yaml(room):
  return yaml.safe_load(open('overworld/overworld-%d.yaml' % room, 'r'))
    
def print_overworld():
  r = []
  for i in range(160):
    addr = ROM.get_24(0x82F94D + i * 3)
    decomp, comp_len = util.decomp(addr, ROM.get_byte, True, True)
    r.append(bytes(ROM.get_bytes(addr, comp_len)))
  add_asset_packed('kOverworld_Hibytes_Comp', r)

  r = []
  for i in range(160):
    addr = ROM.get_24(0x82FB2D + i * 3)
    decomp, comp_len = util.decomp(addr, ROM.get_byte, True, True)
    r.append(bytes(ROM.get_bytes(addr, comp_len)))
  add_asset_packed('kOverworld_Lobytes_Comp', r)

def is_area_head(i):
  return i >= 128 or ROM.get_byte(0x82A5EC + (i & 63)) == (i & 63)

class OutArrays:
  def __init__(self):
    self.arrs = []
  def add(self, type, name, size, initializer = None, items_per_line = 16):
    t = [initializer] * size
    self.arrs.append((type, name, t, items_per_line))
    setattr(self, name, t)
  def write(self):
    for type, name, arr, items_per_line in self.arrs:
      for i, j in enumerate(arr):
        assert isinstance(j, int), (name, i, j, arr)
      if type == 'uint8':
        add_asset_uint8(name, arr)
      elif type == 'uint16':
        add_asset_uint16(name, arr)
      elif type == 'int8':
        add_asset_int8(name, arr)
      elif type == 'int16':
        add_asset_int16(name, arr)
      else:
        assert 0, type
    
def print_overworld_tables():
  A = OutArrays()
  
  A.add('uint8', 'kOverworldMapIsSmall', 192, initializer = 0, items_per_line = 8)
  A.add('uint8', 'kOverworldAuxTileThemeIndexes', 128, items_per_line = 8)
  A.add('uint8', 'kOverworldBgPalettes', 136, items_per_line = 8)
  A.add('uint16', 'kOverworld_SignText', 128, items_per_line = 8)
  A.add('uint8', 'kOwMusicSets', 256, items_per_line = 8)
  A.add('uint8', 'kOwMusicSets2', 96, items_per_line = 8)
  
  def get_music_byte(h, tag):
    return tables.kMusicNamesRev[h['music'][tag]] | tables.kAmbientSoundNameRev[h['ambient'][tag]] << 4

  def get_loadoffs(c, d):
    x, y = c[0] >> 4, c[1] >> 4
    x += d[0]
    y += d[1]
    return (y&0x3f) << 7 | (x&0x3f) << 1

  def awrite(arr, area, key, value):
    arr[key] = value
    if area < 128 and not A.kOverworldMapIsSmall[area]:
      arr[key + 1], arr[key + 8], arr[key + 9] = value, value, value
      
  loaded_areas = [(i, load_overworld_yaml(i)) for i in range(160) if is_area_head(i)]

  for i,y in loaded_areas:
    h = y['Header']
    A.kOverworldMapIsSmall[i] = {'small':1, 'big':0}[h['size']]
    if i < len(A.kOverworldAuxTileThemeIndexes): awrite(A.kOverworldAuxTileThemeIndexes, i, i, h['gfx'])
    if i < len(A.kOverworldBgPalettes): awrite(A.kOverworldBgPalettes, i, i, h['palette'])
    if i < len(A.kOverworld_SignText): awrite(A.kOverworld_SignText, i, i, h['sign_text'])
    if i < 64:
      awrite(A.kOwMusicSets, i, i, get_music_byte(h, 'beginning'))
      awrite(A.kOwMusicSets, i, i + 64, get_music_byte(h, 'zelda'))
      awrite(A.kOwMusicSets, i, i + 128, get_music_byte(h, 'sword'))
      awrite(A.kOwMusicSets, i, i + 192, get_music_byte(h, 'agahnim'))
    elif i < 64 + 96:
      awrite(A.kOwMusicSets2, i, i - 64, get_music_byte(h, 'agahnim'))

  for a in ['kBirdTravel_ScreenIndex', 'kBirdTravel_Map16LoadSrcOff', 'kBirdTravel_ScrollX',
            'kBirdTravel_ScrollY', 'kBirdTravel_LinkXCoord', 'kBirdTravel_LinkYCoord',
            'kBirdTravel_CameraXScroll', 'kBirdTravel_CameraYScroll']:
    A.add('uint16', a, 17)
  A.add('int8', 'kBirdTravel_Unk1', 17)
  A.add('int8', 'kBirdTravel_Unk3', 17)
  A.add('uint16', 'kWhirlpoolAreas', 8)

  next_whirlpool_id = 0
  for i, y in loaded_areas:
    for t in y['Travel']:
      if 'bird_travel_id' in t:
        j = t['bird_travel_id']
      else:
        A.kWhirlpoolAreas[next_whirlpool_id] = t['whirlpool_src_area']
        j = next_whirlpool_id + 9
        next_whirlpool_id += 1
      base_x, base_y = (i & 7) << 9, (i & 56) << 6
      A.kBirdTravel_ScreenIndex[j] = i
      A.kBirdTravel_Map16LoadSrcOff[j] = get_loadoffs(t['scroll_xy'], t['load_xy'])
      A.kBirdTravel_ScrollX[j] = t['scroll_xy'][0] + base_x
      A.kBirdTravel_ScrollY[j] = t['scroll_xy'][1] + base_y
      A.kBirdTravel_LinkXCoord[j] = t['xy'][0] + base_x 
      A.kBirdTravel_LinkYCoord[j] = t['xy'][1] + base_y
      A.kBirdTravel_CameraXScroll[j] = t['camera_xy'][0] + base_x 
      A.kBirdTravel_CameraYScroll[j] = t['camera_xy'][1] + base_y
      A.kBirdTravel_Unk1[j] = t['unk'][0]
      A.kBirdTravel_Unk3[j] = t['unk'][1]

  A.add('uint16', 'kOverworld_Entrance_Area', 129)
  A.add('uint16', 'kOverworld_Entrance_Pos', 129)
  A.add('uint8', 'kOverworld_Entrance_Id', 129)

  for i, y in loaded_areas:
    for e in y['Entrances']:
      j = e['index']
      assert A.kOverworld_Entrance_Id[j] == None
      A.kOverworld_Entrance_Area[j] = i
      A.kOverworld_Entrance_Id[j] = e['entrance_id']
      A.kOverworld_Entrance_Pos[j] = e['x'] << 1 | e['y'] << 7

  A.add('uint16', 'kFallHole_Area', 19)
  A.add('uint16', 'kFallHole_Pos', 19)
  A.add('uint8', 'kFallHole_Entrances', 19)
      
  holes = []
  for i, y in loaded_areas:
    if 'Holes' not in y: continue
    for e in y['Holes']:
      x, y, j = e['x'], e['y'], e['entrance_id']
      holes.append((j, x << 1 | ((y - 8) & 0x3f) << 7, i))
  for i, (entrance, pos, area) in enumerate(sorted(holes)):
    A.kFallHole_Area[i] = area
    A.kFallHole_Pos[i] = pos
    A.kFallHole_Entrances[i] = entrance

  A.add('uint8', 'kExitData_ScreenIndex', 79)
  for a in ['kExitDataRooms', 'kExitData_Map16LoadSrcOff', 'kExitData_ScrollX',
            'kExitData_ScrollY', 'kExitData_XCoord', 'kExitData_YCoord',
            'kExitData_CameraXScroll', 'kExitData_CameraYScroll']:
    A.add('uint16', a, 79)
  A.add('uint16', 'kExitData_NormalDoor', 79, initializer = 0)
  A.add('uint16', 'kExitData_FancyDoor', 79, initializer = 0)
  A.add('int8', 'kExitData_Unk1', 79)
  A.add('int8', 'kExitData_Unk3', 79)

  A.add('uint16', 'kSpExit_Top', 16, initializer = 0)
  A.add('uint16', 'kSpExit_Bottom', 16, initializer = 0)
  A.add('uint16', 'kSpExit_Left', 16, initializer = 0)
  A.add('uint16', 'kSpExit_Right', 16, initializer = 0)
  A.add('int16', 'kSpExit_Tab4', 16, initializer = 0)
  A.add('int16', 'kSpExit_Tab5', 16, initializer = 0)
  A.add('int16', 'kSpExit_Tab6', 16, initializer = 0)
  A.add('int16', 'kSpExit_Tab7', 16, initializer = 0)
  A.add('uint16', 'kSpExit_LeftEdgeOfMap', 16, initializer = 0)
  A.add('uint8', 'kSpExit_Dir', 16, initializer = 0)
  A.add('uint8', 'kSpExit_SprGfx', 16, initializer = 0)
  A.add('uint8', 'kSpExit_AuxGfx', 16, initializer = 0)
  A.add('uint8', 'kSpExit_PalBg', 16, initializer = 0)
  A.add('uint8', 'kSpExit_PalSpr', 16, initializer = 0)
  
  for i, y in loaded_areas:
    for e in y['Exits']:
      j = e['index']
      base_x, base_y = (i & 7) << 9, (i & 56) << 6
      assert A.kExitData_ScreenIndex[j] == None
      A.kExitData_ScreenIndex[j] = i
      room = e['room']
      A.kExitDataRooms[j] = e['room']
      A.kExitData_Map16LoadSrcOff[j] = get_loadoffs(e['scroll_xy'], e['load_xy'])
      A.kExitData_ScrollX[j] = e['scroll_xy'][0] + base_x
      A.kExitData_ScrollY[j] = e['scroll_xy'][1] + base_y
      A.kExitData_XCoord[j] = e['xy'][0] + base_x
      A.kExitData_YCoord[j] = e['xy'][1] + base_y
      A.kExitData_CameraXScroll[j] = e['camera_xy'][0] + base_x 
      A.kExitData_CameraYScroll[j] = e['camera_xy'][1] + base_y
      A.kExitData_Unk1[j] = e['unk'][0]
      A.kExitData_Unk3[j] = e['unk'][1]
      door = e.get('door')
      if door != None:
        if door[0] in ('bombable', 'wooden'):
          A.kExitData_NormalDoor[j] = door[1] << 1 | door[2] << 7 | (0x8000 if door[0] == 'bombable' else 0)
        elif door[0] in ('palace', 'sanctuary'):
          A.kExitData_FancyDoor[j] = door[1] << 1 | door[2] << 7 | (0x8000 if door[0] == 'palace' else 0)
        else:
          assert e[0] == 'none'
      se = e.get('special_exit')
      if se:
        j = room - 0x180
        A.kSpExit_Dir[j] = se['dir'] * 2
        A.kSpExit_SprGfx[j] = se['spr_gfx']
        A.kSpExit_AuxGfx[j] = se['aux_gfx']
        A.kSpExit_PalBg[j] = se['pal_bg']
        A.kSpExit_PalSpr[j] = se['pal_spr']
        A.kSpExit_Top[j] = se['top']
        A.kSpExit_Bottom[j] = se['bottom']
        A.kSpExit_Left[j] = se['left']
        A.kSpExit_Right[j] = se['right']
        A.kSpExit_LeftEdgeOfMap[j] = se['left_edge_of_map']
        A.kSpExit_Tab4[j] = se['unk4']
        A.kSpExit_Tab5[j] = se['unk5']
        A.kSpExit_Tab6[j] = se['unk6']
        A.kSpExit_Tab7[j] = se['unk7']


  A.add('uint16', 'kOverworldSecrets_Offs', 128, initializer = None)
  A.add('uint8', 'kOverworldSecrets', 0)
  for i, y in loaded_areas:
    if len(y['Items']):
      assert i < 128
      j = len(A.kOverworldSecrets)
      awrite(A.kOverworldSecrets_Offs, i, i, j)
      for e in y['Items']:
        pos = e[0] << 1 | e[1] << 7
        item = tables.kSecretNamesRev[e[2]]
        A.kOverworldSecrets.extend([pos & 0xff, pos >> 8, item])
      A.kOverworldSecrets.extend([0xff, 0xff])
  for i in range(128):
    if A.kOverworldSecrets_Offs[i] == None:
      A.kOverworldSecrets_Offs[i] = len(A.kOverworldSecrets) - 2

  A.add('uint16', 'kOverworldSpriteOffs', 144 * 3, initializer = 0)
  A.add('uint8', 'kOverworldSprites', 0)
  A.add('uint8', 'kOverworldSpriteGfx', 256)
  A.add('uint8', 'kOverworldSpritePalettes', 256)
  A.kOverworldSprites.append(0xff)
  def do_sprite_range(start, end, stagename, sprite_stage_idxs, infostage):
    for i, y in loaded_areas:
      if i < start or i >= end: continue
      info = y[stagename]['info']
      if i < 128:
        awrite(A.kOverworldSpriteGfx, i, (i & 63) + infostage * 64, info['gfx'])
        awrite(A.kOverworldSpritePalettes, i, (i & 63) + infostage * 64, info['palette'])
      if len(y[stagename]['sprites']):
        for stage in sprite_stage_idxs:
          A.kOverworldSpriteOffs[stage * 144 + i] = len(A.kOverworldSprites)
        for e in y[stagename]['sprites']:
          A.kOverworldSprites.extend([e[1], e[0], tables.kSpriteNamesRev[e[2]]])
        A.kOverworldSprites.append(0xff)
  do_sprite_range(0, 64, 'Sprites.Beginning', [0], 0)
  do_sprite_range(0, 64, 'Sprites.FirstPart', [1], 1)
  do_sprite_range(0, 64, 'Sprites.SecondPart', [2], 2)
  do_sprite_range(64, 144, 'Sprites', [1, 2], 3)

  A.write()
  add_asset_uint8('kMap8DataToTileAttr', ROM.get_bytes(0x8E9459, 512))
  add_asset_uint8('kSomeTileAttr', ROM.get_bytes(0x9bf110, 3824))


def print_dungeon_map():
  r, r2 = [], []
  for i in range(14):
    kSizes = [75, 125, 50, 75, 175, 75, 50, 75, 50, 200, 150, 75, 100, 200]
    addr = 0xa0000 + ROM.get_word(0x8AF605 + i * 2)
    b = ROM.get_bytes(addr, kSizes[i])
    nonzero_bytes = len(b) - b.count(0xf)
    r.append(bytes(b))
    addr = 0xa0000 + ROM.get_word(0x8AFBE4 + i * 2)
    b = ROM.get_bytes(addr, nonzero_bytes)
    r2.append(bytes(b))
    
  add_asset_packed('kDungMap_FloorLayout', r)
  add_asset_packed('kDungMap_Tiles', r2)


@cache
def load_dungeon_yaml(room):
  return yaml.safe_load(open('dungeon/dungeon-%d.yaml' % room, 'r'))
  
def print_dungeon_sprites():
  offsets=[0 for i in range(320)]
  data = [0, 0xff]
  for i in range(320):
    y = load_dungeon_yaml(i)
    sortmode = y['Header']['sort_sprites']
    if len(y['Sprites']) == 0 and sortmode == 0:
      continue
    offsets[i] = len(data)
    data.append(sortmode)
    for s in y['Sprites']:
      xx, yy, f, name = s[:4]
      f = {'upper' : 0, 'lower' : 1}[f]
      assert xx >= 0 and xx <= 0x1f
      assert yy >= 0 and yy <= 0x1f

      # parse out subcode
      ss = 0
      if len(name) > 2 and name[2] == '.':
        j = name.index('-')
        ss = int(name[3:j])
        name = name[0:2] + name[j:]

      name = tables.kSpriteName2Idx[name]
      if name >= 0x100:
        data.extend((f << 7 | 0 << 5 | yy, xx | 7 << 5, name & 0xff))
      else:
        data.extend((f << 7 | (ss >> 3) << 5 | yy, xx | (ss & 7) << 5, name))
      if len(s) == 5:
        kDropTypesToCode = {'drop_key' : [0xfe, 0, 0xe4], 'drop_big_key' : [0xfd, 0, 0xe4]}
        data.extend(kDropTypesToCode[s[-1]])
        
    data.append(0xff)
  add_asset_uint8('kDungeonSprites', data)
  add_asset_uint16('kDungeonSpriteOffs', offsets)

def print_dungeon_secrets():
  result = [None] * 640
  for i in range(320):
    y = load_dungeon_yaml(i)
    def fixone(s):
      x, y, data = s[0], s[1], tables.kSecretNamesRev[s[2]]
      pos = (x + y * 64) * 2
      return [pos & 0xff, pos >> 8, data]
    if len(y['Secrets']):
      result[i*2+0] = len(result) & 0xff
      result[i*2+1] = len(result) >> 8
      for a in y['Secrets']:
        result.extend(fixone(a))
        
      result.extend([0xff, 0xff])
  for i in range(320):
    if result[i*2+0] == None:
      l = (len(result) - 2)
      result[i*2+0] = l & 0xff
      result[i*2+1] = l >> 8
  add_asset_uint8('kDungeonSecrets', result)

def append_scan_bytes(big, little):
  for n in range(len(little), -1, -1):
    if n == 0 or big[-n:] == little[:n]:
      offset = len(big) - n
      big.extend(little[n:])
      return offset
  
def print_dungeon_rooms():
  def print_layer(objs, doors):
    door_offset = None
    for o in objs:
      if o['n'] in tables.kType0Names_rev:
        index = tables.kType0Names_rev[o['n']] # 0 - 0xf7
        w = int(o['s'][0])
        h = int(o['s'][2])
        assert w >= 0 and w <= 3 and h >= 0 and h <= 3
        p0 = o['x'] * 4 + w
        p1 = o['y'] * 4 + h
        p2 = index
      elif o['n'] in tables.kType1Names_rev:
        index = tables.kType1Names_rev[o['n']]
        p0 = o['x'] * 4 + (index >> 0 & 3)
        p1 = o['y'] * 4 + (index >> 2 & 3)
        p2 = (index >> 4) + 0xf8
      elif o['n'] in tables.kType2Names_rev:
        index = tables.kType2Names_rev[o['n']]
        x, y = o['x'], o['y']
        #111111xx xxxxyyyy yyiiiiii
        p0 = 0xfc + (x >> 4 & 3)
        p1 = (x << 4 & 0xf0) | (y >> 2 & 0x0f)
        p2 = index | (y << 6 & 0xc0)
      else:
        raise Exception('item %s not found' % o['n'])
      data.extend((p0, p1, p2))
    if doors != None:
      data.extend([0xf0, 0xff])
      door_offset = len(data)
      for d in doors:
        data.extend((d['dir'] | d['pos'] << 4, d['type']))
    data.extend([0xff, 0xff])
    return door_offset

  def get_room_header(y):
    h = y['Header']
    p7 = h['hole0_dest'][1] | h['stair0_dest'][1] << 2 | h['stair1_dest'][1] << 4 | h['stair2_dest'][1] << 6
    p8 = h['stair3_dest'][1]
    return [tables.kBg2_rev[h['bg2']] << 5 | tables.kCollisionNames_rev[h['collision']] << 2 | h['lights_out'],
            h['palette'],
            h['blockset'],
            h['enemyblk'],
            tables.kEffectNames_rev[h['effect']],
            tables.kTagNames_rev[h['tag0']],
            tables.kTagNames_rev[h['tag1']],
            p7,
            p8,
            h['hole0_dest'][0],
            h['stair0_dest'][0],
            h['stair1_dest'][0],
            h['stair2_dest'][0],
            h['stair3_dest'][0]]

  data = []
  offsets = [0]*320
  door_offsets = [0]*320
  room_headers = []
  header_offsets = [0] * 320
  chests = []
  entrances = [None] * 133
  starting_points = [None] * 7
  sign_texts = [0] * 320
  pits_hurt_player = []
  
  for i in range(320):
    y = load_dungeon_yaml(i)
    h = y['Header']
    if h['pits_hurt_player']: pits_hurt_player.append(i)
    offsets[i]=len(data)
    data.append(h['floor1'] + h['floor2'] * 16)
    data.append(h['layout'] * 4 + h['start_quadrant'])
    print_layer(y['Layer1'], y.get('Layer1.doors'))
    print_layer(y['Layer2'], y.get('Layer2.doors'))
    door_offsets[i] = print_layer(y['Layer3'], y.get('Layer3.doors') or [])
    header_offsets[i] = append_scan_bytes(room_headers, get_room_header(y))
    sign_texts[i] = h['tele_msg']
    for a in y['Chests']:
      if isinstance(a, int):
        chests.extend([i & 0xff, i >> 8, a])
      else:
        assert a.endswith('!')
        chests.extend([i & 0xff, (i >> 8) | 0x80, int(a[:-1])])
    for e in y['Entrances']:
      a = e['entrance_index']
      e['room'] = i
      assert entrances[a] == None
      entrances[a] = e
    if 'StartingPoints' in y:
      for e in y['StartingPoints']:
        a = e['starting_point_index']
        e['room'] = i
        assert starting_points[a] == None
        starting_points[a] = e
  for i in range(133):
    if entrances[i] == None:
      raise Exception('Entrance %d not defined' % i)
  for i in range(7):
    if starting_points[i] == None:
      raise Exception('Starting point %d not defined' % i)

  def print_entrance_info(entrances, prefix):
    def get_rc(a):
      rep, room, quads, xy = a.get('repair_scroll_bounds'), a['room'], a['quadrants'], a['player_xy']
      if rep == None: rep = (0, 0, 0, 0, 0, 0, 0, 0)
      base_x = (room & 0xf) * 2
      base_y = (room >> 4) * 2
      ym = (xy[1] & 0x100) >> 8
      xm = (xy[0] & 0x100) >> 8
      qqq = xm if room >= 242 and quads[0] == 'single_x' else 0
      l = [base_y + ym, base_y, base_y + ym, base_y + 1,
           base_x + xm, base_x + qqq, base_x + xm, base_x + qqq + 1]
      return [a+b for a,b in zip(l, rep)]
    def get_palace(a):
      i = tables.kPalaceNames.index(a)
      return -1 if i == 0 else (i - 1) * 2
    def get_quadrant1(a):
      return ['single_x', 'double_x'].index(a[0]) * 0x20 + ['single_y', 'double_y'].index(a[1]) * 0x2
    def get_quadrant2(a):
      kScrollNames = { 'upper_left' : 0, 'lower_left' : 2, 'upper_right' : 16, 'lower_right' : 18 }
      return kScrollNames[a[2]]
    def get_exit_door(a):
      if a[0] == 'none': return 0
      if a[0] == 'none_0xffff': return 65535
      return ['wooden', 'bombable'].index(a[0]) << 15 | a[1] << 1 | a[2] << 7
            
    add_asset_uint16(prefix+'rooms', [a['room'] for a in entrances])
    add_asset_uint8(prefix+'relativeCoords', flatten([get_rc(a) for a in entrances]))
    def get_base_x(a): return ((a['room'] & 0x00f) << 9)
    def get_base_y(a): return ((a['room'] & 0x1f0) << 5)
    add_asset_uint16(prefix+'scrollX', [a['scroll_xy'][0] + get_base_x(a) for a in entrances])
    add_asset_uint16(prefix+'scrollY', [a['scroll_xy'][1] + get_base_y(a) for a in entrances])
    add_asset_uint16(prefix+'playerX', [a['player_xy'][0] + get_base_x(a) for a in entrances])
    add_asset_uint16(prefix+'playerY', [a['player_xy'][1] + get_base_y(a) for a in entrances])
    add_asset_uint16(prefix+'cameraX', [a['camera_xy'][0] for a in entrances])
    add_asset_uint16(prefix+'cameraY', [a['camera_xy'][1] for a in entrances])
    add_asset_uint8(prefix+'blockset', [a['blockset'] for a in entrances])
    add_asset_int8(prefix+'floor', [a['floor'] for a in entrances])
    add_asset_int8(prefix+'palace', [get_palace(a['palace']) for a in entrances])
    add_asset_uint8(prefix+'doorwayOrientation', [a['doorway_orientation'] for a in entrances])
    add_asset_uint8(prefix+'startingBg', [a['plane'] + a['ladder_level'] * 16 for a in entrances])
    add_asset_uint8(prefix+'quadrant1', [get_quadrant1(a['quadrants']) for a in entrances])
    add_asset_uint8(prefix+'quadrant2', [get_quadrant2(a['quadrants']) for a in entrances])
    add_asset_uint16(prefix+'doorSettings', [get_exit_door(a['house_exit_door']) for a in entrances])
    if prefix == 'kStartingPoint_':
      add_asset_uint8(prefix+'entrance', [a['associated_entrance_index'] for a in entrances])
    m = invert_dict(tables.kMusicNames)
    add_asset_uint8(prefix+'musicTrack', [m[a['music']] for a in entrances])
    

  add_asset_uint8('kDungeonRoom', data)
  add_asset_uint16('kDungeonRoomOffs', offsets)
  add_asset_uint16('kDungeonRoomDoorOffs', door_offsets)
  add_asset_uint8('kDungeonRoomHeaders', room_headers)
  add_asset_uint16('kDungeonRoomHeadersOffs', header_offsets)
  add_asset_uint8('kDungeonRoomChests', chests)
  add_asset_uint16('kDungeonRoomTeleMsg', sign_texts)
  add_asset_uint16('kDungeonPitsHurtPlayer', pits_hurt_player)
  
  print_entrance_info(entrances, 'kEntranceData_')
  print_entrance_info(starting_points, 'kStartingPoint_')

  data = []
  offsets = [0] * 8
  default_yaml = yaml.safe_load(open('dungeon/default_rooms.yaml', 'r'))
  for i in range(len(offsets)):
    offsets[i] = len(data)
    print_layer(default_yaml['Default%d' % i], None)
  add_asset_uint8('kDungeonRoomDefault', data)
  add_asset_uint16('kDungeonRoomDefaultOffs', offsets)

  data = []
  offsets = [0] * 19
  overlay_yaml = yaml.safe_load(open('dungeon/overlay_rooms.yaml', 'r'))
  for i in range(len(offsets)):
    offsets[i] = len(data)
    print_layer(overlay_yaml['Overlay%d' % i], None)
  add_asset_uint8('kDungeonRoomOverlay', data)
  add_asset_uint16('kDungeonRoomOverlayOffs', offsets)

  print_dungeon_secrets()

  add_asset_uint16('kDungAttrsForTile_Offs', ROM.get_words(0x8e9000, 21))
  add_asset_uint8('kDungAttrsForTile', ROM.get_bytes(0x8e902a, 1024))

  add_asset_uint16('kMovableBlockDataInit', ROM.get_words(0x84f1de, 198))
  add_asset_uint16('kTorchDataInit', ROM.get_words(0x84F36A, 144))
  add_asset_uint16('kTorchDataJunk', ROM.get_words(0x84F48a, 48))
 
def print_enemy_damage_data():
  decomp, comp_len = util.decomp(0x83e800, ROM.get_byte, True, True)
  add_asset_uint8('kEnemyDamageData', decomp)

def print_tilemaps():
  kSrcs = [0xcdd6d, 0xce7bf, 0xce2a8, 0xce63c, 0xce456, 0xeda9c]
  def decode_one(p):
    p_org = p
    while not (ROM.get_byte(p) & 0x80):
      is_memset = ROM.get_byte(p+2) & 0x40
      len = ((ROM.get_byte(p+2)*256+ROM.get_byte(p+3))&0x3fff) + 1
      p += 4
      p += 2 if is_memset else len
    return p - p_org + 1
  for i,s in enumerate(kSrcs):
    l = decode_one(s)
    add_asset_uint8('kBgTilemap_%d' % i, ROM.get_bytes(s, l))

def print_link_graphics():
  image = Image.open('linksprite.png')
  data = image.tobytes()
  def encode_4bit_sprite(data, offset, pitch):
    b = [0] * 32
    for y in range(8):
      for x in range(8):
        v = data[offset + y * pitch + x]
        b[y*2+0] |= (v & 1) << (7-x)
        b[y*2+1] |= (v >> 1 & 1) << (7-x)
        b[y*2+16] |= (v >> 2 & 1) << (7-x)
        b[y*2+17] |= (v >> 3 & 1) << (7-x)
    return bytes(b)
  b = b''
  for y in range(56):
    for x in range(16):
      b += encode_4bit_sprite(data, y * 128 * 8 + x * 8, 128)
  add_asset_uint8('kLinkGraphics', b)

def print_sound_banks():
  for song in ['intro', 'indoor', 'ending']:
    name, data = compile_music.print_song(song)
    add_asset_uint8(name, data)

def print_all(args):
  print_sound_banks()
  print_dungeon_rooms()
  print_enemy_damage_data()
  print_link_graphics()
  print_dungeon_sprites()
  print_map32_to_map16()
  print_images(args)
  print_misc(args)
  print_dialogue(args)
  print_dungeon_map()
  print_tilemaps()
  print_overworld()
  print_overworld_tables()

def write_assets_to_file(print_header = False):
  key_sig = b''
  all_data = []
  if print_header:
    print('''#pragma once
#include "types.h"

enum {
  kNumberOfAssets = %d
};
extern const uint8 *g_asset_ptrs[kNumberOfAssets];
extern uint32 g_asset_sizes[kNumberOfAssets];
extern MemBlk FindInAssetArray(int asset, int idx);
''' % len(assets))

  for i, (k, (tp, data)) in enumerate(assets.items()):
    if print_header:
      if tp == 'packed':
        print('#define %s(idx) FindInAssetArray(%d, idx)' % (k, i))
      else:
        print('#define %s ((%s*)g_asset_ptrs[%d])' % (k, tp, i))
        print('#define %s_SIZE (g_asset_sizes[%d])' % (k, i))
    key_sig += k.encode('utf8') + b'\0'
    all_data.append(data)

  assets_sig = b'Zelda3_v0     \n\0' + hashlib.sha256(key_sig).digest()

  if print_header:
    print('#define kAssets_Sig %s' % ", ".join((str(a) for a in assets_sig)))

  hdr = assets_sig + b'\x00' * 32 + struct.pack('II', len(all_data), len(key_sig))

  encoded_sizes = array.array('I', [len(i) for i in all_data])

  file_data = hdr + encoded_sizes + key_sig

  for v in all_data:
    while len(file_data) & 3:
      file_data += b'\0'
    file_data += v

  open('../zelda3_assets.dat', 'wb').write(file_data)

def main(args):
  print_all(args)
  write_assets_to_file(args.print_assets_header)

if __name__ == "__main__":
  ROM = util.load_rom(sys.argv[1] if len(sys.argv) >= 2 else None)
  class DefaultArgs:
    sprites_from_png = False
    languages = None
    print_assets_header = False
  main(DefaultArgs())
else:
  ROM = util.ROM

