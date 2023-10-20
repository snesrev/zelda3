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
DEFAULT_ROM_DIRECTORY = os.path.join(os.path.dirname(__file__), '..')

ZELDA3_SHA1_US = '6D4F10A8B10E10DBE624CB23CF03B88BB8252973'
ZELDA3_SHA1 = {
   ZELDA3_SHA1_US : ('us', 'Legend of Zelda, The - A Link to the Past (USA)'),
  '2E62494967FB0AFDF5DA1635607F9641DF7C6559' : ('de', 'Legend of Zelda, The - A Link to the Past (Germany)'),
  '229364A1B92A05167CD38609B1AA98F7041987CC' : ('fr', 'Legend of Zelda, The - A Link to the Past (France)'),
  'C1C6C7F76FFF936C534FF11F87A54162FC0AA100' : ('fr-c', 'Legend of Zelda, The - A Link to the Past (Canada)'),
  '7C073A222569B9B8E8CA5FCB5DFEC3B5E31DA895' : ('en',  'Legend of Zelda, The - A Link to the Past (Europe)'),
  '461FCBD700D1332009C0E85A7A136E2A8E4B111E' : ('es',  'Spanish - https://www.romhacking.net/translations/2195/'),
  '3C4D605EEFDA1D76F101965138F238476655B11D' : ('pl',  'Polish - https://www.romhacking.net/translations/5760/'),
  'D0D09ED41F9C373FE6AFDCCAFBF0DA8C88D3D90D' : ('pt',  'Portuguese - https://www.romhacking.net/translations/6530/'),
  'B2A07A59E64C498BC1B2F28728F9BF4014C8D582' : ('redux', 'English Redux - https://www.romhacking.net/translations/6657/'),
  '9325C22EB0A2A1F0017157C8B620BC3A605CEDE1' : ('redux', 'English Redux - https://www.romhacking.net/hacks/2594/'),
  'FA8ADFDBA2697C9A54D583A1284A22AC764C7637' : ('nl', 'Dutch - https://www.romhacking.net/translations/1124/'),
  '43CD3438469B2C3FE879EA2F410B3EF3CB3F1CA4' : ('sv', 'Swedish - https://www.romhacking.net/translations/982/'),
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

    # Remove the SMC header?
    if (len(self.ROM) & 0xfffff) == 0x200:
      self.ROM = self.ROM[0x200:]

    hash = hashlib.sha1(self.ROM).hexdigest().upper()
    entry = ZELDA3_SHA1.get(hash)
    self.language = entry[0] if entry != None else None

    # Workaround for swedish rom with broken size
    if self.language == 'sv' and len(self.ROM) == 0x10083b:
      self.ROM = self.ROM[0x200:]

    if support_multilanguage:
      if self.language == None:
        msg = f"\n\nROM with hash {hash} not supported.\n\nYou need one of the following ROMs to extract the resources:\n"
        for k, v in ZELDA3_SHA1.items():
          msg += '%5s: %s: %s\n' % (v[0], k, v[1])
        raise Exception(msg)
      print('Identified ROM as: %s - "%s"' % entry)
    else:
      if self.language != 'us':
        raise Exception(f"\n\nROM with hash {hash} not supported.\n\nExpected {ZELDA3_SHA1_US}.\nPlease verify your ROM is \"Legend of Zelda, The - A Link to the Past (USA)\"");

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

