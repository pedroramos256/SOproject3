#include "sync.h"

#include <stdio.h>
#include <stdlib.h>


void err_dump(char * str){
    printf("%s\n",str);
    exit(EXIT_FAILURE);
}

/* does an error verification for the pthread functions */
void verify_func(int state,char * message) {
    if (state != 0) {
        printf("%s\n",message);  
        exit(EXIT_FAILURE);
    }
}

void init() {
    /* initializes lockers for de vector of bst's */
    int i;
	lock = (pthread_rwlock_t*)malloc(numberBuckets*sizeof(pthread_rwlock_t));
    if(!lock){
		perror("failed to allocate vector of lockers");
		exit(EXIT_FAILURE);
    }
    printf("antes\n");
    inode_table_init();
    printf("depois\n");
	for (i = 0; i < numberBuckets; i++)
		INIT(i);
	/* Cria socket stream */
    if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0) ) < 0)
        err_dump("server: can't open stream socket");

    /* Elimina o nome, para o caso de já existir.*/
    unlink(total_path);

    /* O nome serve para que os clientes possam identificar o servidor */
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, total_path);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
        err_dump("server, can't bind local address");
}

void destroy() {
    /* destroys lockers for de vector of bst's */
	int i;
    for (i = 0; i < numberBuckets; i++)
		DESTROY(i);

    inode_table_destroy();
}