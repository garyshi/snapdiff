CC=gcc
TARGETS=dumpsnap rawread snapdiff

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

rawread: rawread.o common.o

dumpsnap: dumpsnap.o common.o

snapdiff: snapdiff.o common.o
