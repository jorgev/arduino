#!/usr/bin/env python

import sys
items = map(str.rstrip,open('allophones.txt','r').readlines())
a2ch = dict(zip(items,map(chr,range(32,32+len(items)))))
w2a = dict([(x,y.split()) for x,y in [l.rstrip().split(',') for l in open('alloDict.csv','r').readlines()]])
s = sys.argv[1]
out = [a2ch[a] for w in s.lower().replace(' ',' * ').split() for a in w2a.get(w, [item for sublist in [w2a.get(str(c), []) for c in w] for item in sublist])]
print ''.join(out)

