#ifndef SYNC_H
#define SYNC_H

#include <unistd.h>
#include <pthread.h>

/* Error messages */
#define ERROR_LOCK "lock failure"
#define ERROR_UNLOCK "unlock failure"
#define ERROR_INIT "init failure"
#define ERROR_DESTROY "destroy failure"

        

pthread_rwlock_t *lock;


int numberBuckets; 

void verify_func(int state,char * message);
void init();
void destroy();


#define RDLOCK(i)    verify_func(pthread_rwlock_rdlock(&lock[i]),ERROR_LOCK)
#define WRLOCK(i)    verify_func(pthread_rwlock_wrlock(&lock[i]),ERROR_LOCK)
#define UNLOCK(i)  verify_func(pthread_rwlock_unlock(&lock[i]),ERROR_UNLOCK)

#define INIT(i)      verify_func(pthread_rwlock_init(&lock[i],NULL),ERROR_INIT);
#define DESTROY(i)   verify_func(pthread_rwlock_destroy(&lock[i]),ERROR_DESTROY);




#endif /* SYNC_H */