import sys
import text_compression
import util
from PIL import Image
import yaml
import tables
import compile_music

print_int_array = util.print_int_array

PATH = ''

def flatten(xss):
    return [x for xs in xss for x in xs]
  
def invert_dict(xs):
  return {s:i for i,s in xs.items()}
    
def print_map32_to_map16():
  f = open(PATH+'generated_map32_to_map16.h', 'w')
  tab = {}
  for line in open(PATH + 'map32_to_map16.txt'):
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

  print_int_array('kMap32ToMap16_0', res[0], 'uint8', True, 16, file = f)
  print_int_array('kMap32ToMap16_1', res[1], 'uint8', True, 16, file = f)
  print_int_array('kMap32ToMap16_2', res[2], 'uint8', True, 16, file = f)
  print_int_array('kMap32ToMap16_3', res[3], 'uint8', True, 16, file = f)
  f.close()
  
def print_dialogue():
  f = open(PATH+'generated_dialogue.h', 'w')
  new_r = []
  offs = []
  for line in open(PATH + 'dialogue.txt'):
    line = line.strip('\n')
    a, b = line.split(': ', 1)
    index = int(a)
    offs.append(len(new_r))
    r = text_compression.compress_string(b)
    new_r.extend(r)

  print_int_array('kDialogueOffs', offs, 'uint16', True, 16, file = f)   
  print_int_array('kDialogueText', new_r, 'uint8', True, 16, file = f)
  f.close()

ROM = util.LoadedRom(sys.argv[1])


kCompSpritePtrs = [
  0x10f000,0x10f600,0x10fc00,0x118200,0x118800,0x118e00,0x119400,0x119a00,
  0x11a000,0x11a600,0x11ac00,0x11b200,0x14fffc,0x1585d4,0x158ab6,0x158fbe,
  0x1593f8,0x1599a6,0x159f32,0x15a3d7,0x15a8f1,0x15aec6,0x15b418,0x15b947,
  0x15bed0,0x15c449,0x15c975,0x15ce7c,0x15d394,0x15d8ac,0x15ddc0,0x15e34c,
  0x15e8e8,0x15ee31,0x15f3a6,0x15f92d,0x15feba,0x1682ff,0x1688e0,0x168e41,
  0x1692df,0x169883,0x169cd0,0x16a26e,0x16a275,0x16a787,0x16aa06,0x16ae9d,
  0x16b3ff,0x16b87e,0x16be6b,0x16c13d,0x16c619,0x16cbbb,0x16d0f1,0x16d641,
  0x16d95a,0x16dd99,0x16e278,0x16e760,0x16ed25,0x16f20f,0x16f6b7,0x16fa5f,
  0x16fd29,0x1781cd,0x17868d,0x178b62,0x178fd5,0x179527,0x17994b,0x179ea7,
  0x17a30e,0x17a805,0x17acf8,0x17b2a2,0x17b7f9,0x17bc93,0x17c237,0x17c78e,
  0x17cd55,0x17d2bc,0x17d82f,0x17dcec,0x17e1cc,0x17e36b,0x17e842,0x17eb38,
  0x17ed58,0x17f06c,0x17f4fd,0x17fa39,0x17ff86,0x18845c,0x1889a1,0x188d64,
  0x18919d,0x189610,0x189857,0x189b24,0x189dd2,0x18a03f,0x18a4ed,0x18a7ba,
  0x18aedf,0x18af0d,0x18b520,0x18b953,
]

kCompBgPtrs = [
  0x11b800,0x11bce2,0x11c15f,0x11c675,0x11cb84,0x11cf4c,0x11d2ce,0x11d726,
  0x11d9cf,0x11dec4,0x11e393,0x11e893,0x11ed7d,0x11f283,0x11f746,0x11fc21,
  0x11fff2,0x128498,0x128a0e,0x128f30,0x129326,0x129804,0x129d5b,0x12a272,
  0x12a6fe,0x12aa77,0x12ad83,0x12b167,0x12b51d,0x12b840,0x12bd54,0x12c1c9,
  0x12c73d,0x12cc86,0x12d198,0x12d6b1,0x12db6a,0x12e0ea,0x12e6bd,0x12eb51,
  0x12f135,0x12f6c5,0x12fc71,0x138129,0x138693,0x138bad,0x139117,0x139609,
  0x139b21,0x13a074,0x13a619,0x13ab2b,0x13b00c,0x13b4f5,0x13b9eb,0x13bebf,
  0x13c3ce,0x13c817,0x13cb68,0x13cfb5,0x13d460,0x13d8c2,0x13dd7a,0x13e266,
  0x13e7af,0x13ece5,0x13f245,0x13f6f0,0x13fc30,0x1480e9,0x14863b,0x148a7c,
  0x148f2a,0x149346,0x1497ed,0x149cc2,0x14a173,0x14a61d,0x14ab5d,0x14b083,
  0x14b4bd,0x14b94e,0x14be0e,0x14c291,0x14c7ba,0x14cce4,0x14d1db,0x14d6bd,
  0x14db77,0x14ded1,0x14e2ac,0x14e754,0x14ebae,0x14ef4e,0x14f309,0x14f6f4,
  0x14fa55,0x14ff8c,0x14ff93,0x14ff9a,0x14ffa1,0x14ffa8,0x14ffaf,0x14ffb6,
  0x14ffbd,0x14ffc4,0x14ffcb,0x14ffd2,0x14ffd9,0x14ffe0,0x14ffe7,0x14ffee,
  0x14fff5,0x18b520,0x18b953,
]

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
  
def print_images():
  rall = []
  f = open(PATH+'generated_images.h', 'w')
  for i in range(12):
    r = ROM.get_bytes(kCompSpritePtrs[i], 0x600)
    rall.append('kSprGfx_%d' % i)
    print_int_array('kSprGfx_%d' % i, r, 'uint8', True, 64, file = f)

  for i in range(12, 108):
    decomp, comp_len = util.decomp(kCompSpritePtrs[i], ROM.get_byte, False, True)
    r = ROM.get_bytes(kCompSpritePtrs[i], comp_len)

    #print('%d: %d -> %d' % (i, len(compress_store(decomp)), comp_len))
    rall.append('kSprGfx_%d' % i)
    print_int_array('kSprGfx_%d' % i, r, 'uint8', True, 64, file = f)

  print_int_array('kSprGfx', rall, 'uint8 *const', None, 8, file = f)

  rall = []
  for i in range(len(kCompBgPtrs)):
    decomp, comp_len = util.decomp(kCompBgPtrs[i], ROM.get_byte, False, True)
    r = ROM.get_bytes(kCompBgPtrs[i], comp_len)

    #print('%d: %d -> %d' % (i, len(compress_store(decomp)), comp_len))
    rall.append('kBgGfx_%d' % i)
    print_int_array('kBgGfx_%d' % i, r, 'uint8', True, 64, file = f)
    
  print_int_array('kBgGfx', rall, 'uint8 *const', None, 8, file = f)

  f.close()


def print_misc():
  f = open(PATH+'generated_overworld_map.h', 'w')
  print_int_array('kOverworldMapGfx', ROM.get_bytes(0x18c000, 0x4000), 'uint8', True, 64, file = f)
  print_int_array('kLightOverworldTilemap', ROM.get_bytes(0xac727, 4096), 'uint8', True, 64, file = f)
  print_int_array('kDarkOverworldTilemap', ROM.get_bytes(0xaD727, 1024), 'uint8', True, 64, file = f)
  f.close()

  f = open(PATH+'generated_predefined_tiles.h', 'w')
  print_int_array('kPredefinedTileData', ROM.get_words(0x9B52, 6438), 'uint16', False, 16, file = f)
  f.close()

  f = open(PATH+'generated_font.h', 'w')
  print_int_array('kFontData', ROM.get_words(0xe8000, 2048), 'uint16', False, 16, file = f)
  f.close()

  f = open(PATH+'generated_map16_to_map8.h', 'w')
  print_int_array('kMap16ToMap8', ROM.get_words(0x8f8000, 3752 * 4), 'uint16', False, 16, file = f)
  f.close()

  f = open(PATH+'generated_ancilla.h', 'w')
  print_int_array('kGeneratedWishPondItem', ROM.get_bytes(0x888450, 256), 'uint8', False, 16, file = f)
  print_int_array('kGeneratedBombosArr', ROM.get_bytes(0x8890FC, 256), 'uint8', False, 16, file = f)
  f.close()

  f = open(PATH+'generated_ending.h', 'w')
  print_int_array('kGeneratedEndSequence15', ROM.get_bytes(0x8ead25, 256), 'uint8', False, 16, file = f)
  print_int_array('kEnding_Credits_Text', ROM.get_bytes(0x8EB178, 1989), 'uint8', False, 16, file = f)
  print_int_array('kEnding_Credits_Offs', ROM.get_words(0x8EB93d, 394), 'uint16', False, 16, file = f)
  print_int_array('kEnding_MapData', ROM.get_words(0x8EB038, 160), 'uint16', False, 16, file = f)
  print_int_array('kEnding0_Offs', ROM.get_words(0x8EC2E1, 17), 'uint16', False, 16, file = f)
  print_int_array('kEnding0_Data', ROM.get_bytes(0x8EBF4C, 917), 'uint8', False, 16, file = f)
  f.close()

  f = open(PATH+'generated_palettes.h', 'w')
  print_int_array('kPalette_DungBgMain', ROM.get_words(0x9BD734, 1800), 'uint16', False, 15, file = f)
  print_int_array('kPalette_MainSpr', ROM.get_words(0x9BD218, 120), 'uint16', False, 15, file = f)

  print_int_array('kPalette_ArmorAndGloves', ROM.get_words(0x9BD308, 75), 'uint16', False, 15, file = f)
  print_int_array('kPalette_Sword', ROM.get_words(0x9BD630, 12), 'uint16', False, 3, file = f)
  print_int_array('kPalette_Shield', ROM.get_words(0x9BD648, 12), 'uint16', False, 4, file = f)

  print_int_array('kPalette_SpriteAux3', ROM.get_words(0x9BD39E, 84), 'uint16', False, 7, file = f)
  print_int_array('kPalette_MiscSprite_Indoors', ROM.get_words(0x9BD446, 77), 'uint16', False, 7, file = f)
  print_int_array('kPalette_SpriteAux1', ROM.get_words(0x9BD4E0, 168), 'uint16', False, 7, file = f)

  print_int_array('kPalette_OverworldBgMain', ROM.get_words(0x9BE6C8, 210), 'uint16', False, 7, file = f)
  print_int_array('kPalette_OverworldBgAux12', ROM.get_words(0x9BE86C, 420), 'uint16', False, 21, file = f)
  print_int_array('kPalette_OverworldBgAux3', ROM.get_words(0x9BE604, 98), 'uint16', False, 7, file = f)
  print_int_array('kPalette_PalaceMapBg', ROM.get_words(0x9BE544, 96), 'uint16', False,16, file = f)
  print_int_array('kPalette_PalaceMapSpr', ROM.get_words(0x9BD70A, 21), 'uint16', False,7, file = f)
  print_int_array('kHudPalData', ROM.get_words(0x9BD660, 64), 'uint16', False, 16, file = f)

  print_int_array('kOverworldMapPaletteData', ROM.get_words(0x8ADB27, 256), 'uint16', False, 16, file = f)

  f.close()

g_overworld_yaml_cache = {}
def load_overworld_yaml(room):
  if room not in g_overworld_yaml_cache:
    g_overworld_yaml_cache[room] = yaml.safe_load(open(PATH+'overworld/overworld-%d.yaml' % room, 'r'))
  return g_overworld_yaml_cache[room]
    

def print_overworld():
  f = open(PATH+'generated_overworld.h', 'w')

  r = []
  for i in range(160):
    addr = ROM.get_24(0x82F94D + i * 3)
    decomp, comp_len = util.decomp(addr, ROM.get_byte, True, True)
    r.append('kOverworld_Hibytes_Comp_%d' % i)
    print_int_array('kOverworld_Hibytes_Comp_%d' % i, ROM.get_bytes(addr, comp_len), 'uint8', True, 64, file = f)
  print_int_array('kOverworld_Hibytes_Comp', r, 'uint8 *const', None, 8, file = f)

  r = []
  for i in range(160):
    addr = ROM.get_24(0x82FB2D + i * 3)
    decomp, comp_len = util.decomp(addr, ROM.get_byte, True, True)
    r.append('kOverworld_Lobytes_Comp_%d' % i)
    print_int_array('kOverworld_Lobytes_Comp_%d' % i, ROM.get_bytes(addr, comp_len), 'uint8', True, 64, file = f)
  print_int_array('kOverworld_Lobytes_Comp', r, 'uint8 *const', None, 8, file = f)

  f.close()

def is_area_head(i):
  return i >= 128 or ROM.get_byte(0x82A5EC + (i & 63)) == (i & 63)

class OutArrays:
  def __init__(self):
    self.arrs = []
  def add(self, type, name, size, initializer = None, items_per_line = 16):
    t = [initializer] * size
    self.arrs.append((type, name, t, items_per_line))
    setattr(self, name, t)
  def write(self, f):
    for type, name, arr, items_per_line in self.arrs:
      for i, j in enumerate(arr):
        assert isinstance(j, int), (name, i, j, arr)
      print_int_array(name, arr, type, True, items_per_line, file = f)
    
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

  f = open(PATH+'generated_overworld_tables.h', 'w')
  A.write(f)
  print_int_array('kMap8DataToTileAttr', ROM.get_bytes(0x8E9459, 512), 'uint8', False, 16, file = f)
  print_int_array('kSomeTileAttr', ROM.get_bytes(0x9bf110, 3824), 'uint8', False, 16, file = f)
  f.close()  


def print_dungeon_map():
  f = open(PATH+'generated_dungeon_map.h', 'w')

  r, r2 = [], []
  name = 'kDungMap_FloorLayout'
  name2 = 'kDungMap_Tiles'
  for i in range(14):
    kSizes = [75, 125, 50, 75, 175, 75, 50, 75, 50, 200, 150, 75, 100, 200]
    addr = 0xa0000 + ROM.get_word(0x8AF605 + i * 2)
    r.append('%s_%d' % (name, i))
    bytes = ROM.get_bytes(addr, kSizes[i])
    nonzero_bytes = len(bytes) - bytes.count(0xf)
    print_int_array(r[-1], bytes, 'uint8', False, 25, file = f)
    addr = 0xa0000 + ROM.get_word(0x8AFBE4 + i * 2)
    r2.append('%s_%d' % (name2, i))
    bytes = ROM.get_bytes(addr, nonzero_bytes)
    print_int_array(r2[-1], bytes, 'uint8', False, 25, file = f)
    
  print_int_array(name, r, 'uint8 *const', None, 4, file = f)
  print_int_array(name2, r2, 'uint8 *const', None, 4, file = f)
  f.close()



g_dungeon_yaml_cache = {}
def load_dungeon_yaml(room):
  if room not in g_dungeon_yaml_cache:
    g_dungeon_yaml_cache[room] = yaml.safe_load(open(PATH+'dungeon/dungeon-%d.yaml' % room, 'r'))
  return g_dungeon_yaml_cache[room]
  
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
  f = open(PATH+'generated_dungeon_sprites.h', 'w')
  print_int_array('kDungeonSprites', data, 'uint8', True, 16, file = f)
  print_int_array('kDungeonSpriteOffs', offsets, 'uint16', True, 16, file=f)
  f.close()

def print_dungeon_secrets_to_file(f):
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
  print_int_array('kDungeonSecrets', result, 'uint8', True, 16, file = f)

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

  def print_entrance_info(f, entrances, prefix):
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
            
    print_int_array(prefix+'rooms', [a['room'] for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'relativeCoords', flatten([get_rc(a) for a in entrances]), 'uint8', True, 16, file = f)
    def get_base_x(a): return ((a['room'] & 0x00f) << 9)
    def get_base_y(a): return ((a['room'] & 0x1f0) << 5)
    print_int_array(prefix+'scrollX', [a['scroll_xy'][0] + get_base_x(a) for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'scrollY', [a['scroll_xy'][1] + get_base_y(a) for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'playerX', [a['player_xy'][0] + get_base_x(a) for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'playerY', [a['player_xy'][1] + get_base_y(a) for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'cameraX', [a['camera_xy'][0] for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'cameraY', [a['camera_xy'][1] for a in entrances], 'uint16', True, 16, file = f)
    print_int_array(prefix+'blockset', [a['blockset'] for a in entrances], 'uint8', True, 16, file = f)
    print_int_array(prefix+'floor', [a['floor'] for a in entrances], 'int8', True, 16, file = f)
    print_int_array(prefix+'palace', [get_palace(a['palace']) for a in entrances], 'int8', True, 16, file = f)
    print_int_array(prefix+'doorwayOrientation', [a['doorway_orientation'] for a in entrances], 'uint8', True, 16, file = f)
    print_int_array(prefix+'startingBg', [a['plane'] + a['ladder_level'] * 16 for a in entrances], 'uint8', True, 16, file = f)
    print_int_array(prefix+'quadrant1', [get_quadrant1(a['quadrants']) for a in entrances], 'uint8', True, 16, file = f)
    print_int_array(prefix+'quadrant2', [get_quadrant2(a['quadrants']) for a in entrances], 'uint8', True, 16, file = f)
    print_int_array(prefix+'doorSettings', [get_exit_door(a['house_exit_door']) for a in entrances], 'uint16', True, 16, file = f)
    if prefix == 'kStartingPoint_':
      print_int_array(prefix+'entrance', [a['associated_entrance_index'] for a in entrances], 'uint8', True, 16, file = f)
    m = invert_dict(tables.kMusicNames)
    print_int_array(prefix+'musicTrack', [m[a['music']] for a in entrances], 'uint8', True, 16, file = f)
    

  f = open(PATH+'generated_dungeon_rooms.h', 'w')
  print_int_array('kDungeonRoom', data, 'uint8', True, 16, file = f)
  print_int_array('kDungeonRoomOffs', offsets, 'uint16', True, 16, file = f)
  print_int_array('kDungeonRoomDoorOffs', door_offsets, 'uint16', True, 16, file = f)
  print_int_array('kDungeonRoomHeaders', room_headers, 'uint8', True, 16, file = f)
  print_int_array('kDungeonRoomHeadersOffs', header_offsets, 'uint16', True, 16, file = f)
  print_int_array('kDungeonRoomChests', chests, 'uint8', True, 16, file = f)
  print_int_array('kDungeonRoomTeleMsg', sign_texts, 'uint16', True, 16, file = f)
  print_int_array('kDungeonPitsHurtPlayer', pits_hurt_player, 'uint16', True, 16, file = f)
  
  print_entrance_info(f, entrances, 'kEntranceData_')
  print_entrance_info(f, starting_points, 'kStartingPoint_')

  data = []
  offsets = [0] * 8
  default_yaml = yaml.safe_load(open(PATH+'dungeon/default_rooms.yaml', 'r'))
  for i in range(len(offsets)):
    offsets[i] = len(data)
    print_layer(default_yaml['Default%d' % i], None)
  print_int_array('kDungeonRoomDefault', data, 'uint8', True, 16, file = f)
  print_int_array('kDungeonRoomDefaultOffs', offsets, 'uint16', True, 16, file = f)

  data = []
  offsets = [0] * 19
  overlay_yaml = yaml.safe_load(open(PATH+'dungeon/overlay_rooms.yaml', 'r'))
  for i in range(len(offsets)):
    offsets[i] = len(data)
    print_layer(overlay_yaml['Overlay%d' % i], None)
  print_int_array('kDungeonRoomOverlay', data, 'uint8', True, 16, file = f)
  print_int_array('kDungeonRoomOverlayOffs', offsets, 'uint16', True, 16, file = f)

  print_dungeon_secrets_to_file(f)

  print_int_array('kDungAttrsForTile_Offs', ROM.get_words(0x8e9000, 21), 'uint16', False, 16, file = f)
  print_int_array('kDungAttrsForTile', ROM.get_bytes(0x8e902a, 1024), 'uint8', False, 16, file = f)

  print_int_array('kMovableBlockDataInit', ROM.get_words(0x84f1de, 198), 'uint16', False, 16, file = f)
  print_int_array('kTorchDataInit', ROM.get_words(0x84F36A, 144), 'uint16', False, 16, file = f)
  print_int_array('kTorchDataJunk', ROM.get_words(0x84F48a, 48), 'uint16', False, 16, file = f)

  f.close()


 
def print_enemy_damage_data():
  f = open(PATH+'generated_enemy_damage_data.h', 'w')
  decomp, comp_len = util.decomp(0x83e800, ROM.get_byte, True, True)
  print_int_array('kEnemyDamageData', decomp, 'uint8', False, 32, file = f)
  f.close()

def print_tilemaps():
  f = open(PATH+'generated_bg_tilemaps.h', 'w')
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
    print_int_array('kBgTilemap_%d' % i, ROM.get_bytes(s, l), 'uint8', False, 32, file = f)
  f.close()

def print_link_graphics():
  f = open(PATH+'generated_link_graphics.h', 'w')
  image = Image.open(PATH+'linksprite.png')
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
  print_int_array('kLinkGraphics', b, 'uint8', False, 32, file = f)
  #print(list(encode_4bit_sprite(data, 0, 128)))
  f.close()

def print_sound_banks():
  f = open(PATH+'generated_sound_banks.h', 'w')
  for song in ['intro', 'indoor', 'ending']:
    compile_music.print_song(song, f)
  f.close()



def print_all():
  print_sound_banks()
  print_dungeon_rooms()
  print_enemy_damage_data()
  print_link_graphics()
  print_dungeon_sprites()
  print_map32_to_map16()
  print_dialogue()
  print_images()
  print_misc()
  print_dungeon_map()
  print_tilemaps()
  print_overworld()
  print_overworld_tables()

print_all()

