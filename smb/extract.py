# pip install zstandard
import zstandard
import hashlib

SHA1_HASH = 'c05817c5b7df2fbfe631563e0b37237156a8f6b6' # smas
SHA1_HASH_SMB1 = '4a5278150f3395419d68cb02a42f7c3c62cdf8b4'
SHA1_HASH_SMBLL = '493e14812af7a92d0eacf00ba8bb6d3a266302ca'

smas = open('../smas.sfc', 'rb').read()
hash = hashlib.sha1(smas).hexdigest()
if hash != SHA1_HASH:
  raise Exception('You need SMAS with sha1 ' + SHA1_HASH + ' yours is ' + hash)

dict_data = zstandard.ZstdCompressionDict(smas)

cctx = zstandard.ZstdDecompressor(dict_data=dict_data)
out = cctx.decompress(open('smb1.zst', 'rb').read())

hash = hashlib.sha1(out).hexdigest()
if hash != SHA1_HASH_SMB1:
  raise Exception('Error. SMB1 hash is supposed to be ' + SHA1_HASH_SMB1 + ' yours is ' + hash)

with open('smb1.sfc', 'wb') as ofp:
    ofp.write(out)


cctx = zstandard.ZstdDecompressor(dict_data=dict_data)
out = cctx.decompress(open('smbll.zst', 'rb').read())

hash = hashlib.sha1(out).hexdigest()
if hash != SHA1_HASH_SMBLL:
  raise Exception('Error. SMBLL hash is supposed to be ' + SHA1_HASH_SMBLL + ' yours is ' + hash)

with open('smbll.sfc', 'wb') as ofp:
    ofp.write(out)
