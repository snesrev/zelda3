import struct
import sys

data = open(sys.argv[1], 'rb').read()

ver, total_frames, log_size, last_inputs = struct.unpack_from('IIII', data)

log = data[32:32+log_size]

KEYS = "AXsSUDLRBY"

keys_memo = {}
def get_keys(i):
  if i not in keys_memo:
    keys_memo[i] = "".join(k for j, k in enumerate(KEYS) if i & (1 << j))
  return keys_memo[i]

frame_ctr = 0
bb = iter(log)

try:
  while True:
    cmd = bb.__next__()
    mask = 0xf if cmd < 0xc0 else 1
    frames = cmd & mask
    if frames == mask:
      while True:
        t = bb.__next__()
        frames += t
        if t != 255:
          break
    frame_ctr += frames
    if cmd < 0xc0:
      last_inputs ^= 1 << (cmd >> 4)
      print('%d: %s' % (frame_ctr, get_keys(last_inputs)))
    elif cmd < 0xd0:
      nb = 1 + ((cmd >> 2) & 3)
      addr = ((cmd >> 1) & 1) << 16
      addr |= bb.__next__() << 8
      addr |= bb.__next__()
      for i in range(nb):
        bb.__next__()
      print('%d: patchbytes(0x%x)' % (frame_ctr, addr))
    else:
      assert 0
except StopIteration:
  pass
  