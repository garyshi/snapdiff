#!/usr/bin/env python
import sys, struct

f = open(sys.argv[1], 'rb')
chunk_bytes = int(sys.argv[2]) * 512
chunk = long(sys.argv[3])

f.seek(chunk * chunk_bytes)
data = f.read(chunk_bytes)
for i in xrange(0, chunk_bytes, 16):
	s = data[i:i+16]
	if s != '\x00'*16 and s != '\xff'*16:
		s1 = ' '.join(['%04x' % x for x in struct.unpack('>8H', s)])
		s2 = ''.join([x.isalnum() and x or '.' for x in s])
		print '%04x:' % i, s1, s2

