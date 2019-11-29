# Sistemas Operativos, DEI/IST/ULisboa 2019-20

#SOURCES = tecnicofs-client-api.c api-tests/client-api-test-create.c 
#SOURCES = tecnicofs-client-api.c api-tests/client-api-test-delete.c
SOURCES = tecnicofs-client-api.c api-tests/client-api-test-success.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread

.PHONY: all clean

#all: api-tests/client-api-test-create
#all: api-tests/client-api-test-delete
all: api-tests/client-api-test-success

#api-tests/client-api-test-create.o: api-tests/client-api-test-create.c tecnicofs-client-api.h tecnicofs-api-constants.h 
	#$(CC) $(CFLAGS) -c -o $@ $<

#api-tests/client-api-test-create: tecnicofs-client-api.o api-tests/client-api-test-create.o
	#$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tecnicofs-client-api.o: tecnicofs-client-api.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<

#api-tests/client-api-test-delete.o: api-tests/client-api-test-delete.c tecnicofs-client-api.h tecnicofs-api-constants.h
	#$(CC) $(CFLAGS) -c -o $@ $<

#api-tests/client-api-test-delete: tecnicofs-client-api.o api-tests/client-api-test-delete.o
	#$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

#client-api-test-read.o: client-api-test-read.c  ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h

#client-api-test-read: client-api-test-read.o tecnicofs-api-client.o

api-tests/client-api-test-success.o: api-tests/client-api-test-success.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-success: tecnicofs-client-api.o api-tests/client-api-test-success.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	@echo Cleaning...
	rm -f $(OBJS) api-tests/client-api-test-success
