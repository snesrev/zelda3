#!/usr/bin/env python3
import concurrent.futures
import opuslib
import struct
import samplerate # pip install samplerate
import numpy as np

# File format
# 4 byte: OPUZ
# 4 byte: VERSION = 0
# Foreach Repeat Entry {
#   4 byte: seek to file offset
#   4 byte: num pcm samples to play from here
#   2 byte: # samples to skip + flags
# }

def encode_to_msu_opus(msu_infile, bitrate = 128000):
  raw_msu1 = bytearray(open(msu_infile, 'rb').read())

  if raw_msu1[:4] != b'MSU1':
    raise Exception('Expecting MSU1 file')
  msu_repeat_pos, = struct.unpack_from('<I', raw_msu1, 4)

  np_audio = np.frombuffer(raw_msu1[8:], dtype=np.int16)
  np_audio = np.reshape(np_audio, (-1, 2)).astype(np.float32) * (1.0/32768.0)
  assert np_audio.shape[1] == 2

  #print('Song has %d samples in 44.1khz: %s' % (np_audio.shape[0], np_audio.dtype))
  np_audio = samplerate.resample(np_audio, 48000 / 44100, 'sinc_best')
  source_samples = np_audio.shape[0]
  msu_repeat_pos = msu_repeat_pos * 48000 // 44100
  #print('Song has %d samples in 48khz: %s' % (np_audio.shape[0], np_audio.dtype))

  audio = bytearray(np_audio.tobytes())

  FRAME_SIZE = 960
  BYTES_PER_SAMPLE = 8
  BYTES_PER_FRAME = FRAME_SIZE * BYTES_PER_SAMPLE

  def create_encoder(bitrate):
    enc = opuslib.Encoder(48000, 2, 'audio')
    enc.vbr = True
    enc.complexity = 10
    enc.bitrate = bitrate 
    opuslib.api.encoder.encoder_ctl(enc.encoder_state, opuslib.api.ctl.ctl_set(11002), 1002)  # force celt encoding
    return enc

  def pad_data_to_frame_size(audio, lookahead):
    audio.extend(bytearray(lookahead * BYTES_PER_SAMPLE))
    rest = len(audio) % BYTES_PER_FRAME
    if rest:
      audio.extend(bytearray(BYTES_PER_FRAME - rest))

  enc = create_encoder(bitrate)
  pad_data_to_frame_size(audio, enc.lookahead)

  assert len(audio) % BYTES_PER_FRAME == 0

  result = bytearray()
  framelist = []
  for i in range(len(audio)//BYTES_PER_FRAME):
    l = enc.encode_float(bytes(audio[i*BYTES_PER_FRAME:i*BYTES_PER_FRAME+BYTES_PER_FRAME]), frame_size = FRAME_SIZE)
    code = len(l)
    if l[0] == 0xfc:
      l = l[1:]
      code = (code - 1) | 0x8000
    framelist.append((i*FRAME_SIZE, len(result)))
    result.extend(struct.pack('<H', code))
    result.extend(l)
#    print(len(l), '%.2x'%l[0])
  return result, framelist, enc.lookahead, msu_repeat_pos, source_samples

def convert_to_opuz(filename, repeat):
  print('Converting %s' % filename)
  result, framelist, preskip, msu_repeat_pos, songlength = encode_to_msu_opus(filename, 128000)

  # lookup what sample to seek to based on a sample nr
  def lookup_sample(framelist, sample_nr):
    last = None
    for sample, fileoffs in framelist:
      if sample_nr < sample:
        break
      last = sample, fileoffs
    return last

  def calc_play_ranges(xs):
    r = []
    for i, (start, end, repeat_from_here) in enumerate(xs):
      frame = lookup_sample(framelist, start)
      r.append((frame[1], end - start, preskip + start - frame[0], start, repeat_from_here))
    return r

  def serialize_header(xs):
    r = b'OPUZ' + struct.pack('<I', 0)
    offs = len(r) + len(xs) * 10
    for i, (a, b, c, d, e) in enumerate(xs):
      f = c | (0x4000 if e else 0x0) | (0x8000 if i == len(xs) - 1 else 0)
      r += struct.pack('<IIH', a + offs, b, f)
    return r

  # lookup the frame that contains msu_repeat_pos
  if repeat:
    play_ranges = [(0, songlength, False), (msu_repeat_pos, songlength, True)]
  else:
    play_ranges = [(0, songlength, False)]

  play_ranges = calc_play_ranges(play_ranges)

  header = serialize_header(play_ranges)
  if filename.endswith('.pcm'):
    filename = filename[:-4]
  open(filename + '.opuz', 'wb').write(header + result)

kMsuTracksWithRepeat = [
  1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,0,
  1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,
  1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
]

def task(i):
  convert_to_opuz(r"../../msu/ALttP-msu-Deluxe-%d.pcm" % i, i >= 48 or kMsuTracksWithRepeat[i])

with concurrent.futures.ThreadPoolExecutor() as executor:
  results = [executor.submit(task, i) for i in range(1, 115)]

  for future in concurrent.futures.as_completed(results):
    result = future.result()

