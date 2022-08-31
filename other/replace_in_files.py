import os, re, sys, glob

repl = open(sys.argv[1],'r').read().splitlines()
repl = [a.split(' ') for a in repl]
repl_dict = dict(repl)

c = re.compile('\\b(' + "|".join(a[0] for a in repl) + ')\\b')

def replacer(m):
  return repl_dict[m.group(0)]

def replace_in_file(fname):
  s = open(fname, 'r').read()
  s_org = s

  s, num = c.subn(replacer, s)
  print(num)
  if num:
    assert s != s_org
    open(fname, 'w').write(s)
  


for filename in glob.glob('*.c') + glob.glob('*.h'):
  replace_in_file(filename)



