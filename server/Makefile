# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = main.c fs.c sync.c
SOURCES+= lib/bst.c lib/hash.c lib/inodes.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread

.PHONY: all clean

all: tecnicofs
	
lib/bst.o: lib/bst.c lib/bst.h
lib/hash.o: lib/hash.c lib/hash.h
lib/inodes.o: lib/inodes.c lib/inodes.h

fs.o: fs.c fs.h lib/bst.h lib/hash.h lib/inodes.h

sync.o: sync.c sync.h

main.o: main.c fs.h lib/bst.h lib/hash.h lib/inodes.h sync.h

tecnicofs: lib/bst.o lib/hash.o lib/inodes.o fs.o sync.o main.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
