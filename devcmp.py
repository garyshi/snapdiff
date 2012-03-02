#!/usr/bin/env python
import sys
import struct
import binascii

i = 1
CHUNK_BYTES = 4096
if sys.argv[i] == '-c':
	CHUNK_BYTES = int(sys.argv[i+1])
	i += 2
f1 = open(sys.argv[i+0], 'rb')
f2 = open(sys.argv[i+1], 'rb')

i = 0
while True:
	s1 = f1.read(CHUNK_BYTES)
	s2 = f2.read(CHUNK_BYTES)
	if not s1 or not s2: break
	if s1 != s2:
		print 'DIFF in chunk', i
		for j in xrange(0, CHUNK_BYTES, 16):
			x1 = s1[j:j+16]
			x2 = s2[j:j+16]
			if x1 != x2: print '%04x:'%j, binascii.b2a_hex(x1), binascii.b2a_hex(x2)
	i += 1
