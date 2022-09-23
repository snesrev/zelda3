import array
import sys
import yaml
import re
import util

class Song:
  name = 'Song'
  def __str__(self):
    s = '[Song_0x%x idx=%d]\n' % (self.ea, self.index)
    s += "".join(x.name + '\n' for x in self.phrases)
    return s

class Phrase:
  name = 'Phrase'
  def __str__(self):
    s = '[%s]' % (self.name)
    return s

class PhraseLoop:
  name = 'PhraseLoop'
  def __init__(self, loops, jmp):
    self.loops = loops
    self.jmp = jmp
    self.name = 'PhraseLoop %d %d' % (self.loops, self.jmp)
  def __str__(self):
    return self.name

class Pattern:
  name = 'Pattern'

class SfxPattern:
  name = 'SfxPattern'

class SongList:
  name = 'SongList'

class SfxList:
  name = 'SfxList'

types_for_name = {}

def get_type_for_name(nam, tp, is_create):
  if nam == 'None':
    assert not is_create
    return None
  a = types_for_name.get(nam)
  if a != None:
    if is_create:
      if a.defined:
        raise Exception('%s already defined' % nam)
      a.defined = True
    assert type(a) == tp, (nam, type(a), tp)
    return a
  a = tp()
  a.name = nam
  a.defined = is_create
  types_for_name[nam] = a
  a.ea = None
  if '_0x' in nam:
    a.ea = int(nam[nam.index('_0x') + 3:], 16)
  return a

def process_song(caption, args, lines):
  song = get_type_for_name(caption, Song, True)
  song.phrases = []
  for line in lines:
    line_cmd, *line_args = line.split(' ')
    if line_cmd == 'PhraseLoop':
      song.phrases.append(PhraseLoop(int(line_args[0]), int(line_args[1])))
    else:
      song.phrases.append(get_type_for_name(line_cmd, Phrase, False))
  return song

def process_song_list(caption, args, lines):
  song_list = get_type_for_name(caption, SongList, True)
  song_list.songs = [get_type_for_name(line, Song, False) for line in lines]
  return song_list

def process_phrase(caption, args, lines):
  phrase = get_type_for_name(caption, Phrase, True)
  assert len(lines)==8
  phrase.patterns = [get_type_for_name(lines[i], Pattern, False) for i in range(8)]
  return phrase
  
kEffectByteLength = [1, 1, 2, 3, 0, 1, 2, 1, 2, 1, 1, 3, 0, 1, 2, 3, 1, 3, 3, 0, 1, 3, 0, 3, 3, 3, 1]
kEffectNames = ['Instrument', 'Pan', 'PanFade', 'Vibrato', 'VibratoOff',
                'SongVolume', 'SongVolumeFade', 'Tempo', 'TempoFade',
                'Transpose', 'ChannelTranpose', 'Tremolo', 'TremoloOff',
                'Volume', 'VolumeFade', 'Call', 'VibratoFade',
                'PitchEnvelopeTo', 'PitchEnvelopeFrom', 'PitchEnvelopeOff',
                'FineTune', 'EchoEnable', 'EchoOff', 'EchoSetup', 'EchoVolumeFade',
                'PitchSlide', 'PercussionDefine']
kEffectNamesDict = {a:i for i, a in enumerate(kEffectNames)}
kKeys  = ['C-', 'C#', 'D-', 'D#', 'E-', 'F-', 'F#', 'G-', 'G#', 'A-', 'A#', 'B-']
kKeysDict = {a+str(j+1):i+j*12 for i, a in enumerate(kKeys) for j in range(6)}
kKeysDict['-+-'] = 72
kKeysDict['---'] = 73

def process_pattern(caption, args, lines):
  pattern = get_type_for_name(caption, Pattern, True)
  pattern.lines = []
  pattern.fallthrough = False
  for line in lines:
    assert not pattern.fallthrough
    line_cmd, *line_args = line.split()
    if line_cmd == 'Call':
      assert len(line_args) == 2
      pattern.lines.append(('Call', (get_type_for_name(line_args[0], Pattern, False),int(line_args[1]) ), None, None))
    elif line_cmd == 'Fallthrough':
      pattern.fallthrough = True
    elif line_cmd in kEffectNamesDict:
      line_args = [int(a) for a in line_args]
      pattern.lines.append((line_cmd, line_args, None, None))
    elif line_cmd in kKeysDict:
      assert len(line_args) == 2, line_args
      note_length = None if line_args[0] == '--' else int(line_args[0])
      volstuff = None if line_args[1] == '--' else int(line_args[1], 16)
      pattern.lines.append((line_cmd, line_args, note_length, volstuff))
    else:
      assert 0, repr(line_cmd)
  return pattern

def process_sfx_pattern(caption, args, lines):
  pattern = get_type_for_name(caption, SfxPattern, True)
  pattern.lines = lines
  return pattern

def write_sfx_pattern(serializer, o):
#  print('# Creating %s (%x)' % ( o.name, serializer.addr))
  for i,line in enumerate(o.lines):
#    print(line)
    line_cmd, *line_args = re.split(r' +', line)
    if line_cmd == 'SetInstrument':
      assert len(line_args) == 1
      serializer.write([0xe0, int(line_args[0])])
    elif line_cmd == 'Restart':
      serializer.write([0xff])
      assert i == len(o.lines) - 1
      return
    elif line_cmd == 'Fallthrough':
      assert i == len(o.lines) - 1
      return
    elif line_cmd in kKeysDict or line_cmd == '.':
      if line_args[0] != '--':
        serializer.write((int(line_args[0]),))
        if line_args[1] != '---':
          serializer.write((int(line_args[1]),))
          if line_args[2] != '---':
            serializer.write((int(line_args[2]),))
      if len(line_args) >= 4:
        assert line_args[3] == 'PitchSlide'
        if line_cmd == '.':
          serializer.write([0xf1, int(line_args[4]), int(line_args[5]), int(line_args[6])])
        else:
          serializer.write([0xf9, kKeysDict[line_cmd] | 0x80, int(line_args[4]), int(line_args[5]), int(line_args[6])])
      else:
        serializer.write([kKeysDict[line_cmd] | 0x80])
    else:
      assert 0, repr(line_cmd)
  serializer.write([0])

def process_sfx_list(caption, args, lines):
  sfx_list = get_type_for_name(caption, SfxList, True)
  sfx_list.patterns = []
  sfx_list.next = []
  sfx_list.echo = []
  for line in lines:
    r = line.split(',')
    sfx_list.patterns.append(get_type_for_name(r[0], SfxPattern, False))
    sfx_list.next.append(int(r[1]))
    if len(r) >= 3:
      sfx_list.echo.append(int(r[2]))
  assert len(sfx_list.echo) in (0, len(lines))
  return sfx_list

def write_sfx_list(serializer, o):
  for line in o.patterns:
    serializer.write_reloc_entry(line)
  for next in o.next:
    serializer.write([next])
  for i in o.echo:
    serializer.write([i])

kGapStartAddrs = (0x2b00, 0x2880, 0xd000)

class Serializer:
  def __init__(self):
    self.memory = [None] * 65536
    self.relocs = []
    self.addr = None

  def write(self, data):
    for d in data:
      assert self.memory[self.addr] == None, '0x%x' % self.addr
      self.memory[self.addr] = d
      self.addr += 1

  def write_at(self, a, data):
    for d in data:
      self.memory[a] = d
      a += 1

  def write_word(self, a, v):
    self.memory[a] = v & 0xff
    self.memory[a + 1] = v >> 8 & 0xff
      

  def write_reloc_entry(self, r):
    self.write([0, 0])
    if r:
      self.relocs.append((self.addr - 2, r))
    

  def write_song(self, song):
    for phrase in song.phrases:
      if isinstance(phrase, PhraseLoop):
        i = self.addr + phrase.jmp * 2
        self.write([phrase.loops, 0])
        self.write([i & 0xff, i >> 8])
      else:
        self.write_reloc_entry(phrase)
    self.write([0, 0])

  def write_phrase(self, phrase):
    for i in range(8):
      self.write_reloc_entry(phrase.patterns[i])

  def write_song_list(self, song_list):
    for song in song_list.songs:
      self.write_reloc_entry(song)
      
  def write_pattern(self, patt):
    for cmd, args, note_length, volstuff in patt.lines:
      #print(cmd, args, note_length, volstuff)
      if note_length != None: self.write((note_length, ))
      if volstuff != None: self.write((volstuff, ))
      if cmd in kKeysDict:
        self.write((0x80 | kKeysDict[cmd], ))
      elif cmd == 'Call':
        self.write([0xef, 0, 0, int(args[1])])
        self.relocs.append((self.addr - 3, args[0]))
      elif cmd in kEffectNamesDict:
        i = kEffectNamesDict[cmd]
        assert len(args) == kEffectByteLength[i]
        self.write([0xe0 + i])
        self.write(args)
    if not patt.fallthrough:
      self.write((0, ))

  def write_obj(self, what):
 #   print(what.name, self.addr, what.ea)
 #   print('Writing %s at 0x%x. Curr pos 0x%x' % (what.name, what.ea, 0 if self.addr == None else self.addr))
    if what.ea != None:
      
      if self.addr == None or what.ea in kGapStartAddrs:# or True:#what.ea >= self.addr:
        self.addr = what.ea
      elif what.ea != self.addr:
        print('%s: 0x%x != 0x%x' % (what.name, what.ea, self.addr))
        assert(0)
   
    what.write_addr = self.addr
    assert self.addr != None
    if isinstance(what, Phrase):
      self.write_phrase(what)
    elif isinstance(what, Pattern):
      self.write_pattern(what)
    elif isinstance(what, Song):
      self.write_song(what)
    elif isinstance(what, SongList):
      self.write_song_list(what)
    elif isinstance(what, SfxPattern):
      write_sfx_pattern(self, what)
    elif isinstance(what, SfxList):
      write_sfx_list(self, what)
    else:
      print(type(what))
      assert(0)

  def write_zeros(self, a, b):
    while a < b:
      self.memory[a] = 0
      a += 1

  def process_relocs(self):
    for p, r in self.relocs:
      self.memory[p + 0] = r.write_addr & 0xff
      self.memory[p + 1] = r.write_addr >> 8

def process_file(file):
  sorted_ents = []
  def add_collect(heading, collect):
    caption, *args = heading.strip('[]').split(' ', 1)
    if caption.startswith('Song_'):
      r = process_song(caption, args, collect)
    elif caption.startswith('Phrase_'):
      r = process_phrase(caption, args, collect)
    elif caption.startswith('Pattern_'):
      r = process_pattern(caption, args, collect)
    elif caption.startswith('SongList_'):
      r = process_song_list(caption, args, collect)
    elif caption.startswith('Sfx_'):
      r = process_sfx_pattern(caption, args, collect)
    elif caption.startswith('SfxPort'):
      r = process_sfx_list(caption, args, collect)
    else:
      assert 0
    sorted_ents.append(r)
  collect = None
  heading = None
  for line in file:
    line = line.strip()
    if line == '' or line.startswith('#'): continue
    if line.startswith('['):
      if heading != None:
        add_collect(heading, collect)
      heading = line
      collect = []
      #
      #print(caption, args)
    else:
      collect.append(line.strip('\n'))
  if heading != None:
    add_collect(heading, collect)
  return sorted_ents

def serialize_song(serializer, song, sorted_ents):
  if song == 'intro':
#    serializer.write_zeros(0x2a8b, 0x2b00)
#    serializer.write_zeros(0x3188, 0x4000)
 #   serializer.write_zeros(0xbaa0, 0xd000)
 #   serializer.write_zeros(0xfdae, 0x10000)
 #   serializer.write_zeros(0x2850, 0x2880)
    serializer.addr = 0x4000
    sample_to_addr = {}
    music_info = yaml.safe_load(open('music_info.yaml', 'r'))
    for i, info in enumerate(music_info['samples']):
      if info['file'] not in sample_to_addr:
        sample_to_addr[info['file']] = serializer.addr
        serializer.write(open('%s.brr' % info['file'], 'rb').read())
      addr = sample_to_addr[info['file']]
      serializer.write_word(0x3c00 + i * 4, addr)
      serializer.write_word(0x3c00 + i * 4 + 2, addr + info['repeat'] // 16 * 9 if 'repeat' in info else serializer.addr)
    for i in range(6):
      serializer.write_word(0x3c64 + i * 2, 0xffff)
    
    for i, info in enumerate(music_info['instruments']):
      ea = 0x3d00 + i * 6
      serializer.memory[ea + 0] = info['sample']
      serializer.memory[ea + 1] = 0x80 | info['decay'] << 4 | info['attack']
      serializer.memory[ea + 2] = info['sustain_level'] << 5 | info['sustain_rate']
      serializer.memory[ea + 3] = info['vxgain']
      serializer.memory[ea + 4] = info['pitch_base'] >> 8
      serializer.memory[ea + 5] = info['pitch_base'] & 0xff

    serializer.write_at(0x3D96, music_info['note_gate_off'])
    serializer.write_at(0x3D9e, music_info['note_volume'])

    for i, info in enumerate(music_info['sfx_instruments']):
      ea = 0x3e00 + i * 9
      serializer.memory[ea + 0] = info['voll']
      serializer.memory[ea + 1] = info['volr']
      serializer.write_word(ea + 2, info['pitch'])
      serializer.memory[ea + 4] = info['sample']
      serializer.memory[ea + 5] = 0x80 | info['decay'] << 4 | info['attack']
      serializer.memory[ea + 6] = info['sustain_level'] << 5 | info['sustain_rate']
      serializer.memory[ea + 7] = info['vxgain']
      serializer.memory[ea + 8] = info['pitch_base']

  serializer.addr = None
  for e in sorted_ents:
    serializer.write_obj(e)

  if song == 'indoor':
    t = types_for_name['Song_0x2880']
    t.defined = True
    t.write_addr = 0x2880


  for e in types_for_name.values():
    if not e.defined:
      raise Exception('Symbol %s not defined' % e.name)

  serializer.process_relocs()

def compare_with_orig(serializer, song):
  ranges=[]
  ok = True
  spc = open('sound/%s.spc' % song, 'rb').read()
  for i in range(65536):
    if serializer.memory[i] != None:
      if serializer.memory[i]!= spc[i]:
        print('0x%x: 0x%x != 0x%x' % (i, serializer.memory[i], spc[i]))
        ok = False
    else:
#      serializer.memory[i] = spc[i]
      if len(ranges) and ranges[-1][1] == i:
        ranges[-1][1] = i + 1
      else:
        ranges.append([i, i + 1])
  if __name__ == "__main__":
    for a, b in ranges:
      print('// undefined %x-%x' % (a, b))
  return ok

def produce_loadable_seq(serializer):
  r = []
  # count non zeros
  start, i = 0, 0
  while start < 0x10000:    
    while i < 0x10000 and serializer.memory[i] != None:
      i += 1
    j = i
    while i < 0x10000 and serializer.memory[i] == None:
      i += 1
    
    if j == start:
      start = i
      continue
    r.extend([(j - start) & 0xff, (j - start) >> 8, start & 0xff, start >> 8])
    r.extend(serializer.memory[start:j])
#    print('copy 0x%x - 0x%x (%d)' % (start, j, j - start))
    start = i
  r.extend([0, 0])
  return r

def print_song(song):
  global types_for_name
  types_for_name = {}

  serializer = Serializer()

  sorted_ents = process_file(open('sound_%s.txt' % song, 'r'))
  if song == 'intro':
    sorted_ents.extend(process_file(open('sfx.txt' , 'r')))

  serialize_song(serializer, song, sorted(sorted_ents, key = lambda x: x.ea))

  r = produce_loadable_seq(serializer)

  result = 'kSoundBank_%s' % song, r

  if not compare_with_orig(serializer, song):
    raise Exception('compare failed')

  return result

if __name__ == "__main__":
  if len(sys.argv) == 1:
    for song in ['intro', 'indoor', 'ending']:
      print_song(song, sys.stdout)  
  else:
    print_song(sys.argv[1], sys.stdout)
