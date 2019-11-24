#include "../tecnicofs-client-api.h"
#include <assert.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s sock_path\n", argv[0]);
        exit(0);
    }
    assert(tfsMount(argv[1]) == 0);
    printf("Test: tfsMount sucess\n");
    assert(tfsCreate("Somos bosses", RW, READ) == 0);
    printf("Test: create file with name that already exists\n");
    assert(tfsCreate("Again", RW, READ) == 0);
    printf("Test: create sucess\n");
    sleep(2);
    assert(tfsUnmount() == 0);
    printf("Test: tfsUnmount sucess\n");

    return 0;
}
