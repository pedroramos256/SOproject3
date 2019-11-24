# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES =  api-tests/client-api-test-create.c tecnicofs-client-api.c
#SOURCES += api-tests/client-api-test-delete.c api-tests/client-api-test-read.c api-tests/client-api-test-success.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
#TARGETS=client-api-test-create client-api-test-delete client-api-test-read client-api-test-success

.PHONY: all clean

all: api-tests/client-api-test-create

#$(TARGETS):
	#$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tecnicofs-client-api.o: tecnicofs-client-api.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<
	
api-tests/client-api-test-create.o: api-tests/client-api-test-create.c tecnicofs-client-api.h tecnicofs-api-constants.h 
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-create: tecnicofs-client-api.o api-tests/client-api-test-create.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

#client-api-test-delete.o: client-api-test-delete.c ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h

#client-api-test-delete: client-api-test-delete.o tecnicofs-api-client.o

#client-api-test-read.o: client-api-test-read.c  ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h

#client-api-test-read: client-api-test-read.o tecnicofs-api-client.o

#client-api-test-success.o: client-api-test-success.c ../client/tecnicofs-client-api.h ../client/tecnicofs-api-constants.h

#client-api-test-success: client-api-test-success.o tecnicofs-api-client.o



	

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
