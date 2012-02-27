#!/usr/bin/env python
import sys
import fcntl
import struct

def get_blkdev_size(fd):
	BLKGETSIZE64 = 2148012658
	s = fcntl.ioctl(fd, BLKGETSIZE64, ' '*8)
	return struct.unpack('L', s)[0]

class SnapshotDevice(object):
	def __init__(self, filename):
		self.filename = filename
		self.f = open(filename, 'rb')
		self.devsize = get_blkdev_size(self.f.fileno())
		self.chunk_size = self.chunk_bytes = 0
		self.exc_per_area = 0

	def read_header(self):
		self.f.seek(0)
		buf = self.f.read(4 * 4)
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
		self.f.seek(chunk * self.chunk_bytes)
		area = self.f.read(self.chunk_bytes)
		for i in xrange(0, self.exc_per_area*16, 16):
			old_chunk,new_chunk = struct.unpack('<QQ', area[i:i+16])
			if new_chunk == 0: return False
			print 'COW %ld => %ld' % (old_chunk, new_chunk)
			if show_diff:
				new_data = self.read_chunk(new_chunk)
				for j in xrange(0, self.chunk_bytes, 16):
					s = new_data[j:j+16]
					s1 = ' '.join(['%04x' % x for x in struct.unpack('>8H', s)])
					s2 = ''.join([x.isalnum() and x or '.' for x in s])
					print '%04x:' % j, s1, s2
		return True

	def read_chunk(self, chunk):
		self.f.seek(chunk * self.chunk_bytes)
		return self.f.read(self.chunk_bytes)

sd = SnapshotDevice(sys.argv[1])
if not sd.read_header(): sys.exit(1)
#print sd.num_areas()
for i in xrange(sd.num_areas()):
	if not sd.dump_area(i, True): break
