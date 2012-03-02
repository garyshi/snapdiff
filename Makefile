CC=gcc
TARGETS=dumpsnap rawread

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

dumpsnap: dumpsnap.o common.o

rawread: rawread.o common.o
