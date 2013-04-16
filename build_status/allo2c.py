#!/usr/bin/env python

items = map(str.rstrip,open('allophones.txt','r').readlines())
a2ch = dict(zip(items,map(chr,range(32,32+len(items)))))
words = [(x,''.join([a2ch[a] for a in y.split()])) for x,y in [l.rstrip().split(',') for l in open('alloDict.csv','r').readlines()]]
words.sort()
print 'struct W2V { char* word; char* data; };'
print 'struct W2V w2v[] = {'
print ',\n'.join(map('    {{"{0[0]}", "{0[1]}"}}'.format, [(w,v.replace('\\','\\\\').replace('"','\\"')) for w,v in words] ))
print '};'

