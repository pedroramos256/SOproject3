/*
*   File System made by
*       Pedro Ramos 92539 and Sancha Barroso 92557
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>

#include "fs.h"
#include "sync.h"
#include "../unix.h"

#include <sys/time.h>

#define MAXLINE     512
#define MAXCLIENTS  10


tecnicofs* fs;                  


FILE *output;

/******************************************************************************
 *                         FUNCTIONS USED IN MAIN
 *****************************************************************************/


void err_dump(char * str){
    printf("%s\n",str);
    exit(EXIT_FAILURE);
}

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
    int n, numTokens;
    char token;
    char line[MAXLINE];
    char arg1[MAXLINE];
    char arg2[MAXLINE];
    int socket = (intptr_t) sockfd;
    int iNumber;
    for (;;) {
        /* Lê uma linha do socket */
        n = read(socket, line, MAXLINE);
        if (n == 0) {
            printf("socket closed\n");
            close(socket);
            return NULL;
        }
        else if (n < 0)
            err_dump("give_receive_order: readline error");
        else {
            printf("%s\n", line);
            numTokens = sscanf(line, "%c %s %s", &token, arg1, arg2); 
            switch(token) {
                case 'c':
                    LOCK();
                    iNumber = obtainNewInumber(fs);
                    UNLOCK();
                    create(fs, arg1, iNumber);
                    break;
            }
        }
    }
}


/******************************************************************************
 *                                  MAIN
 *****************************************************************************/


int main(int argc, char* argv[]) {
    int num_clients = 0;
    struct timeval start, end;
    char *total_path;
    int sockfd, newsockfd, servlen;
    struct sockaddr_un cli_addr, serv_addr;
    pthread_t tid[MAXCLIENTS];
    unsigned int clilen;

    parseArgs(argc, argv);

    /*using files given in arguments*/
    total_path = (char *)malloc(sizeof(char)*(strlen(argv[1])+5));
    strcpy(total_path,"/tmp/");
    strcat(total_path,argv[1]);
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
    
    /*counting just the execution time*/
    gettimeofday(&start, NULL);

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

    listen(sockfd, MAXCLIENTS);

    for (;;) {
        clilen = sizeof(cli_addr);
        newsockfd =accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            err_dump("server: accept error");
        if(pthread_create(&tid[num_clients], NULL, give_receive_order, (void *)(intptr_t)newsockfd) != 0)
            err_dump("server: thread creation error");
        else {
            num_clients++;
            printf("%d\n", num_clients);
        }

    }
    
    gettimeofday(&end, NULL);

    /*destroys the lockers*/
    destroy();

    /* instead of stdout, it is used a file as output */
    print_tecnicofs_tree(output, fs);

    /*closing files*/
    fclose(output);

    execution_time(start, end);

    free_tecnicofs(fs);

    exit(EXIT_SUCCESS);
}


