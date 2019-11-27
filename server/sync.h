#ifndef SYNC_H
#define SYNC_H

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../unix.h"
#include "lib/inodes.h"
#include "../client/tecnicofs-api-constants.h"

/* Error messages */
#define ERROR_LOCK "lock failure"
#define ERROR_UNLOCK "unlock failure"
#define ERROR_INIT "init failure"
#define ERROR_DESTROY "destroy failure"

pthread_rwlock_t *lock;
pthread_mutex_t inode_lock;

int numberBuckets; 
char *total_path;
int sockfd, newsockfd, servlen;
struct sockaddr_un cli_addr, serv_addr;

void err_dump(char *str);
void verify_func(int state,char * message);
void init();
void destroy();

#define RDLOCK(i)    verify_func(pthread_rwlock_rdlock(&lock[i]),ERROR_LOCK)
#define WRLOCK(i)    verify_func(pthread_rwlock_wrlock(&lock[i]),ERROR_LOCK)
#define UNLOCK(i)  verify_func(pthread_rwlock_unlock(&lock[i]),ERROR_UNLOCK)

#define INIT(i)      verify_func(pthread_rwlock_init(&lock[i],NULL),ERROR_INIT);
#define DESTROY(i)   verify_func(pthread_rwlock_destroy(&lock[i]),ERROR_DESTROY);




#endif /* SYNC_H */