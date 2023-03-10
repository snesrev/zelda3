import array
import sys
import hashlib
import os
from functools import lru_cache

def cache(user_function):
  'Simple lightweight unbounded cache.  Sometimes called "memoize".'
  return lru_cache(maxsize=None)(user_function)

# Both are common SNES rom extensions. For Zelda3 (NA), they are equivalent files.
COMMON_ROM_NAMES = ['zelda3.sfc', 'zelda3.smc']
DEFAULT_ROM_DIRECTORY = os.path.dirname(__file__)

ZELDA3_SHA256_US = '66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb'
ZELDA3_SHA256 = {
  '030ff80d0087bca440094cd914c03da0aa199dc6edb9adfb43f1267e99fde45f' : 'de',
  '29a0d5812ccbb4b11bdb55d8f751a2a797b4110bf402ca8ba15eb2bf85db7b39' : 'fr',
   ZELDA3_SHA256_US : 'us',
}

def load_rom(filename, support_multilanguage = False):
  global ROM
  ROM = LoadedRom(filename, support_multilanguage)
  return ROM

def get_byte(addr):
  return ROM.get_byte(addr)

@cache
def get_bytes(addr, n):
  return ROM.get_bytes(addr, n)

@cache
def get_words(addr, n):
  return ROM.get_words(addr, n)

def get_int8(ea):
  b = get_byte(ea)
  if b & 0x80: b -= 256
  return b

def get_int16(ea):
  b = get_word(ea)
  if b & 0x8000: b -= 65536
  return b

def get_word(addr):
  return ROM.get_word(addr)


class LoadedRom:
  def __init__(self, path = None, support_multilanguage = False):
    rom_path = self.__get_rom_path(path)
    self.ROM = open(rom_path, 'rb').read()
    hash = hashlib.sha256(self.ROM).hexdigest()
    self.language = ZELDA3_SHA256.get(hash)

    if support_multilanguage:
      if self.language == None:
        raise Exception(f"ROM with hash {hash} not supported.");
    else:
      if self.language != 'us':
        raise Exception(f"ROM with hash {hash} not supported. Expected {ZELDA3_SHA256_US}. Please verify your ROM is the NA 1.0 version.");

  def get_byte(self, ea):
    assert (ea & 0x8000)
    ea = ((ea >> 16) & 0x7f) * 0x8000 + (ea & 0x7fff)
    return self.ROM[ea]

  def get_word(self, ea):
    return self.get_byte(ea) + self.get_byte(ea + 1) * 256

  def get_24(self, ea):
    return self.get_byte(ea) + self.get_byte(ea + 1) * 256 + self.get_byte(ea + 2) * 65536

  def get_bytes(self, addr, n):
    r = bytearray()
    for i in range(n):
      r.append(self.get_byte(addr))
      addr += 1
      if (addr & 0x8000) == 0:
        addr += 0x8000
    return r

  def get_words(self, addr, n):
    r = []
    for i in range(n):
      r.append(self.get_word(addr))
      addr += 2
      if (addr & 0x8000) == 0:
        addr += 0x8000
    return r

  def __get_rom_path(self, path = None):
    # Check default locations when no path is given by user.
    if path is None:
      for rom_name in COMMON_ROM_NAMES:
        rom_path = os.path.join(DEFAULT_ROM_DIRECTORY, rom_name)
        if os.path.isfile(rom_path):
          return rom_path
      raise Exception(f"Could not find any ROMs ({', '.join(COMMON_ROM_NAMES)}) at the default location {DEFAULT_ROM_DIRECTORY}.") 

    rom_path = os.path.join(DEFAULT_ROM_DIRECTORY, path)
    if os.path.isfile(rom_path):
      return rom_path
    raise Exception(f"No ROM found at provided path {rom_path}.")



def split_list(l, n):
  return [l[i:i+n] for i in range(0, len(l), n)]

def to_hex(v):
  return '%#x' % v if v < -9 or v >9 else '%d'%v


def print_int_array(name, r, tname, decimal, split_length = 16, file = sys.stdout):
  rlen = len(r)
  rr = split_list(r, split_length )
  if decimal != None:
    if decimal:
      rr = [['%d' % s for s in t] for t in rr]
    else:
      rr = [[to_hex(s) for s in t] for t in rr]

  def pad_all_columns(rrs):
    colsiz = [max((0 if j >= len(r) else len(r[j])) for r in rrs) for j in range(len(rrs[0]))]
    def pad(c, i):
      return (' ' * (i - len(c))) + c
    return [[pad(c, colsiz[i]) for (i, c) in enumerate(r)] for r in rrs]

  if len(rr) == 1:
    print('static const %s %s[%d] = {%s};' % (tname, name, rlen, ", ".join(rr[0])), file = file)
  else:
    print('static const %s %s[%d] = {' % (tname, name, rlen), file = file)
    for t in pad_all_columns(rr):
      print("  " + "".join([(a + ', ') for a in t]), file = file)
    print('};', file = file)



class Reader:
  def __init__(self, ea, rb):
    self.ea, self.rb = ea, rb
  def next(self):
    r = self.rb(self.ea)
    self.ea += 1
    if (self.ea & 0xffff) == 0:
      self.ea += 0x8000
    return r

def decomp(ea, rb, offset_is_be = True, return_length = False):
  result = bytearray()
  reader = Reader(ea, rb)
  while True:
    b = reader.next()
    if b == 0xff:
      if return_length:
        return result, (reader.ea - ea) & 0x7fff
      else:
        return result
    if (b & 0xe0) != 0xe0:
      lx = b & 0x1f
      cmd = b & 0xe0
    else:
      cmd = (b << 3) & 0xe0
      lx = ((b & 3) << 8) | reader.next()
    lx += 1
    if cmd == 0x00: # 000 - literal
#      print('literal %d' % lx)
      while lx:
        result.append(reader.next())
        lx -= 1
    elif cmd & 0x80: # 1xx - copy
#      print('copy %d' % lx)
      offs = reader.next() << 8
      offs |= reader.next()
      if not offset_is_be: offs = ((offs >> 8) | (offs << 8)) & 0xffff
      while lx:
        result.append(result[offs])
        offs += 1
        lx -= 1
    elif (cmd & 0x40) == 0: # 00x - memset
#      print('memset %d' % lx)
      b = reader.next()
      while lx:
        result.append(b)
        lx -= 1
    elif (cmd & 0x20) == 0: # 010 - memset16
#      print('memsetw %d' % lx)
      b1, b2 = reader.next(), reader.next()
      while lx:
        result.append(b1)
        if lx==1: break
        result.append(b2)
        lx -= 2
    else: # 011 - incr
#      print('incr %d' % lx)
      b = reader.next()
      while lx:
        result.append(b)
        b = (b + 1) & 0xff
        lx -= 1


def decode_brr(get_byte, olds = (0, 0)):
  ea=0
  r = []
  old , older = olds
  while True:
    cmd = get_byte(ea)
    
    shift = cmd >> 4
    filter = (cmd >> 2) & 3
    #print("shift=%d, filter=%d" % (shift, filter))
    for i in range(16):
      t = (get_byte(ea+1+i//2) >> (0 if i & 1 else 4)) & 0xf
      s = (t & 7) - (t & 8)
      if shift <= 12:
        s = ((s << shift) >> 1)
      else:
        s = (s >> 3) << 12 # -2048 or 0

      if filter == 1:
        s += old*1+((-old*1) >> 4)
      elif filter == 2:
        s += old*2 + ((-old*3) >> 5)  - older + ((older*1) >> 4)
      elif filter == 3:
        s += old*2 + ((-old*13) >> 6) - older + ((older*3) >> 4)
      # saturate to 16 bits
      if s < -0x8000: s = -0x8000
      elif s >= 0x7fff: s = 0x7fff
      # wrap to 15 bits
      s = (s & 0x3fff) - (s & 0x4000)

      older, old = old, s
      #print('%d: 0x%x -> %d (shift %d, filter %d)' % (i, t, s*2, shift, filter))
      r.append(s*2)
    ea += 9
    if cmd & 1:
      break
  return array.array('h', r)

kBrrFilters = [
  lambda old, older: 0,
  lambda old, older: old*1+((-old*1) >> 4),
  lambda old, older: old*2 + ((-old*3) >> 5)  - older + ((older*1) >> 4),
  lambda old, older: old*2 + ((-old*13) >> 6) - older + ((older*3) >> 4)
]

def brr_get_one(old, rs, r):
  s = (rs << r) >> 1 if r <= 12 else (rs >> 3) << 12
  s += old
  s = -0x8000 if s < -0x8000 else 0x7fff if s > 0x7fff else s
  return (s & 0x3fff) - (s & 0x4000) # wrap to 15 bits

def encode_brr_generic(data, brr_repeat, olds = (0, 0), lossless=True):  
  assert len(data) % 16 == 0
  loop_enabled, loop_offset = 1 if brr_repeat != 0 else 0, 0
  result = []
  blk_data = [0] * 16
  best_data = [0] * 9
  p = 0
  best_old, best_older = olds
  while p < len(data):
#    print(p)
    best_err = 1 << 60
    startold, startolder = best_old, best_older

    if all(data[p + i] == 0 for i in range(16)):
      result.extend((loop_enabled * 2, 0, 0, 0, 0, 0, 0, 0, 0))
      p += 16
      continue
    for filter in range(4):
      if filter != 0 and (p == 0 or p == loop_offset):
        continue
      for r in range(12, 0, -1):
        blk_err = 0
        old, older = startold, startolder
        for i in range(16):
          s = kBrrFilters[filter](old, older)
          xs = data[p + i] >> 1 
          best_e = 1<<60
          for j in (0, 1, -1, 2, -2, 3, -3, 4, -4, 5, -5, 6, -6, 7, -7, -8):
            s0 = brr_get_one(s, j, r)
            e = (xs - s0) * (xs - s0)
            if e < best_e:
              best_e, best_j, best_s0 = e, j, s0
              if e == 0:
                #print(j)
                break
          if best_e != 0 and lossless:
            break
          blk_err += best_e
          blk_data[i] = best_j & 0xf
          older, old = old, best_s0
        else:
          if blk_err < best_err:
            best_err = blk_err
            best_old, best_older = old, older
            best_data[0] = r << 4 | filter << 2 | loop_enabled << 1
            for i in range(8):
              best_data[i + 1] = blk_data[i * 2] << 4 | blk_data[i * 2 + 1]
    result.extend(best_data)
    if lossless: assert best_err==0
    p += 16
#  result[-9] |= 1
  return result

