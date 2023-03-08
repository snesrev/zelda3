import array

memos = {}
memoslist = []
def memo(s):
  m = memos.get(s)
  if m == None:
    m = len(memoslist)
    memos[s] = m
    memoslist.append(s)
  return m

def tos(s): return "".join(memoslist[c] for c in s)

lines = []
for line in open('dialogue.txt', 'r').read().splitlines():
  line = line.split(': ')[1]

  r = array.array('H')

  i = 0
  while i < len(line):
    if line[i] == '[':
      j = line.index(']', i + 1)
      r.append(memo(line[i:j+1]))
      i = j + 1
    else:
      r.append(memo(line[i]))
      i += 1
      
  #print(repr(line))
  #print(r)
  lines.append(list(r))
import collections


def find_all_ngrams(lines, N, cost):
  ctr = collections.Counter()
  for line in lines:
    for i in range(len(line) - N + 1):
      if line[i] != line[i+1]:
        ctr[tuple(line[i:i+N])] += 1
  r = list((b, a) for a, b in ctr.items() if b >= 2)
  if len(r) == 0:
    return None, 0
  b, a = max(r)
  return a, (N - cost) * b - N - 2 # 2 is the overhead of the dict

def find_best_ngram(cost):
  best_score=0

  for i in range(2, 32):
    text, score = find_all_ngrams(lines, i, cost)
    if score > best_score:
      best_score = score
      best_text = text
  return best_score, best_text

def update_ngrams(lines, replace_from, replace_to):
  for line in lines:
    for i in range(len(line) - len(replace_from) + 1):
      if tuple(line[i:i+len(replace_from)]) == replace_from:
        line[i:i+len(replace_from)] = replace_to

total_gain = 0

original_tokens = sum(len(line) for line in lines)


kTextDictionary_US = [
'    ', '   ', '  ', "'s ", 'and ', 
'are ', 'all ', 'ain', 'and', 'at ', 
'ast', 'an', 'at', 'ble', 'ba', 
'be', 'bo', 'can ', 'che', 'com', 
'ck', 'des', 'di', 'do', 'en ', 
'er ', 'ear', 'ent', 'ed ', 'en', 
'er', 'ev', 'for', 'fro', 'give ', 
'get', 'go', 'have', 'has', 'her', 
'hi', 'ha', 'ight ', 'ing ', 'in', 
'is', 'it', 'just', 'know', 'ly ', 
'la', 'lo', 'man', 'ma', 'me', 
'mu', "n't ", 'non', 'not', 'open', 
'ound', 'out ', 'of', 'on', 'or', 
'per', 'ple', 'pow', 'pro', 're ', 
're', 'some', 'se', 'sh', 'so', 
'st', 'ter ', 'thin', 'ter', 'tha', 
'the', 'thi', 'to', 'tr', 'up', 
'ver', 'with', 'wa', 'we', 'wh', 
'wi', 'you', 'Her', 'Tha', 'The', 
'Thi', 'You', 
]


dictionary = []

for i in range(111+256):
  best_score, best_text = find_best_ngram(1 if i < 111 else 2)
  if best_score == 0:
    break

  total_gain += best_score

  print(f'Removed best bigram "{tos(best_text)}" with gain {best_score}, total gain {total_gain} / {original_tokens}')

  dictionary.append(best_text)

  update_ngrams(lines, best_text, [memo('{%s}' % tos(best_text))])

#print('kTextDictionary_NEW = [')
#for i, d in enumerate(dictionary):
#  repl = tos(d).replace('{', '').replace('}', '')
#  print(f'{repr(repl)},')
#print(']')


for i, a in enumerate(lines):
  print(i, tos(a))
