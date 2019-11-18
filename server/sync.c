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
    /* initializes lockers for the sempahores */
    verify_func(pthread_mutex_init(&v1_lock,NULL),ERROR_INIT);
    verify_func(pthread_mutex_init(&v2_lock,NULL),ERROR_INIT);

    /* initializes lockers for de vector of bst's */
	lock = (lock_t*)malloc(numberBuckets*sizeof(lock_t));
    if(!lock){
		perror("failed to allocate vector of lockers");
		exit(EXIT_FAILURE);
    }
	int i;
	for (i = 0; i < numberBuckets; i++)
		INIT(i);
    /* initializes the semaphores */
    verify_func(sem_init(&produce, 0, MAX_COMMANDS), ERROR_INIT);
    verify_func(sem_init(&consume, 0, 0), ERROR_INIT);
}

void destroy() {
    /* destroys lockers for the semaphores */
    verify_func(pthread_mutex_destroy(&v1_lock),ERROR_DESTROY);
    verify_func(pthread_mutex_destroy(&v2_lock),ERROR_DESTROY);

    /* destroys lockers for de vector of bst's */
	int i;
    for (i = 0; i < numberBuckets; i++)
		DESTROY(i);
	free(lock);
    /* destroys semaphores */
    verify_func(sem_destroy(&produce), ERROR_DESTROY);
    verify_func(sem_destroy(&consume), ERROR_DESTROY);
}