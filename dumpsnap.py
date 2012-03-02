#!/usr/bin/env python
import sys, stat, os
import fcntl
import struct

def get_blkdev_size(fd):
	BLKGETSIZE64 = 2148012658
	s = fcntl.ioctl(fd, BLKGETSIZE64, ' '*8)
	return struct.unpack('L', s)[0]

class SnapshotDevice(object):
	def __init__(self, filename):
		statinfo = os.stat(filename)
		self.filename = filename
		self.f = open(filename, 'rb')
		if stat.S_ISBLK(statinfo.st_mode): self.devsize = get_blkdev_size(self.f.fileno())
		else: self.devsize = statinfo.st_size
		self.chunk_size = self.chunk_bytes = 0
		self.exc_per_area = 0

	def dev_read(self, pos, size):
		self.f.seek(pos)
		return self.f.read(size)

	def read_header(self):
		buf = self.dev_read(0, 4 * 4)
		p = struct.unpack('<4I', buf)
		print 'HEADER'
		print '  magic: %08x' % p[0]
		print '  valid: %d' % p[1]
		print '  version: %d' % p[2]
		print '  chunk_size in sectors: %d' % p[3]
		print '  device size: %d' % self.devsize
		if p[0] != 0x70416e53:
			print >>sys.stderr, 'Error: magic mismatch'
			return False
		if p[1] == 0:
			print >>sys.stderr, 'Error: invalid snapshot'
			return False
		if p[2] != 1:
			print >>sys.stderr, 'Error: unsupported version'
			return False
		self.chunk_size = p[3]
		self.chunk_bytes = self.chunk_size * 512
		self.exc_per_area = self.chunk_bytes / 16
		self.area_bytes = (self.exc_per_area + 1) * self.chunk_bytes
		print '  chunk_size in bytes:', self.chunk_bytes
		print '  exc_per_area:', self.exc_per_area
		print '  area_bytes:', self.area_bytes
		return True

	def num_areas(self):
		return self.devsize / self.area_bytes - 1

	def area_to_chunk(self, area):
		return 1 + ((self.exc_per_area + 1) * area)

	def dump_area(self, area, show_diff=False):
		chunk = self.area_to_chunk(area)
		print 'area %d chunk: %ld => offset %ld' % (area, chunk, chunk * self.chunk_bytes)
		area = self.dev_read(chunk * self.chunk_bytes, self.chunk_bytes)
		for i in xrange(0, self.exc_per_area*16, 16):
			old_chunk,new_chunk = struct.unpack('<QQ', area[i:i+16])
			if new_chunk == 0: return False
			print 'COW %ld => %ld' % (old_chunk, new_chunk)
			if show_diff:
				new_data = self.read_chunk(new_chunk)
				for j in xrange(0, self.chunk_bytes, 16):
					s = new_data[j:j+16]
					if s != '\x00'*16 and s != '\xff'*16:
						s1 = ' '.join(['%04x' % x for x in struct.unpack('>8H', s)])
						s2 = ''.join([x.isalnum() and x or '.' for x in s])
						print '%04x:' % j, s1, s2
		return True

	def read_chunk(self, chunk):
		return self.dev_read(chunk * self.chunk_bytes, self.chunk_bytes)

verbose = False
if len(sys.argv) == 3 and sys.argv[1] == '-v':
	verbose = True
	sys.argv.pop(0)

sd = SnapshotDevice(sys.argv[1])
if not sd.read_header(): sys.exit(1)
#print sd.num_areas()
for i in xrange(sd.num_areas()):
	if not sd.dump_area(i, verbose): break
