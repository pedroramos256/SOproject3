# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = tecnicofs-client-api.c 
SOURCES += api-tests/client-api-test-create.c api-tests/client-api-test-delete.c
SOURCES += api-tests/client-api-test-read.c api-tests/client-api-test-success.c
OBJS = $(SOURCES:%.c=%.o)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS = -lm -pthread
TARGETS = api-tests/client-api-test-create api-tests/client-api-test-delete
TARGETS += api-tests/client-api-test-read api-tests/client-api-test-success

.PHONY: all clean

all: $(TARGETS)

tecnicofs-client-api.o: tecnicofs-client-api.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-create.o: api-tests/client-api-test-create.c tecnicofs-client-api.h tecnicofs-api-constants.h 
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-create: tecnicofs-client-api.o api-tests/client-api-test-create.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


api-tests/client-api-test-delete.o: api-tests/client-api-test-delete.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<


api-tests/client-api-test-read.o: api-tests/client-api-test-read.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-read: tecnicofs-client-api.o api-tests/client-api-test-read.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

api-tests/client-api-test-success.o: api-tests/client-api-test-success.c tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -c -o $@ $<

api-tests/client-api-test-success: tecnicofs-client-api.o api-tests/client-api-test-success.o
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
