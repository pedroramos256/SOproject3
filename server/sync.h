#ifndef SYNC_H
#define SYNC_H

#include <unistd.h>
#include <pthread.h>

/* Error messages */
#define ERROR_LOCK "lock failure"
#define ERROR_UNLOCK "unlock failure"
#define ERROR_INIT "init failure"
#define ERROR_DESTROY "destroy failure"
#define ERROR_TRYLOCK "trylock failure"
#define ERROR_WAIT "semaphore wait failure"
#define ERROR_POST "semaphore post failure"


#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

pthread_mutex_t v1_lock;         /* locker for the producer thread */
pthread_mutex_t v2_lock;         /* locker for the consumer threads */           

/* different types of lockers for different flags */
#ifdef MUTEX
    #define lock_t      pthread_mutex_t
#elif RWLOCK
    #define lock_t      pthread_rwlock_t
#else
    #define lock_t      void*
#endif

lock_t *lock;                    /* lockers for the bst's */

int numberBuckets; 

void verify_func(int state,char * message);
void init();
void destroy();

/*defining macros*/
#define SEM_WAIT(i)      verify_func(sem_wait(&i),ERROR_WAIT)
#define SEM_POST(i)      verify_func(sem_post(&i),ERROR_POST)

#ifdef MUTEX

    #define LOCK1()      verify_func(pthread_mutex_lock(&v1_lock),ERROR_LOCK)
    #define UNLOCK1()    verify_func(pthread_mutex_unlock(&v1_lock),ERROR_UNLOCK)

    #define LOCK2()      verify_func(pthread_mutex_lock(&v2_lock),ERROR_LOCK)
    #define UNLOCK2()    verify_func(pthread_mutex_unlock(&v2_lock),ERROR_UNLOCK)

    #define RDLOCK(i)    verify_func(pthread_mutex_lock(&lock[i]),ERROR_LOCK)
    #define WRLOCK(i)    verify_func(pthread_mutex_lock(&lock[i]),ERROR_LOCK)
    #define RWUNLOCK(i)  verify_func(pthread_mutex_unlock(&lock[i]),ERROR_UNLOCK)

    #define INIT(i)      verify_func(pthread_mutex_init(&lock[i],NULL),ERROR_INIT);
    #define DESTROY(i)   verify_func(pthread_mutex_destroy(&lock[i]),ERROR_DESTROY);


#elif RWLOCK

    #define LOCK1()      verify_func(pthread_mutex_lock(&v1_lock),ERROR_LOCK)
    #define UNLOCK1()    verify_func(pthread_mutex_unlock(&v1_lock),ERROR_UNLOCK)

    #define LOCK2()      verify_func(pthread_mutex_lock(&v2_lock),ERROR_LOCK)
    #define UNLOCK2()    verify_func(pthread_mutex_unlock(&v2_lock),ERROR_UNLOCK)

    #define RDLOCK(i)    verify_func(pthread_rwlock_rdlock(&lock[i]),ERROR_LOCK)
    #define WRLOCK(i)    verify_func(pthread_rwlock_wrlock(&lock[i]),ERROR_LOCK)
    #define RWUNLOCK(i)  verify_func(pthread_rwlock_unlock(&lock[i]),ERROR_UNLOCK)

    #define INIT(i)      verify_func(pthread_rwlock_init(&lock[i],NULL),ERROR_INIT);
    #define DESTROY(i)   verify_func(pthread_rwlock_destroy(&lock[i]),ERROR_DESTROY);


/*when using nosync, it is just empty functions*/
#else
    #define lock_t void*

    #define LOCK1() {}
    #define UNLOCK1() {}
    #define LOCK2() {}
    #define UNLOCK2() {}

    #define RDLOCK(i) {}
    #define WRLOCK(i) {}
    #define RWUNLOCK(i) {}

    #define INIT(i) {}
    #define DESTROY(i) {}

#endif

#endif /* SYNC_H */