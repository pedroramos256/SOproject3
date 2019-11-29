#include "../tecnicofs-client-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s sock_path\n", argv[0]);
        exit(0);
    }
    
    /*char readBuffer[4] = {0};*/
    printf("Test: tfsMount sucess\n");
    assert(tfsMount(argv[1]) == 0);
    printf("Test: tfsCreate sucess\n");
    assert(tfsCreate("abc", RW, READ) == 0);
    printf("Test: tfsRename sucess\n");
    assert(tfsRename("abc", "bcd") == 0);
    /*int fd = -1;
    assert((fd = tfsOpen("bcd", RW)) == 0);

    assert(tfsWrite(fd, "hmm", 3) == 0);

    assert(tfsRead(fd, readBuffer, 4) == 3);

    puts(readBuffer);

    assert(tfsClose(fd) == 0);*/
    printf("Test: tfsDelete sucess\n");
    assert(tfsDelete("bcd") == 0);
    printf("Test: tfsUnmount sucess\n");
    assert(tfsUnmount() == 0);

    return 0;
}