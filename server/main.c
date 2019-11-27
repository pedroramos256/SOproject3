/*
*   File System made by
*       Pedro Ramos 92539 and Sancha Barroso 92557
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#include "fs.h"
#include "sync.h"
#include "../client/tecnicofs-api-constants.h"

#define MAXLINE     512
#define MAXCLIENTS  10

tecnicofs* fs;                  


FILE *output;

/******************************************************************************
 *                         FUNCTIONS USED IN MAIN
 *****************************************************************************/




static void displayUsage (const char* appName){
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}


static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

/*prints time*/
void execution_time(struct timeval start, struct timeval end) {
    double time = (end.tv_sec - start.tv_sec) * 1e6; 
    time = (time + (end.tv_usec - start.tv_usec)) * 1e-6; 
	printf("TecnicoFS completed in %.4f seconds.\n", time);
}

void * give_receive_order(void *sockfd){
    int n, iNumber, ownerPerm, othersPerm, returnValue = 0;
    unsigned int len;
    char token;
    char *buffer;
    FILE *stream;
    size_t max;
    struct ucred ucred;
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    int socket = (intptr_t) sockfd;
    len = sizeof(struct ucred);
    stream = fdopen(socket, "r");
    if(getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        err_dump("give_receive_order: read UID error");
    for (;;) {
        printf("chegou aqui\n");
        /*printf("--%s--\n",buffer);*/
        printf("!!\n");
        n = getdelim(&buffer, &max, 0, stream);
        /* Lê uma linha do socket */
        if (n == -1) {
            printf("socket closed\n");
            close(socket);
            return NULL;
        }
        else if (n < 0)
            err_dump("give_receive_order: readline error");
        else {
            sscanf(buffer, "%c %s %s", &token, arg1, arg2);
            switch(token) {
                case 'c':
                    if(lookup(fs,arg1) == 0) {
                        ownerPerm = atoi(arg2) / 10;
                        othersPerm = atoi(arg2) % 10;
                        iNumber = inode_create((long) ucred.uid, ownerPerm, othersPerm);
                        create(fs, arg1, iNumber);
                    } 
                    else {
                        printf("entrou\n");
                        returnValue = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
                    }

                    break;
                case 'd':
                    if(lookup(fs,arg1) == 0)
                        returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                    else{
                        uid_t ownerUID;
                        iNumber = lookup(fs,arg1);
                        inode_get(iNumber,&ownerUID,NULL,NULL,NULL,0);
                        if(ownerUID == ucred.uid)
                            returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
                        else{
                            delete(fs,arg1);
                            inode_delete(iNumber);
                        }
                    }
                    break;
            }
            write(socket, &returnValue, sizeof(int));
            returnValue = 0;
        }
    }
}

/******************************************************************************
 *                                  MAIN
 *****************************************************************************/


int main(int argc, char* argv[]) {
    int num_clients = 0;
    pthread_t tid[MAXCLIENTS];
    unsigned int clilen;

    parseArgs(argc, argv);
    /*using files given in arguments*/
    total_path = (char *)malloc(sizeof(char)*(strlen(argv[1])+5));
    sprintf(total_path, "/tmp/%s", argv[1]);
    output = fopen(argv[2], "w");
    if (output == NULL) {   
        err_dump("output file open failure");
    }
    numberBuckets = atoi(argv[3]);
    if(numberBuckets <= 0){
        err_dump("invalid number of buckets");
    }
    fs = new_tecnicofs();
    /*initializes the thread lockers and semaphores needed with validation */
    init();
    listen(sockfd, MAXCLIENTS);
    printf("funciona\n");
    for (;;) {
        clilen = sizeof(cli_addr);
        newsockfd =accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            err_dump("server: accept error");
        if(pthread_create(&tid[num_clients], NULL, give_receive_order, (void *)(intptr_t)newsockfd) != 0)
            err_dump("server: thread creation error");
        else num_clients++;
    }
    /*destroys the lockers*/
    destroy();
    /* instead of stdout, it is used a file as output */
    print_tecnicofs_tree(output, fs);
    /*closing files*/
    fclose(output);
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}


