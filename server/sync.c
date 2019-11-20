#include "sync.h"

#include <stdio.h>
#include <stdlib.h>

/* does an error verification for the pthread functions */
void verify_func(int state,char * message) {
    if (state != 0) {
        printf("%s\n",message);  
        exit(EXIT_FAILURE);
    }
}

void init() {
    /* initializes lockers for de vector of bst's */
	lock = (pthread_rwlock_t*)malloc(numberBuckets*sizeof(pthread_rwlock_t));
    if(!lock){
		perror("failed to allocate vector of lockers");
		exit(EXIT_FAILURE);
    }
	int i;
	for (i = 0; i < numberBuckets; i++)
		INIT(i);
}

void destroy() {
    /* destroys lockers for de vector of bst's */
	int i;
    for (i = 0; i < numberBuckets; i++)
		DESTROY(i);
	free(lock);
}