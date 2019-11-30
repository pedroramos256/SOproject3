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
#include <signal.h>
#include <errno.h>

#include "fs.h"
#include "sync.h"

#define MAXLINE     512
#define MAXCLIENTS  10
#define MAX_OPENED_FILES 5

tecnicofs* fs;                  

FILE *output;

static sigset_t signal_mask;

typedef struct openedFile{
    int inumber;
    permission p;
}*openedFile;


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

int verify_permission(int iNumber,permission p,struct ucred ucred){
    uid_t UID;
    permission ownerPerm, othersPerm;
    inode_get(iNumber,&UID,&ownerPerm,&othersPerm,NULL,0);

    return (UID == ucred.uid && (ownerPerm == RW || ownerPerm == p)) || 
        (UID != ucred.uid && (othersPerm == RW || othersPerm == p));  
}


int isFull(openedFile openedFiles[]){
    for(int i = 0;i < MAX_OPENED_FILES;i++)
        if(openedFiles[i] == NULL)return 0;
    return 1;
}


int insertFile(openedFile openedFiles[],openedFile of){
    for(int i = 0;i < MAX_OPENED_FILES;i++){
        if(openedFiles[i] == NULL){
            openedFiles[i] = of;
            return i;
        }
    }
    return -1;//never appens
}

openedFile getFile(openedFile openedFiles[],int iNumber){
    for(int i = 0;i < MAX_OPENED_FILES;i++){
        if(openedFiles[i] != NULL && openedFiles[i]->inumber == iNumber)
            return openedFiles[i];
    }
    return NULL;
}


void *give_receive_order(void *sockfd){
    permission ownerPerm, othersPerm;
    int n, iNumber, returnValue, searchResult1, searchResult2,fd;
    unsigned int len;
    char token;
    char *buffer = NULL;
    char *fileToSend;
    FILE *stream;
    size_t max;
    struct ucred ucred;
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    uid_t UID;
    int socket = (intptr_t) sockfd;

    openedFile file;
    openedFile openedFiles[MAX_OPENED_FILES] = {NULL};

    len = sizeof(struct ucred);
    if(getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        err_dump("give_receive_order: read UID error");
    if((stream = fdopen(socket, "r")) == NULL)
        err_dump("give_receive_order: fdopen failure");
    for (;;) {
        returnValue = 0;
        n = getdelim(&buffer, &max, '\0', stream);
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
            printf("%s\n", buffer);
            switch(token) {
                case 'c':
                    if(lookup(fs, arg1) == -1) {
                        ownerPerm = atoi(arg2) / 10;
                        othersPerm = atoi(arg2) % 10;
                        iNumber = inode_create((long) ucred.uid, ownerPerm, othersPerm);
                        create(fs, arg1, iNumber);
                    }
                    else returnValue = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
                    break;  
                case 'd':
                    if((iNumber = lookup(fs, arg1)) != -1) {
                        inode_get(iNumber,&UID,NULL,NULL,NULL,0);
                        printf("%d %d\n",iNumber,UID);
                        if(UID == ucred.uid) {
                            delete(fs, arg1);
                            inode_delete(iNumber);  
                        }else 
                            returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
                    }else 
                        returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                    break;
                case 'r':
                    searchResult1 = lookup(fs, arg1);
                    searchResult2 = lookup(fs, arg2);
                    if(searchResult1 != -1) {
                        if (searchResult2 == -1) {
                            inode_get(searchResult1,&UID,NULL,NULL,NULL,0);
                            if (UID == ucred.uid) {
                                exchange(fs, arg1, arg2, searchResult1);
                            }else
                                returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
                        }else
                            returnValue = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;  
                    }else
                        returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                    break;
                case 'o':
                    if(!isFull(openedFiles)) {
                        if((iNumber = lookup(fs,arg1)) != -1) {
                            if((file = getFile(openedFiles,iNumber)) == NULL) {
                                if (verify_permission(iNumber,atoi(arg2),ucred)) {
                                    file = (openedFile)malloc(sizeof(struct openedFile));
                                    file->inumber = iNumber;
                                    file->p = atoi(arg2);
                                    insertFile(openedFiles,file);
                                    returnValue = iNumber;
                                } else
                                    returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
                            } else
                                returnValue = TECNICOFS_ERROR_FILE_IS_OPEN;
                        } else
                            returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                    } else
                        returnValue = TECNICOFS_ERROR_MAXED_OPEN_FILES;
                    break;
                case 'x':
                    fd = atoi(arg1);
                    if((file = openedFiles[fd]) != NULL) {
                        openedFiles[fd] = NULL;
                    } else
                        returnValue = TECNICOFS_ERROR_FILE_NOT_OPEN;
                    break;
                case 'l':
                    fd = atoi(arg1);
                    len = atoi(arg2);
                    fileToSend = (char*)malloc(sizeof(char)*len);
                    if((file = openedFiles[fd]) != NULL) {
                        if (file->p != READ && file->p != RW)
                            returnValue = TECNICOFS_ERROR_INVALID_MODE;
                        else{
                            returnValue = inode_get(file->inumber,NULL,NULL,NULL,fileToSend,len-1);
                            
                            if(returnValue == -1) 
                                returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                            else
                                returnValue = strlen(fileToSend);
                        }   
                    } else 
                        returnValue = TECNICOFS_ERROR_FILE_NOT_OPEN;
                    
                    write(socket,fileToSend,len);
                    read(socket,buffer,0);
                    break;
                case 'w':
                    fd = atoi(arg1);
                    if((file = openedFiles[fd]) != NULL) {
                        if(file->p != WRITE && file->p != RW)
                            returnValue = TECNICOFS_ERROR_INVALID_MODE;
                        else {
                            returnValue = inode_set(file->inumber,arg2,strlen(arg2));
                            if(returnValue == -1) 
                                returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
                        }
                    } else
                        returnValue = TECNICOFS_ERROR_FILE_NOT_OPEN;
                    break;
            }
            write(socket, &returnValue, sizeof(int));
        }
    }
}

void terminate_server(int signum) {
    return;
}

/******************************************************************************
 *                                  MAIN
 *****************************************************************************/


int main(int argc, char* argv[]) {
    int num_clients = 0;
    pthread_t tid[MAXCLIENTS];
    unsigned int clilen;
    struct sigaction action;
    int error;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGINT);
    action.sa_handler = terminate_server;
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    parseArgs(argc, argv);
    /*using files given in arguments*/
    total_path = (char *)malloc(sizeof(char)*(strlen(argv[1])+6));
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
    for (;;) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        
        if (newsockfd < 0)
            break;
            
        error = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
        if (error != 0)
            err_dump("server: mask error");
        if(pthread_create(&tid[num_clients], NULL, give_receive_order, (void *)(intptr_t)newsockfd) != 0)
            err_dump("server: thread creation error");
        error = pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);
        if (error != 0)
            err_dump("server: mask error");
        else {
            num_clients++;
        }
    }
    printf("YEEES\n");
    for (int i = 0; i < num_clients; i++) {
        printf("%d\n", i);
        pthread_join(tid[i], NULL);
    }
    close(sockfd);
    /*destroys the lockers*/
    destroy();
    /* instead of stdout, it is used a file as output */
    print_tecnicofs_tree(output, fs);
    /*closing files*/
    fclose(output);
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}


