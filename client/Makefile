# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = tecnicofs-client-api.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread

.PHONY: all clean

all: apiclient
	
tecnicofs-api-client.o: tecnicofs-client-api.c tecnicofs-client-api.h tecnicofs-api-constants.h


apiclient: tecnicofs-api-client.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
