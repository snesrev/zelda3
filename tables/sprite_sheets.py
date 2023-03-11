from PIL import Image
import sprite_sheet_info
import util
from util import get_bytes, get_words, get_byte, cache
import array
import tables
import sys

override_armor_palette = None
#override_armor_palette = [0x7fff, 0x237e, 0x11b7, 0x369e, 0x14a5,  0x1ff, 0x1078, 0x599d, 0x3647, 0x3b68, 0xa4a, 0x12ef, 0x2a5c, 0x1571, 0x7a18,
#                          0x7fff, 0x237e, 0x11b7, 0x369e, 0x14a5,  0x1ff, 0x1078, 0x599d, 0x6980, 0x7691, 0x26b8, 0x437f, 0x2a5c, 0x1199, 0x7a18,
#                          0x7fff, 0x237e, 0x11b7, 0x369e, 0x14a5,  0x1ff, 0x1078, 0x599d, 0x1057, 0x457e, 0x6df3, 0xfeb9, 0x2a5c, 0x2227, 0x7a18,
#                          0x7fff, 0x237e, 0x11b7, 0x369e, 0x14a5,  0x1ff, 0x1078, 0x3d97, 0x3647, 0x3b68, 0xa4a, 0x12ef, 0x567e, 0x1571, 0x7a18,
#                          0,      0x0EFA, 0x7DD1, 0,      0x7F1A,  0x7F1A,0,      0x716E, 0x7DD1, 0x40A7, 0x7DD1, 0x40A7, 0x48E9, 0x50CF, 0x7FFF]


def save_as_png(dimensions, data, fname, palette = None):
  img = Image.new('L' if palette == None else 'P', dimensions)
  img.putdata(data)
  if palette != None:
    img.putpalette(palette)
  img.save(fname)

def save_as_24bpp_png(dimensions, data, fname):
  img = Image.new("RGB", dimensions)
  img.putdata(data)
  img.save(fname)

@cache
def decode_2bit_tileset(tileset, height = 32, base = 0):
  data = util.decomp(tables.kCompSpritePtrs[tileset], get_byte, False)
  assert len(data) == 0x400 * height // 32
  dst = bytearray(128*height)
  def decode_2bit(offs, toffs, base):
    for y in range(8):
      d0, d1 = data[offs + y * 2], data[offs + y * 2 + 1]
      for x in range(8):
        t = ((d0 >> x) & 1) * 1 + ((d1 >> x) & 1) * 2
        dst[toffs + y * 128 + (7 - x)] = t + base
  for i in range(16*height//8):
    x = i % 16
    y = i // 16
    decode_2bit(i * 16, x * 8 + y * 8 * 128, base[i] if isinstance(base, tuple) else base)
  return dst

def is_high_3bit_tileset(tileset):
  # is 0x5c, 0x5e, 0x5f included or not?
  return tileset in (0x52, 0x53, 0x5a, 0x5b, 0x5c, 0x5e, 0x5f)

def get_unpacked_snes_tileset(tileset):
  if tileset < 12:
    return get_bytes(tables.kCompSpritePtrs[tileset], 0x600)
  else:
    return util.decomp(tables.kCompSpritePtrs[tileset], get_byte, False)

@cache
def decode_3bit_tileset(tileset):
  data = get_unpacked_snes_tileset(tileset)
  assert len(data) == 0x600
  base = 8 if is_high_3bit_tileset(tileset) else 0
  height = 32
  dst = bytearray(128*height)
  def decode_3bit(offs, toffs):
    for y in range(8):
      d0, d1, d2 = data[offs + y * 2], data[offs + y * 2 + 1], data[offs + y + 16]
      for x in range(8):
        t = ((d0 >> x) & 1) * 1 + ((d1 >> x) & 1) * 2 + ((d2 >> x) & 1) * 4
        dst[toffs + y * 128 + (7 - x)] = base + t
  for i in range(16*height//8):
    x, y = i % 16, i // 16
    decode_3bit(i * 24, x * 8 + y * 8 * 128)
  return dst

def decode_4bit_tileset_link():
  height = 448
  data = get_bytes(0x108000, 0x800 * height // 32) # only link sprites for now
  dst = bytearray(128*height)
  def decode_4bit(offs, toffs):
    for y in range(8):
      d0, d1, d2, d3 = data[offs + y * 2 + 0], data[offs + y * 2 + 1], data[offs + y * 2 + 16], data[offs + y * 2 + 17]
      for x in range(8):
        t = ((d0 >> x) & 1) * 1 + ((d1 >> x) & 1) * 2 + ((d2 >> x) & 1) * 4 + ((d3 >> x) & 1) * 8
        dst[toffs + y * 128 + (7 - x)] = t
  for i in range(16*height//8):
    x, y = i % 16, i // 16
    decode_4bit(i * 32, x * 8 + y * 8 * 128)
  return dst

def convert_snes_palette(v):
  rr=bytearray()
  for x in v:
    r, g, b = x & 0x1f, x >> 5 & 0x1f, x >> 10 & 0x1f
    rr.extend((r << 3 | r >> 2, g << 3 | g >> 2, b << 3 | b >> 2))
  return rr

def convert_int24_palette_to_bytes(v):
  rr=bytearray()
  for x in v:
    rr.extend((x & 0xff, x >> 8 & 0xff, x >> 16 & 0xff))
  return rr

def convert_snes_palette_to_int(v):
  rr=[]
  for x in v:
    r, g, b = x & 0x1f, x >> 5 & 0x1f, x >> 10 & 0x1f
    rr.append((r << 3 | r >> 2) | (g << 3 | g >> 2) << 8 | (b << 3 | b >> 2) << 16)
  return rr

def decode_link_sprites():
  kLinkPalette = [0, 0x7fff, 0x237e, 0x11b7, 0x369e, 0x14a5,  0x1ff, 0x1078, 0x599d, 0x3647, 0x3b68,  0xa4a, 0x12ef, 0x2a5c, 0x1571, 0x7a18]
  save_as_png((128, 448), decode_4bit_tileset_link(), 'linksprite.png', convert_snes_palette(kLinkPalette))

def get_hud_snes_palette():
  hud_palette  = get_words(0x9BD660, 64)
  palette = [(31 << 10 | 31) for i in range(256)]
  for i in range(16):
    for j in range(1, 4):
      palette[i * 16 + j] = hud_palette[i * 4 + j]
  return palette

def decode_hud_icons():
  class PaletteUsage:
    def __init__(self):
      self.data = open('palette_usage.bin', 'rb').read()
    def get(self, icon):
      usage = self.data[icon]
      for j in range(8):
        if usage & (1 << j):
          return j 
      return 0
  pu = PaletteUsage()
  dst = bytearray()
  for slot, image_set in enumerate([106, 107, 105]):
    tbase = tuple([pu.get(slot * 128 + i) * 16 for i in range(128)])
    dst += decode_2bit_tileset(image_set, height = 64, base = tbase)

  save_as_png((128, 64 * 3), dst, 'hud_icons.png', convert_snes_palette(get_hud_snes_palette()[:128]))

def get_pt_remapper():
  b = util.ROM.get_bytes(0x8EFC09, 121 * 3)
  d = {}
  for i in range(121):
    ch = (i & 0xf) | (i << 1) & 0xe0
    d[ch] = b[i*3+0]
    d[ch|0x10] = b[i*3+1]
  return d

kFontTypes = {
  'us'   : (0x8e8000, 256, 'font.png', (0x8ECADF, 99)),
  'de'   : (0xCC6E8, 256, 'font_de.png', (0x8CDECF, 112)),
  'fr'   : (0xCC6E8, 256, 'font_fr.png', (0x8CDEAF, 112)),
  'fr-c' : (0xCD078, 256, 'font_fr_c.png', (0x8CE83F, 112)),
  'en'   : (0x8E8000, 256, 'font_en.png', (0x8ECAFF, 102)),
  'es'   : (0x8e8000, 256, 'font_es.png', (0x8ECADF, 99)),
  'pl'   : (0x8e8000, 256, 'font_pl.png', (0x8ECADF, 99)),
  'pt'   : (0x8e8000, 256, 'font_pt.png', (0x8ECADF, 121)),
  'redux': (0x8e8000, 256, 'font_redux.png', (0x8ECADF, 99)),
  'nl': (0x8e8000, 256, 'font_nl.png', (0x8ECADF, 99)),
  'sv': (0x8e8000, 256, 'font_sv.png', (0x8ECADF, 99)),
}

def decode_font():
  lang = util.ROM.language
  def decomp_one_spr_2bit(data, offs, target, toffs, pitch, palette_base):
    for y in range(8):
      d0, d1 = data[offs + y * 2], data[offs + y * 2 + 1]
      for x in range(8):
        t = ((d0 >> x) & 1) * 1 + ((d1 >> x) & 1) * 2
        target[toffs + y * pitch + (7 - x)] = t + palette_base
  ft = kFontTypes[lang]
  if lang == 'pt':
    W = util.ROM.get_bytes(0x8EFC09, 121 * 3)
    W = [W[i*3+2] for i in range(121)]
    remapper = get_pt_remapper()
  else:
    W = get_bytes(*ft[3])
    remapper = {}
  w = 128 + 15
  hi = ft[1] // 32
  h = hi * 17
  data = get_bytes(ft[0], ft[1] * 16)
  dst = bytearray(w * h)
  
  for i in range(ft[1]):
    x, y = i % 16, i // 16
    pal_base = 6 * 16
    base_offs = x * 9 + (y * 8 + (y >> 1)) * w
    decomp_one_spr_2bit(data, remapper.get(i, i) * 16, dst, base_offs + w, w, pal_base)
    if (y & 1) == 0:
      j = (y >> 1) * 16 + x
      if j < len(W):
        dst[base_offs + W[j] - 1] = 255
  pal = convert_snes_palette(get_hud_snes_palette()[:128])
  pal.extend([0] * 384)
  pal[0], pal[1], pal[2] = 192, 192, 192
  pal[255*3+0], pal[255*3+1], pal[255*3+2] = 128, 128, 128
  save_as_png((w, h), dst, ft[2], pal)
  if lang != 'pt':
    assert (data, W) == encode_font_from_png(lang)

def encode_font_from_png(lang):
  font_data = Image.open(kFontTypes[lang][2]).tobytes()
  def encode_one_spr_2bit(data, offs, target, toffs, pitch):
    for y in range(8):
      d0, d1 = 0, 0
      for x in range(8):
        pixel = data[offs + y * pitch + 7 - x]
        d0 |= (pixel & 1) << x
        d1 |= ((pixel >> 1) & 1) << x
      target[toffs + y * 2 + 0], target[toffs + y * 2 + 1] = d0, d1
  w = 128 + 15
  dst = bytearray(256 * 16)
  def get_width(base_offs):
    for i in range(8):
      if font_data[base_offs + i] == 255:
        break
    return i + 1
  W = bytearray()
  for i in range(256):
    x, y = i % 16, i // 16
    base_offs = x * 9 + (y * 8 + (y >> 1)) * w
    if (y & 1) == 0:
      W.append(get_width(base_offs))
    encode_one_spr_2bit(font_data, base_offs + w, dst, i * 16, w)
  chars_per_lang = kFontTypes[lang][3][1]
  return dst, W[:chars_per_lang]

# Returns the dungeon palette for the specified palette index
# 0 = lightworld, 1 = darkworld, 2 = dungeon
@cache
def get_palette_subidx(palset_idx, dungeon_or_ow, which_palette):
  spmain = 1 if (dungeon_or_ow == 1) else 0
  if dungeon_or_ow == 2:
    main, sp0l, sp5l, sp6l = tables.kDungPalinfos[palset_idx]
    sp0r = (main // 2) + 11
    sp6r = 10
  else:
    sp5l, sp6l = tables.kOwSprPalInfo[palset_idx * 2 : palset_idx * 2 + 2]
    sp0l = 3 if dungeon_or_ow == 1 else 1
    sp0r = 9 if dungeon_or_ow == 1 else 7
    sp6r = 8 if dungeon_or_ow == 1 else 6
  pal_defs = (sp0l, sp0r, 
    spmain, None, spmain, None, spmain, None, None, None,
    sp5l, None,
    sp6l, sp6r,
    None, None)
  return pal_defs[which_palette]

@cache
def get_palette_subset(pal_idx, j):
  if j == None: j = 0
  pal = [0] * 7
  if pal_idx == 0: # 0
    kPalette_SpriteAux3 = get_words(0x9BD39E, 84)
    return kPalette_SpriteAux3[j * 7 : j * 7 + 7]
  if pal_idx == 1: # 0R
    if j < 11:
      kPalette_MiscSprite = get_words(0x9BD446, 77)
      return kPalette_MiscSprite[j * 7 : j * 7 + 7]
    else:
      kPalette_DungBgMain = get_words(0x9BD734, 1800)
      return kPalette_DungBgMain[(j - 11) * 90 : (j - 11) * 90 + 7]
  if pal_idx in (2, 3, 4, 5, 6, 7, 8, 9):
    o = j * 60 + ((pal_idx - 2) >> 1) * 15 + (pal_idx & 1) * 8
    kPalette_MainSpr = get_words(0x9BD218, 120)
    return kPalette_MainSpr[o : o + 7]
  if pal_idx in (10, 12):
    kPalette_SpriteAux1 = get_words(0x9BD4E0, 168)
    return kPalette_SpriteAux1[j * 7 : j * 7 + 7]
  assert pal_idx != 11
  if pal_idx == 13:
    kPalette_MiscSprite = get_words(0x9BD446, 77)
    return kPalette_MiscSprite[j * 7 : j * 7 + 7]
  elif pal_idx == 14:
    kPalette_ArmorAndGloves = get_words(0x9BD308, 75)
    return kPalette_ArmorAndGloves[1:8]
  elif pal_idx == 15:
    kPalette_ArmorAndGloves = get_words(0x9BD308, 75)
    return kPalette_ArmorAndGloves[9:16]

@cache
def get_full_palette(pal_idx, pal_subidx):
  rv = []
  if pal_idx & 1:
    rv.extend([0x00fe00] * 9)
  else:
    rv.extend([0x00fe00] * 1)
  rv.extend(convert_snes_palette_to_int(get_palette_subset(pal_idx, pal_subidx)))
  rv += [0xe000e0] * (256 - len(rv))
  # change the transparent to teal #008080
  for i in range(0, 128, 8):
    rv[i] = 0x808000
  rv[251] = 0xe0c0c0 # blueish text
  rv[252] = 0xc0c0c0 # palette text
  rv[253] = 0xf0f0f0 # unallocated
  rv[254] = 0x404040 # lines
  rv[255] = 0xe0e0e0 # bg
  return rv

@cache
def get_font_3x5():
  return Image.open('../other/3x5_font.png').tobytes()

def draw_letter3x5(dst, dst_pitch, dx, dy, ch, color):
  font = get_font_3x5()
  dst_offs = dy * dst_pitch + dx
  src_offs = (ord(ch) & 31) * 4 + (ord(ch) // 32) * 6 * 128
  for y in range(6):
    for x in range(3):
      if font[src_offs + y * 128 + x] == 0:
        dst[dst_offs + y * dst_pitch + x] = color

def draw_string3x5(dst, dst_pitch, dx, dy, s, color):
  for ch in s:
    draw_letter3x5(dst, dst_pitch, dx, dy, ch, color)
    dx += 4

def convert_to_24bpp(data, palette):
  out = [0] * len(data)
  for i in range(len(data)):
    out[i] = palette[data[i]]
  return out

def concat_byte_arrays(ar):
  r = bytearray()
  for a in ar:
    r += a
  return r

def fixup_sprite_set_entry(e, e_prev):
  sprite_index = int(e.name[:2], 16) if e.name[0] != 'X' else 10000

  if e.dungeon_or_ow != 3:
    e.tileset = tables.kSpriteTilesets[e.tileset + (64 if e.dungeon_or_ow == 2 else 0)][e.ss_idx]
  e.high_palette = is_high_3bit_tileset(e.tileset)
  e.skip_header = False
  if e.pal_base == None:
    if sprite_index < len(tables.kSpriteInit_Flags3):
      e.pal_base = (tables.kSpriteInit_Flags3[sprite_index] >> 1) & 7
    else:
      e.pal_base = 4 # just default to some palette

  e.pal_idx = e.pal_base * 2 + e.high_palette
  e.pal_subidx = get_palette_subidx(e.palset_idx, e.dungeon_or_ow, e.pal_idx)

  if e_prev != None and e_prev.name == e.name and e_prev.pal_idx == e.pal_idx and e_prev.pal_subidx == e.pal_subidx:
    e.skip_header = True

  # the (pal_idx, pal_subidx) tuple identifies the palette
  e.encoded_id = e.pal_base | e.tileset << 3 | e.skip_header << 10 | (e.pal_subidx or 0) << 11 
  e.encoded_id = e.encoded_id << 8 | ((e.encoded_id + 41) % 255)


BIGW = 148
def save_sprite_set_entry(finaldst, e, master_tilesheets = None):
  empty_253 = [253] * 8
  def fill_with_empty(dst, dst_offs):
    for y in range(8):
      o = dst_offs + y * BIGW
      dst[o : o + 8] = empty_253

  def copy_8x8(dst, dst_offs, src, src_offs, base):
    for y in range(8):
      for x in range(8):
        dst[dst_offs + y * BIGW + x] = src[src_offs + y * 128 + x] + base

  def hline(dst, x1, x2, y, color):
    for x in range(x1, x2 + 1):
      dst[y * BIGW + x] = color

  def vline(dst, x, y1, y2, color):
    for y in range(y1, y2 + 1):
      dst[y * BIGW + x] = color
  
  def fillrect(dst, x1, x2, y1, y2, color):
    for y in range(y1, y2 + 1):
      hline(dst, x1, x2, y, color)

  def encode_id(dst, x, y, v):
    while v:
      if v & 1:
        dst[y * BIGW + x] = 253
      x -= 1
      v >>= 1

  palette = get_full_palette(e.pal_idx, e.pal_subidx)

  if not e.skip_header:
    pal_subidx = e.pal_subidx
    if e.high_palette:
      pal_name = '%sR' % e.pal_base
    else:
      pal_name = '%s' % e.pal_base
      if e.pal_base in (1, 2, 3):
        assert e.pal_subidx in (0, 1)
        pal_subidx = 'LW' if pal_subidx == 0 else 'DW'
    if pal_subidx != None:
      pal_name = '%s-%s' % (pal_name, pal_subidx)

    header = bytearray([255] * BIGW * 9)
    draw_string3x5(header, BIGW, 1, 3, e.name[:22], 254)

    for i in range(7):
      xx = BIGW - 37 + i * 5 - 9
      fillrect(header, xx, xx + 4, 3, 7, i + (9 if e.high_palette else 1))

    draw_string3x5(header, BIGW, BIGW - 9, 3, '%2s' % e.tileset, 252)
    draw_string3x5(header, BIGW, BIGW - 45 - 1 - len(pal_name) * 4, 3, pal_name, 252)
    finaldst.extend(convert_to_24bpp(header, palette))

  bigdst = bytearray([255] * BIGW * 36)
  hline(bigdst, 1, 137, 0, 254)
  hline(bigdst, 1, 137, 34, 254)
  vline(bigdst, 1, 0, 34, 254)
  vline(bigdst, 137, 0, 34, 254)

  encode_id(bigdst, 137, 35, e.encoded_id << 9 | 0x55)

  src = decode_3bit_tileset(e.tileset) # returns 128x64

  for i in range(16*4):
    x, y = i % 16, i // 16
    dst_offs = (y * 8 + 1 + (y >> 1)) * BIGW + x * 8 + 2 + (x >> 1)
    if x & 8: dst_offs
    m = e.matrix[y][x]
    if m == '.':
      fill_with_empty(bigdst, dst_offs)
    else:
      copy_8x8(bigdst, dst_offs, src, x * 8 + y * 8 * 128, 0)
      if master_tilesheets:
        master_tilesheets.insert(e.tileset, src, x, y, palette, 0)

  # display vram addresses
  draw_string3x5(bigdst, BIGW, BIGW - 9, 4, '%Xx' % (e.ss_idx * 0x4), 251)
  draw_string3x5(bigdst, BIGW, BIGW - 9, 4 + 17, '%Xx' % (e.ss_idx * 0x4 + 2), 251)

  # collapse unused matrix sections
  if all(m=='.' for m in e.matrix[0]) and all(m=='.' for m in e.matrix[1]):
    del bigdst[1*BIGW:17*BIGW]
  elif all(m=='.' for m in e.matrix[2]) and all(m=='.' for m in e.matrix[3]):
    del bigdst[18*BIGW:34*BIGW]

  if e.skip_header:
    draw_string3x5(bigdst, BIGW, BIGW - 9, len(bigdst)//BIGW - 6, '%.2d' % e.tileset, 252)
  
  finaldst.extend(convert_to_24bpp(bigdst, palette))

def fixup_entries():
  e_prev = None
  for e in sprite_sheet_info.entries:
    fixup_sprite_set_entry(e, e_prev)
    e_prev = e

class MasterTilesheets:
  def __init__(self):
    self.sheets = {}

  def insert(self, tileset, src, xp, yp, palette, pal_base):
    sheet = self.sheets.get(tileset)
    if sheet == None:
      sheet = (array.array('I', [0x00f000] * 128 * 32), None)
      self.sheets[tileset] = sheet
    sheet24 = sheet[0]
    for y in range(yp * 8, yp * 8 + 8):
      for x in range(xp * 8, xp * 8 + 8):
        o = y * 128 + x
        sheet24[o] = palette[pal_base + src[o]]

  def add_verify_8x8(self, tileset, pal_lut, img_data, pitch, dst_pos, src_pos):
    # add to the master sheet
    sheet = self.sheets.get(tileset)
    if sheet == None:
      sheet = (array.array('I', [0x00f000] * 128 * 32), bytearray([248] * 128 * 32))
      self.sheets[tileset] = sheet
    sheet24, sheet8 = sheet[0], sheet[1]
    for y in range(8):
      for x in range(8):
        pixel = img_data[src_pos + y * pitch + x]
        o = dst_pos + y * 128 + x 
        sheet24[o] = pixel
        pixel8 = pal_lut.get(pixel)
        if pixel8 == None:
          raise Exception('Pixel #%.6x not found' % pixel)
        if sheet8[o] != 248 and pixel8 != sheet8[o]:
          raise Exception('Pixel has more than one value')
        sheet8[o] = pixel8

  def save_to_all_sheets(self, save_also_8bit = False):
    rv = array.array('I')
    if save_also_8bit:
      rv8 = bytearray()

    def extend_with_string_line(text):
      header = array.array('I', [0xf0f0f0] * 128 * 8)
      draw_string3x5(header, 128, 1, 2, text, 0x404040)
      rv.extend(header)
      if save_also_8bit:
        header8 = bytearray([255] * 128 * 8)
        draw_string3x5(header8, 128, 1, 2, text, 254)
        rv8.extend(header8)
    extend_with_string_line('AUTO GENERATED DO NOT EDIT')
    kk = 0
    for k in sorted(self.sheets.keys()):
      extend_with_string_line('Sheet %d' % k)
      rv.extend(self.sheets[k][0])
      if save_also_8bit:
        rv8.extend(self.sheets[k][1])
      if k != kk:
        print('Missing %d' % kk)
      kk = k + 1
    if save_also_8bit:
      palette8 = get_full_palette(4, 0)
      for i, v in enumerate(convert_snes_palette_to_int(get_palette_subset(5, 0))):
        palette8[i + 9] = v
      palette8[248] = 0x00f000
      save_as_png((128, len(rv)//128), rv8, 'sprites/all_sheets_8bit.png', palette = convert_int24_palette_to_bytes(palette8))
      self.verify_identical_to_snes()
    save_as_24bpp_png((128, len(rv)//128), rv, 'sprites/all_sheets.png')

  def verify_identical_to_snes(self):
    for tileset in range(103):
      a = self.encode_sheet_in_snes_format(tileset)
      b = get_unpacked_snes_tileset(tileset)
      if a != b:
        print('Sheet %d mismatch' % tileset)
        break

  # convert a tileset to the 8bit snes format, 24 bytes per tile
  def encode_sheet_in_snes_format(self, tileset):
    sheet8 = self.sheets[tileset][1]
    result = bytearray(24 * 16 * 4)
    def encode_into(dst_pos, src_pos):
      for y in range(8):
        for x in range(8):
          b = sheet8[src_pos + 128 * y + 7 - x]
          result[dst_pos+2*y] |= (b & 1) << x
          result[dst_pos+2*y+1] |= ((b & 2) >> 1) << x
          result[dst_pos+y+16] |= ((b & 4) >> 2) << x
    for y in range(4):
      for x in range(16):
        encode_into((y * 16 + x) * 24, y * 8 * 128 + x * 8)
    return result

def decode_sprite_sheets():
  master_tilesheets = MasterTilesheets()

  fixup_entries()
  for char in "0123456789ABCDEFX":
    dst = []
    for e in sprite_sheet_info.entries:
      if e.name[0].upper() == char:
        save_sprite_set_entry(dst, e, master_tilesheets)
    if len(dst):
      save_as_24bpp_png((BIGW, len(dst) // BIGW), dst, 'sprites/sprites_%s.png' % char)
  master_tilesheets.save_to_all_sheets()

def load_sprite_sheets():
  master_tilesheets = MasterTilesheets()
  for char in "0123456789ABCDEFX":
    img = Image.open('sprites/sprites_%s.png' % char)
    img_size = img.size
    img_data = array.array('I', (a[0] | a[1] << 8 | a[2] << 16 for a in img.getdata()))
    pitch = img.size[0]

    def find_next_tag(start_pos):
      for i in range(start_pos, len(img_data) - pitch * 2, pitch if start_pos != 0 else 1):
        if img_data[i+0] == 0x404040 and img_data[i+pitch] == 0x404040 and \
          img_data[i+pitch * 2 + 0] == 0xf0f0f0 and img_data[i+pitch * 2 - 1] == 0xe0e0e0 and \
          img_data[i+pitch * 2 - 2] == 0xf0f0f0 and img_data[i+pitch * 2 - 3] == 0xe0e0e0 and \
          img_data[i+pitch * 2 - 4] == 0xf0f0f0 and img_data[i+pitch * 2 - 5] == 0xe0e0e0 and \
          img_data[i+pitch * 2 - 6] == 0xf0f0f0 and img_data[i+pitch * 2 - 7] == 0xe0e0e0:
          return i + pitch * 2
      else:
        return None

    def decode_tag(pos):
      r = 0
      for i in range(64):
        v = img_data[pos - 63 + i]
        assert v in (0xe0e0e0, 0xf0f0f0)
        r = r * 2 + (v == 0xf0f0f0)
      if (r & 0x1ff) != 0x55:
        raise Exception('Invalid tag')
      r >>= 9
      if (((r >> 8) + 41) % 255) != (r & 0xff):
        raise Exception('Invalid checksum')
      r >>= 8
      return r & 7, (r >> 3) & 127, (r >> 10) & 1, (r >> 11) & 31

    def determine_image_rects(tag_pos):
      h = 1
      while (img_data[tag_pos - pitch * (h + 1)] == 0x404040):
        h += 1
      if h == 19:
        if img_data[tag_pos - pitch * 2 - 1] == 0xe0e0e0:
          image_rects = ((0, tag_pos - 135 - pitch * 18), )
        elif img_data[tag_pos - pitch * 18 - 1] == 0xe0e0e0:
          image_rects = ((1, tag_pos - 135 - pitch * 17), )
        else:
          raise Exception('cant uncompact')
      elif h == 35:
        image_rects = ((0, tag_pos - 135 - pitch * 34), (1, tag_pos - 135 - pitch * 17))
      else:
        raise Exception('height not found')
      return image_rects, h

    def get_pal_lut(tag_pos, is_high):
      # read palette colors from the image pixels
      pal_lut = {img_data[tag_pos + 5 * i] : i + (9 if is_high else 1) for i in range(7)}
      pal_lut[0x808000] = 0 # transparent color
      return pal_lut

    def is_empty(pos):
      return all(img_data[pos+y*pitch+x] == 0xf0f0f0 for x in range(8) for y in range(8))

    pal_lut = None
    tag_pos = 0
    while True:
      tag_pos = find_next_tag(tag_pos)
      if tag_pos == None:
        break
      pal_base, tileset, headerless, pal_subidx = decode_tag(tag_pos)
      image_rects, box_height = determine_image_rects(tag_pos)
      if not headerless:
        pal_lut = get_pal_lut(tag_pos - pitch * (box_height + 3) - 34, is_high_3bit_tileset(tileset))
      for idx, sheet_pos in image_rects:
        for y in range(2):
          for x in range(16):
            src_pos = sheet_pos + (y * 8 + (y >> 1)) * pitch + (x * 8 + (x >> 1))
            dst_pos = (y + idx * 2) * 8 * 128 + x * 8
            if not is_empty(src_pos):
              master_tilesheets.add_verify_8x8(tileset, pal_lut, img_data, pitch, dst_pos, src_pos)
  return master_tilesheets

#if __name__ == "__main__":
#  ROM = util.load_rom(sys.argv[1] if len(sys.argv) >= 2 else None, True)
#  decode_font()
  


