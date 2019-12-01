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
#include <sys/time.h>

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
    return -1;//never happens
}

openedFile getFile(openedFile openedFiles[],int iNumber){
    for(int i = 0;i < MAX_OPENED_FILES;i++){
        if(openedFiles[i] != NULL && openedFiles[i]->inumber == iNumber)
            return openedFiles[i];
    }
    return NULL;
}

int serverCreate(char *arg1, char *arg2, struct ucred ucred) {
    permission ownerPerm, othersPerm;
    int iNumber, returnValue = 0;
     if(lookup(fs, arg1) == -1) {
        ownerPerm = atoi(arg2) / 10;
        othersPerm = atoi(arg2) % 10;
        iNumber = inode_create((long) ucred.uid, ownerPerm, othersPerm);
        create(fs, arg1, iNumber);
    }
    else returnValue = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    return returnValue;
}

int serverDelete(char *arg1, struct ucred ucred) {
    int iNumber, returnValue = 0;
    uid_t UID;

    if((iNumber = lookup(fs, arg1)) != -1) {
        inode_get(iNumber,&UID,NULL,NULL,NULL,0);
        if(UID == ucred.uid) {
            delete(fs, arg1);
            inode_delete(iNumber);  
        } else 
            returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
    }
    else returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
    return returnValue;
}

int serverRename(char *arg1, char *arg2, struct ucred ucred) {
    int searchResult1, searchResult2, returnValue = 0;
    uid_t UID;
    searchResult1 = lookup(fs, arg1);
    searchResult2 = lookup(fs, arg2);
    if(searchResult1 != -1) {
        if (searchResult2 == -1) {
            inode_get(searchResult1,&UID,NULL,NULL,NULL,0);
            if (UID == ucred.uid) {
                exchange(fs, arg1, arg2, searchResult1);
            } else
                returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
        } else
            returnValue = TECNICOFS_ERROR_FILE_ALREADY_EXISTS;  
    } else
    returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
    return returnValue;
}

int serverOpen(char *arg1, char *arg2, openedFile openedFiles[], struct ucred ucred) {
    int iNumber, returnValue = 0;
    openedFile file;
    if(!isFull(openedFiles)) {
        if((iNumber = lookup(fs,arg1)) != -1) {
            if((file = getFile(openedFiles,iNumber)) == NULL) {
                if (verify_permission(iNumber,atoi(arg2),ucred)) {
                    file = (openedFile)malloc(sizeof(struct openedFile));
                    file->inumber = iNumber;
                    file->p = atoi(arg2);
                    returnValue = insertFile(openedFiles,file);
                } else
                    returnValue = TECNICOFS_ERROR_PERMISSION_DENIED;
            } else
                returnValue = TECNICOFS_ERROR_FILE_IS_OPEN;
        } else
            returnValue = TECNICOFS_ERROR_FILE_NOT_FOUND;
    } else
        returnValue = TECNICOFS_ERROR_MAXED_OPEN_FILES;
    return returnValue;
}

int serverClose(char *arg1, openedFile openedFiles[]) {
    int fd, returnValue = 0;
    fd = atoi(arg1);
    openedFile file;
    if((file = openedFiles[fd]) != NULL) {
        openedFiles[fd] = NULL;
    } else
        returnValue = TECNICOFS_ERROR_FILE_NOT_OPEN;
    return returnValue;
}

int serverRead(char *arg1, char *arg2, openedFile openedFiles[], int socket, char *buffer) {
    int fd, len, returnValue;
    char * fileToSend;
    fd = atoi(arg1);
    len = atoi(arg2);
    openedFile file;
    fileToSend = (char*)malloc(sizeof(char)*len);
    if((file = openedFiles[fd]) != NULL) {
        if (file->p != READ && file->p != RW)
            returnValue = TECNICOFS_ERROR_INVALID_MODE;
        else {
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
    return returnValue;
}

int serverWrite(char *arg1, char *arg2, openedFile openedFiles[]) {
    int returnValue, fd;
    fd = atoi(arg1);
    openedFile file;
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
    return returnValue;
}

void *give_receive_order(void *sockfd){
    int n, returnValue;
    unsigned int len;
    char token;
    char *buffer = NULL;
    FILE *stream;
    size_t max;
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    int socket = (intptr_t) sockfd;
    struct ucred ucred;
    
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
            close(socket);
            return NULL;
        }
        else if (n < 0)
            err_dump("give_receive_order: readline error");
        else {
            sscanf(buffer, "%c %s %s", &token, arg1, arg2);
            switch(token) {
                case 'c':
                    returnValue = serverCreate(arg1, arg2, ucred);
                    break;  
                case 'd':
                    returnValue = serverDelete(arg1, ucred);
                    break;
                case 'r':
                    returnValue = serverRename(arg1, arg2, ucred);
                    break;
                case 'o':
                    returnValue = serverOpen(arg1, arg2, openedFiles, ucred);
                    break;
                case 'x':
                    returnValue = serverClose(arg1, openedFiles);
                    break;
                case 'l':
                    returnValue = serverRead(arg1, arg2, openedFiles, socket, buffer);
                    break;
                case 'w':
                    returnValue = serverWrite(arg1, arg2, openedFiles);
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
    struct timeval start, end;
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
    gettimeofday(&start, NULL);
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
    gettimeofday(&end, NULL);
    close(sockfd);
    /*destroys the lockers*/
    destroy();
    /* instead of stdout, it is used a file as output */
    print_tecnicofs_tree(output, fs);
    execution_time(start, end);
    /*closing files*/
    fclose(output);
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}


