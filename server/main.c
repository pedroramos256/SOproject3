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

#define MAXLINE 512


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


void str_echo(int sockfd){
    int n;
    char line[MAXLINE];
    for (;;) {
        /* Lê uma linha do socket */
        n = read(sockfd,line, MAXLINE);
        if (n == 0)
            return;
        else if (n < 0)
            err_dump("str_echo: readline error");
        if (write(sockfd, line, n)!= n)
            err_dump("str_echo: write error");
    }
}


/******************************************************************************
 *                                  MAIN
 *****************************************************************************/


int main(int argc, char* argv[]) {
    struct timeval start, end;
    char *nomesocket;
    int sockfd, newsockfd, childpid, servlen;
    struct sockaddr_un cli_addr, serv_addr;
    unsigned int clilen;


    parseArgs(argc, argv);

    /*using files given in arguments*/
    nomesocket = (char *)malloc(sizeof(char)*strlen(argv[1]));
    strcpy(nomesocket,argv[1]);
    output = fopen(argv[2], "w");
    if (output == NULL) {   
        err_dump("output file open failure");
    }
    numberBuckets = atoi(argv[4]);
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
    unlink(UNIXSTR_PATH);

    /* O nome serve para que os clientes possam identificar o servidor */
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIXSTR_PATH);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
        err_dump("server, can't bind local address");

    listen(sockfd, 5);



    for (;;) {
        clilen = sizeof(cli_addr);
        newsockfd =accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            err_dump("server: accept error");
        /* Lança processo filho para tratar do cliente */
        if ((childpid =fork()) < 0)
            err_dump("server: fork error");
        else if (childpid == 0) {
            close(sockfd);
            str_echo(newsockfd);
            exit(0);
        }
        /* Processo pai. Fecha newsockfd que não utiliza */
        close(newsockfd);
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


