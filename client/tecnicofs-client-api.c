/* Cliente do tipo socket stream.*/
#include "tecnicofs-client-api.h"

#define MAXLINE 512

int sockfd = -1, servlen;
struct sockaddr_un serv_addr;

void err_dump(char * str){
    printf("%s\n",str);
    exit(EXIT_FAILURE);
}

int tfsMount(char *adress){
    char *total_path;
    total_path = (char*)malloc(sizeof(char)*(strlen(adress)+5));
    strcpy(total_path,"/tmp/");
    strcat(total_path,adress);
    /* Cria socket stream */
    if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0) ) < 0){
        err_dump("tfsMount: can't open stream socket");
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    }
    /* Primeiro uma limpeza preventiva */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* Dados para o socket stream: tipo + nome queidentifica o servidor */
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, total_path);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);


    /* Estabelece uma ligação. Só funciona se o socket tiver sido criado eo nome associado*/
    if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen)<0){
        err_dump("tfsMount: can't connect to server");
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    }
    return 0;
}

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {

    int returnValue;
    char *command;
    int len = strlen(filename) + 6;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "c %s %d%d", filename, ownerPermissions, othersPermissions);
    write(sockfd, command, len);

    if(read(sockfd, &returnValue, sizeof(int)) == TECNICOFS_ERROR_FILE_ALREADY_EXISTS)
        err_dump("tfsCreate: file already exists");

    return returnValue;
}

int tfsDelete(char *filename) {

    int returnValue;
    char * command;
    int len = strlen(filename) + 3;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "d %s", filename);

    write(sockfd, command, len);

    read(sockfd, &returnValue, sizeof(int));

    return returnValue;
}

int tfsRename(char *filenameOld, char *filenameNew) {

    int returnValue;
    char * command;
    int len = strlen(filenameOld) + strlen(filenameNew) + 4;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "r %s %s", filenameOld, filenameNew);

    write(sockfd, command, len);

    read(sockfd, &returnValue, sizeof(int));

    return returnValue; 
}

int tfsOpen(char *filename,permission mode){
    int returnValue,n;
    char *command;
    char *buffer = NULL;
    FILE *stream;
    size_t max;

    int len = strlen(filename) + 5;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "o %s %d", filename, mode);
    write(sockfd, command, len);

    if((stream = fdopen(sockfd, "r")) == NULL)
        err_dump("give_receive_order: fdopen failure");
    n = getdelim(&buffer, &max, '\0', stream);
    /* Lê uma linha do socket */
    if (n < 0)
        err_dump("give_receive_order: readline error");
    
    read(sockfd, &returnValue, sizeof(int));

    return returnValue; 
}

int tfsUnmount() {
    int n;
    if(sockfd < 0) {
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }

    if((n = close(sockfd)) != 0)
        err_dump("tfsUnmount:failed to close socket");

    return n;
}



