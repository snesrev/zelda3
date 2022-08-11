
kTextAlphabet = [
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
  "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",

  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
  "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",

  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",

  # codes 0x3E and up
  "!", "?", "-", ".", ",",

  # codes 0x43 and up
  "[...]", ">", "(", ")",

  # codes 0x47 and up
  "[Ankh]", "[Waves]", "[Snake]", "[LinkL]", "[LinkR]",
  "\"", "[Up]", "[Down]", "[Left]", "[Right]", "'",

  # codes 0x52 and up
  "[1HeartL]", "[1HeartR]", "[2HeartL]", "[3HeartL]",
  "[3HeartR]", "[4HeartL]", "[4HeartR]",

  " ", "<", "[A]", "[B]", "[X]", "[Y]",
]

kText_CommandLengths = [1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, ]
kText_CommandNames = [
  "NextPic",
  "Choose",
  "Item",
  "Name",
  "Window",
  "Number",
  "Position",
  "ScrollSpd",
  "Selchg",
  "Crash",
  "Choose3",
  "Choose2",
  "Scroll",
  "1",
  "2",
  "3",
  "Color",
  "Wait",
  "Sound",
  "Speed",
  "Mark",
  "Mark2",
  "Clear",
  "Waitkey",
  "EndMessage"
]

kTextDictionary = [  0x59, 0x59, 0x59, 0x59,
  0x59, 0x59, 0x59,
  0x59, 0x59,
  0x51, 0x2c, 0x59,
  0x1a, 0x27, 0x1d, 0x59,
  0x1a, 0x2b, 0x1e, 0x59,
  0x1a, 0x25, 0x25, 0x59,
  0x1a, 0x22, 0x27,
  0x1a, 0x27, 0x1d,
  0x1a, 0x2d, 0x59,
  0x1a, 0x2c, 0x2d,
  0x1a, 0x27,
  0x1a, 0x2d,
  0x1b, 0x25, 0x1e,
  0x1b, 0x1a,
  0x1b, 0x1e,
  0x1b, 0x28,
  0x1c, 0x1a, 0x27, 0x59,
  0x1c, 0x21, 0x1e,
  0x1c, 0x28, 0x26,
  0x1c, 0x24,
  0x1d, 0x1e, 0x2c,
  0x1d, 0x22,
  0x1d, 0x28,
  0x1e, 0x27, 0x59,
  0x1e, 0x2b, 0x59,
  0x1e, 0x1a, 0x2b,
  0x1e, 0x27, 0x2d,
  0x1e, 0x1d, 0x59,
  0x1e, 0x27,
  0x1e, 0x2b,
  0x1e, 0x2f,
  0x1f, 0x28, 0x2b,
  0x1f, 0x2b, 0x28,
  0x20, 0x22, 0x2f, 0x1e, 0x59,
  0x20, 0x1e, 0x2d,
  0x20, 0x28,
  0x21, 0x1a, 0x2f, 0x1e,
  0x21, 0x1a, 0x2c,
  0x21, 0x1e, 0x2b,
  0x21, 0x22,
  0x21, 0x1a,
  0x22, 0x20, 0x21, 0x2d, 0x59,
  0x22, 0x27, 0x20, 0x59,
  0x22, 0x27,
  0x22, 0x2c,
  0x22, 0x2d,
  0x23, 0x2e, 0x2c, 0x2d,
  0x24, 0x27, 0x28, 0x30,
  0x25, 0x32, 0x59,
  0x25, 0x1a,
  0x25, 0x28,
  0x26, 0x1a, 0x27,
  0x26, 0x1a,
  0x26, 0x1e,
  0x26, 0x2e,
  0x27, 0x51, 0x2d, 0x59,
  0x27, 0x28, 0x27,
  0x27, 0x28, 0x2d,
  0x28, 0x29, 0x1e, 0x27,
  0x28, 0x2e, 0x27, 0x1d,
  0x28, 0x2e, 0x2d, 0x59,
  0x28, 0x1f,
  0x28, 0x27,
  0x28, 0x2b,
  0x29, 0x1e, 0x2b,
  0x29, 0x25, 0x1e,
  0x29, 0x28, 0x30,
  0x29, 0x2b, 0x28,
  0x2b, 0x1e, 0x59,
  0x2b, 0x1e,
  0x2c, 0x28, 0x26, 0x1e,
  0x2c, 0x1e,
  0x2c, 0x21,
  0x2c, 0x28,
  0x2c, 0x2d,
  0x2d, 0x1e, 0x2b, 0x59,
  0x2d, 0x21, 0x22, 0x27,
  0x2d, 0x1e, 0x2b,
  0x2d, 0x21, 0x1a,
  0x2d, 0x21, 0x1e,
  0x2d, 0x21, 0x22,
  0x2d, 0x28,
  0x2d, 0x2b,
  0x2e, 0x29,
  0x2f, 0x1e, 0x2b,
  0x30, 0x22, 0x2d, 0x21,
  0x30, 0x1a,
  0x30, 0x1e,
  0x30, 0x21,
  0x30, 0x22,
  0x32, 0x28, 0x2e,
  0x7, 0x1e, 0x2b,
  0x13, 0x21, 0x1a,
  0x13, 0x21, 0x1e,
  0x13, 0x21, 0x22,
  0x18, 0x28, 0x2e,
]

kTextDictionary_Idx = [
  0, 4, 7, 9, 12, 16, 20, 24, 27, 30, 33, 36, 38, 40, 43, 45, 47, 49, 53, 56, 59, 61, 64, 66, 68, 71, 74, 77, 80, 83, 85, 87, 89, 92, 95, 100, 103, 105, 109, 112, 115, 117, 119, 124, 128, 130, 132, 134, 138, 142, 145, 147, 149, 152, 154, 156, 158, 162, 165, 168, 172, 176, 180, 182, 184, 186, 189, 192, 195, 198, 201, 203, 207, 209, 211, 213, 215, 219, 223, 226, 229, 232, 235, 237, 239, 241, 244, 248, 250, 252, 254, 256, 259, 262, 265, 268, 271, 274
]

def make_dict():
  r, rinv = {}, {}
  for i in range(len(kTextDictionary_Idx) - 1):
    ln = kTextDictionary_Idx[i + 1] - kTextDictionary_Idx[i]
    idx = kTextDictionary_Idx[i]
    s = "".join(kTextAlphabet[kTextDictionary[idx+i]] for i in range(ln))
    r[i] = s
    rinv[s] = i
  return r, rinv

kTextDictionary_Ascii, kTextDictionary_AsciiBack = make_dict()

def decode_strings(get_byte):
  p = 0x9c8000
  result = []
  while True:
    org_p = p
    #print('0x%x' % p)
    s = ''
    srcdata = []
    while True:
      c = get_byte(p)
      srcdata.append(c)
      l = kText_CommandLengths[c - 0x67] if c >= 0x67 and c < 0x80 else 1
      p += l
      if c == 0x7f:
        break
      if c < 0x67:
        s += kTextAlphabet[c]
      elif c < 0x80:
        if l == 2:
          srcdata.append(get_byte(p-1))
          s += '[%s %.2d]' % (kText_CommandNames[c - 0x67], get_byte(p-1))
        else:
          s += '[%s]' % kText_CommandNames[c - 0x67]
      elif c == 0x80:
        p = 0x8edf40
        s = None
        break
      elif c > 0x80 and c < 0x88:
        assert 0
      elif c == 0xff:
        return result
      else:
        s += kTextDictionary_Ascii[c - 0x88]
    if s != None:
      result.append((s, srcdata))
    
def print_strings(f, get_byte):
  for i, s in enumerate(decode_strings(get_byte)):
    print('%s: %s' % (i + 1, s[0]), file = f)

def find_string_char_at(s, i):
  a = s[i:]

  for k, v in kTextDictionary_AsciiBack.items():
    if a.startswith(k):
      return [v + 0x88], len(k)

  for i, s in enumerate(kTextAlphabet):
    if a.startswith(s):
      return [i], len(s)

  if a.startswith('['):
    cmd = a[1:a.index(']')]
    if cmd in kText_CommandNames:
      i = kText_CommandNames.index(cmd)
      return [i + 0x67], len(cmd) + 2
    
    for i, s in enumerate(kText_CommandNames):
      if kText_CommandLengths[i] == 2 and cmd.startswith(s):
        e = cmd[len(s):].strip()
        return [i + 0x67, int(e)], len(cmd) + 2
    
  print('substr %s not found' % a)
  assert 0

def compress_string(s):
  # find the greedy best match
  i = 0
  r = []
  while i < len(s):
    what, num = find_string_char_at(s, i)
    r.extend(what)
    i += num
  r.append(0x7f)
  return r

def verify(get_byte):
  for i, (decoded, original) in enumerate(decode_strings(get_byte)):
    c = compress_string(decoded)
    if c != original:
      print('String %s not match: %s, %s' % (decoded, c, original))
      break
    else:
      pass
