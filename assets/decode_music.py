import hashlib
import array
import heapq, sys
import yaml
import time
import util, brr_tools


def load_sound_bank(rom, ea, mem_in = None):
  memory = list(mem_in) if mem_in else [None]*65536
  j =0 
  while True:
    numbytes = rom.get_word(ea)
    target = rom.get_word(ea+2)
    if numbytes==0:
#      print('Entry point = 0x%x' % target)
      return memory, target
    print('# Copy %d bytes to 0x%x' % (numbytes, target))
    ea += 4
    for i in range(numbytes):
      memory[target+i] = rom.get_byte(ea)
      ea += 1
      if (ea & 0xffff) < 0x8000:
        ea += 0x8000
    j += 1
    if j > 256:
      break

def get_byte(ea):
  return memory[ea]

def get_word(ea):
  return get_byte(ea) | get_byte(ea + 1) * 256


# lightworld
# Copy 11694 bytes to 0xd000
# Copy 1672 bytes to 0x2b00


# indoor
# Copy 11455 bytes to 0xd000
# Copy 1292 bytes to 0x2b00

def to_str(s):
  if isinstance(s, str):
    return s
  if isinstance(s, int):
    return str(s)
  return s.name


class Song:
  name = 'Song'
  def __str__(self):
    s = '# Song index %d\n' % self.index
    s += '[Song_0x%x]\n' % (self.ea)
    s += "".join(x.name + '\n' for x in self.phrases)
    return s

class SongList:
  name = 'SongList'
  def __str__(self):
    s = '[SongList_0x%x]\n' % (self.ea)
    s += "".join(('None' if x == None else x.name) + '\n' for x in self.songs)
    return s

class Phrase:
  name = 'Phrase'
  def __str__(self):
    s = '[Phrase_0x%x]\n' % (self.ea)
    s += "".join(('None' if x == None else x.name) + '\n' for x in self.patterns)
    return s

class Pattern:
  name = 'Pattern'
  def __str__(self):
    r = '[Pattern_0x%x]\n' % (self.ea)
    last_len = None
    for a in self.lines:
      s = ''
      if len(a) == 4:
        s += a[0] + " " + " ".join(map(to_str, a[1]))
      else:
        s += '%s' % (a[0])

        if a[-2] != None:
          s += ' %2d' % a[-2]
          last_len = a[-2]
        else:
          s += ' --'# % last_len

        if a[-1] != None:
          s += ' %2x' % a[-1]
        else:
          s += ' --'

      r += s + '\n'
    return r
  
class PhraseLoop:
  name = 'PhraseLoop'
  def __init__(self, loops, jmp):
    self.loops = loops
    self.jmp = jmp
    self.name = 'PhraseLoop %d %d' % (self.loops, self.jmp)
  def __str__(self):
    return self.name

types_for_ea = {}
pqueue_by_ea = []

def reset_queues():
  global types_for_ea, pqueue_by_ea
  types_for_ea = {}
  pqueue_by_ea = []

def get_type_for_ea(ea, tp):
  if ea == 0:
    return None
  assert(ea >= 256), ea
  a = types_for_ea.get(ea)
  if a != None:
    assert type(a)==tp, (type(a), tp, '0x%x' % ea)
    return a
  a = tp()
  a.ea = ea
  a.name = '%s_0x%x' % (a.name, ea)
  types_for_ea[ea] = a
  if get_byte(ea) != None:
    heapq.heappush(pqueue_by_ea, (ea, a))
    a.is_imported = False
  else:
    a.is_imported = True
  return a

kEffectByteLength = [1, 1, 2, 3, 0, 1, 2, 1, 2, 1, 1, 3, 0, 1, 2, 3, 1, 3, 3, 0, 1, 3, 0, 3, 3, 3, 1]
kEffectNames = ['Instrument', 'Pan', 'PanFade', 'Vibrato', 'VibratoOff',
                'SongVolume', 'SongVolumeFade', 'Tempo', 'TempoFade',
                'Transpose', 'ChannelTranpose', 'Tremolo', 'TremoloOff',
                'Volume', 'VolumeFade', 'Call', 'VibratoFade',
                'PitchEnvelopeTo', 'PitchEnvelopeFrom', 'PitchEnvelopeOff',
                'FineTune', 'EchoEnable', 'EchoOff', 'EchoSetup', 'EchoVolumeFade',
                'PitchSlide', 'PercussionDefine']
assert(len(kEffectNames) == 27)

def note_to_str(note):
  kKeys  = ['C-', 'C#', 'D-', 'D#', 'E-', 'F-', 'F#', 'G-', 'G#', 'A-', 'A#', 'B-']
  if note >= 72:
    if note == 72:
      return '-+-' # don't write kof
    elif note == 73:
      return '---' # want kof
    else:
      assert 0
  octave = note / 12
  key = note % 12
  return '%s%d' % (kKeys[key], octave + 1)

def get_pattern(ea):
  if ea == 0:
    return None
  pattern = get_type_for_ea(ea, Pattern)
  return pattern

def get_song(ea, index):
  song = get_type_for_ea(ea, Song)
  if song:
    song.index = index
  return song

def get_phrase(ea):
  phrase = get_type_for_ea(ea, Phrase)
  return phrase

def decode_pattern(pattern, next_ea):
  ea = pattern.ea
  pattern.lines = []
  start_ea = ea
  while True:
#    print('0x%x 0x%x' % (ea, start_ea))
#    assert ea != 0x28f0
    if ea != start_ea and ea == next_ea:
      pattern.lines.append(('Fallthrough', (), None, None))
      return
    note_length, volstuff = None, None
    cmd = get_byte(ea); ea += 1
    if cmd == 0:
      break
    if not (cmd & 0x80):
      note_length = cmd
      cmd = get_byte(ea); ea += 1
      if not (cmd & 0x80):
        volstuff = cmd
        cmd = get_byte(ea); ea += 1
    if cmd == 0xef:
      addr = get_word(ea)
      loops = get_byte(ea + 2)
      ea += 3
      pattern.lines.append((kEffectNames[cmd-0xe0], (get_pattern(addr), loops), note_length, volstuff))
    elif cmd >= 0xe0:
      assert note_length == None and volstuff == None, (note_length, volstuff)
      x = kEffectByteLength[cmd - 0xe0]
      args = [get_byte(ea+i) for i in range(x)]
      ea += x
      pattern.lines.append((kEffectNames[cmd-0xe0], args, note_length, volstuff))
    else:
      assert(cmd & 0x80)
      pattern.lines.append((note_to_str(cmd & 0x7f), note_length, volstuff))

  return pattern

def decode_phrase(phrase):
  phrase.patterns = [get_pattern(get_word(phrase.ea + i * 2)) for i in range(8)]

def decode_song(song):
  ea = song.ea
  song.phrases = []
  ea_org = ea
  eas_in_phrase = []
  while True:
    eas_in_phrase.append(ea)
    phrase = get_word(ea)
    if phrase == 0:
      break
    if phrase < 0x100:
      assert phrase != 0x80 and phrase != 0x81
      tgt = get_word(ea + 2)
      assert tgt in eas_in_phrase
      song.phrases.append(PhraseLoop(phrase, (tgt - ea) // 2))
      ea += 4                                     
    else:
      song.phrases.append(get_phrase(phrase))
      ea += 2
  return song

def decode_any(what, next_ea):
  if isinstance(what, Song):
    decode_song(what)
  elif isinstance(what, SongList):
    pass # no need
  elif isinstance(what, Phrase):
    decode_phrase(what)
  elif isinstance(what, Pattern):
    decode_pattern(what, next_ea)
  else:
    assert 0

def get_song_list(ea, num):
  song_list = get_type_for_ea(ea, SongList)
  song_list.songs  = [get_song(get_word(ea + i * 2), i) for i in range(num)]    

def load_song(ROM, song):
  global memory, SONGS_IN_BANK
  reset_queues()
  if song == 'intro':
    memory, entry_point = load_sound_bank(ROM, 0x998000) # intro
    SONGS_IN_BANK = (get_word(0xd000) - 0xd000) // 2
  elif song == 'lightworld':
    memory, entry_point = load_sound_bank(ROM, 0x9a9ef5) # lw
    SONGS_IN_BANK = (get_word(0xd000) - 0xd000) // 2
  elif song == 'indoor':
    memory, entry_point = load_sound_bank(ROM, 0x9b8000) # indoor
    SONGS_IN_BANK = (0xd046 - 0xd000) // 2
  elif song == 'ending':
    memory, entry_point = load_sound_bank(ROM, 0x9ad380) # ending
    SONGS_IN_BANK = (0xd046 - 0xd000) // 2


def print_song(song, f):
  get_song_list(0xd000, SONGS_IN_BANK)
  if song in ('intro', 'lightworld'):
    get_phrase(0xD878)
    get_phrase(0xD8A8)
    get_phrase(0xD8B8)
    get_phrase(0xDf11)
    get_phrase(0xe37c)
  if song == 'indoor':
    get_phrase(0xDc5e)
    get_phrase(0xDc6e)
    get_pattern(0xe905)
    get_phrase(0xe94a)
  if song == 'ending':
    get_phrase(0x2a10)    
  while len(pqueue_by_ea):
    _, item = heapq.heappop(pqueue_by_ea)
    decode_any(item, pqueue_by_ea[0][0] if len(pqueue_by_ea) else None)
  for a, b in sorted(types_for_ea.items()):
    if not b.is_imported:
      print(b, file = f)


def dump_brr_audio():
  def decode_brr(snd):
    start, loop_start = get_word(0x3c00 + snd * 4), get_word(0x3c00 + snd * 4 + 2)
    r = brr_tools.decode_brr(lambda x: get_byte(start+x))
    return r, [get_byte(start+x) for x in range(len(r)//16 * 9)], get_byte(start)&0x2 != 0
  for audio_idx in range(25):
    sound_data, brr_data, brr_repeat = decode_brr(audio_idx)
    open('sound/sound%d.pcm.brr' % audio_idx, 'wb').write(bytes(brr_data))
    open('sound/sound%d.pcm' % audio_idx, 'wb').write(sound_data)

def dump_music_info():
  music_info = {}
  kDupSamples = {10 : 9, 20 : 19}
  music_info['samples'] = []
  for audio_idx in range(25):
    start, rep = get_word(0x3c00 + audio_idx * 4), get_word(0x3c00 + audio_idx * 4 + 2)
    sample_info = {
      'file' : 'sound/sound%d.pcm' % kDupSamples.get(audio_idx, audio_idx)
    }
    if get_byte(start) & 2:
      sample_info['repeat'] = (rep - start) // 9 * 16
    music_info['samples'].append(sample_info)

  def add_sustain_decay_etc(ea, info):
    adsr1, adsr2, gain = get_byte(ea), get_byte(ea + 1), get_byte(ea + 2)
    info['decay'] = (adsr1 >> 4) & 7
    info['attack'] = adsr1 & 0xf
    info['sustain_level'] = adsr2 >> 5
    info['sustain_rate'] = adsr2 & 0x1f
    info['vxgain'] = gain  

  music_info['instruments'] = []
  for i in range(25):
    ea = 0x3d00 + i * 6
    adsr1, adsr2 = get_byte(ea + 1), get_byte(ea + 2)
    info = {
      'sample' : get_byte(ea),
    }
    add_sustain_decay_etc(ea + 1, info)
    info['pitch_base'] = get_byte(ea + 4) << 8 | get_byte(ea + 5)
    music_info['instruments'].append(info)

  music_info['note_gate_off'] = [get_byte(i) for i in range(0x3D96, 0x3D96 + 8)]
  music_info['note_volume'] = [get_byte(i) for i in range(0x3D9E, 0x3D9E + 16)]

  music_info['sfx_instruments'] = []

  for i in range(25):
    ea = 0x3e00 + i * 9
    info = {
      'voll' : get_byte(ea),
      'volr' : get_byte(ea + 1),
      'pitch' : get_word(ea + 2),
      'sample' : get_byte(ea + 4)
    }
    add_sustain_decay_etc(ea + 5, info)
    info['pitch_base'] = get_byte(ea + 8)
    music_info['sfx_instruments'].append(info)  

  s = yaml.dump(music_info, default_flow_style=None, sort_keys=False)
  open('music_info.yaml', 'w').write(s)

def decode_sfx(ea, next_addr):
  r = []
  while True:
    if ea == next_addr:
      r.append(('Fallthrough', ))
      return r
    b = get_byte(ea); ea += 1
    if b == 0:
      return r
    note_length = None
    volume_left, volume_right = None, None
    if not (b & 0x80):
      note_length = b
      b = get_byte(ea); ea += 1
      if not (b & 0x80):
        volume_left, volume_right = b, None
        b = get_byte(ea); ea += 1
        if not b & 0x80:
          volume_right = b
          b = get_byte(ea); ea += 1
    if b == 0xe0:
      assert note_length == None and volume_left == None and volume_right == None, ea
      b = get_byte(ea); ea += 1
      r.append(('SetInstrument %d' % b, ))
    elif b == 0xf9:
      #assert note_length == None and volume_left == None and volume_right == None, ea
      b = get_byte(ea); ea += 1
      b0, b1, b2 = get_byte(ea), get_byte(ea+1), get_byte(ea+2); ea += 3
      r.append(('PitchSlide %d %d %d' % (b0, b1, b2), note_to_str(b & 0x7f), note_length, volume_left, volume_right))
    elif b == 0xf1:
      #assert note_length == None and volume_left == None and volume_right == None, ea
      b0, b1, b2 = get_byte(ea), get_byte(ea+1), get_byte(ea+2); ea += 3
      r.append(('PitchSlide %d %d %d' % (b0, b1, b2), None, note_length, volume_left, volume_right))
    elif b == 0xff:
      assert note_length == None and volume_left == None and volume_right == None, ea
      r.append(('Restart',))
      return r
    else:
      r.append((None, note_to_str(b & 0x7f), note_length, volume_left, volume_right))

def print_all_sfx(f):
  items = set()
  def add_sfx_top(base, num, name):
    print('[%s_0x%x]' % (name, base), file = f)
    next_ea = base + num * 2
    echo_ea = next_ea + num
    for i in range(num):
      r = []
      ea = get_word(base + i * 2)
      if ea == 0:
        t = 'None'
      else:
        items.add(ea)
        t = 'Sfx_0x%x' % ea
      if name == 'SfxPort1':
        print('%s,%d' % (t, get_byte(next_ea + i)), file = f)
      else:
        print('%s,%d,%d' % (t, get_byte(next_ea + i), get_byte(echo_ea + i)), file = f)
    print(file = f)
  add_sfx_top(0x17c0, 32, 'SfxPort1')
  add_sfx_top(0x1820, 63, 'SfxPort2')
  add_sfx_top(0x191c, 63, 'SfxPort3')
  items.add(0x1a5b)
  items.add(0x1d1c)
  items.add(0x1ee2)
  items.add(0x1f13)
  items.add(0x1f1c)
  items.add(0x252d)
  items.add(0x2533)
  items.add(0x26a2)
  items.add(0x277e)
  items.add(0x279d)
  items.add(0x27c9)
  items.add(0x27f6)
  items.add(0x2807)
  items.add(0x2818)
  items.add(0x2829)
  items.add(0x2831)
  items.add(0x284a)
  items = sorted(list(items))
  for i in range(len(items)):
    print('[Sfx_0x%x]' % items[i], file = f)
    next_addr = items[i + 1] if i + 1 < len(items) else 0
    rs = decode_sfx(items[i], next_addr)
    for r in rs:
      if len(r) == 5:
        aa = '.  ' if r[1] == None else r[1]
        bb = '--' if r[2] == None else '%2d' % r[2]
        cc = '---' if r[3] == None else '%3d' % r[3]
        dd = '---' if r[4] == None else '%3d' % r[4]
        r0 = '' if r[0] == None else ' ' + r[0]
        print('%s %s %s %s%s' % (aa, bb, cc, dd, r0), file = f)
      else:
        print(r[0], file = f)
    print(file = f)

def extract_sound_data(rom):
  load_song(rom, 'intro')
  print_song('intro', open('sound_intro.txt', 'w'))

  dump_brr_audio()
  dump_music_info()
  print_all_sfx(open('sfx.txt', 'w'))

  load_song(rom, 'indoor')
  print_song('indoor', open('sound_indoor.txt', 'w'))

  load_song(rom, 'ending')
  print_song('ending', open('sound_ending.txt', 'w'))

if __name__ == "__main__":
  ROM = util.LoadedRom(sys.argv[1])
  song = sys.argv[2]

  load_song(ROM, song)
  print_song(song, sys.stdout)

