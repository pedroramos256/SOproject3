/* Cliente do tipo socket stream.*/
#include "tecnicofs-client-api.h"

#define MAXLINE 512

int sockfd = -1;

void err_dump(char * str){
    printf("%s\n",str);
    exit(EXIT_FAILURE);
}

int tfsMount(char *adress){
    struct sockaddr_un serv_addr;
    int servlen;
    char *total_path;
    total_path = (char*)malloc(sizeof(char)*(strlen(adress)+5));
    strcpy(total_path,"/tmp/");
    strcat(total_path,adress);
    /* Cria socket stream */

    if(sockfd > 0)
        return TECNICOFS_ERROR_OPEN_SESSION;
        
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


    /* Estabelece uma ligação. Só funciona se o socket tiver sido criado e o nome associado*/
    if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen)<0){
        err_dump("tfsMount: can't connect to server");
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    }
    return 0;
}


int tfsComunicate(char *command,int len){
    int returnValue;

    write(sockfd, command, len);

    read(sockfd, &returnValue, sizeof(int));
    return returnValue;
}


int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
    int len = strlen(filename) + 6;
    char *command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "c %s %d%d", filename, ownerPermissions, othersPermissions);
  
    return tfsComunicate(command,len);
}

int tfsDelete(char *filename) {
    int len = strlen(filename) + 3;
    char *command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "d %s", filename);

    return tfsComunicate(command,len);
}

int tfsRename(char *filenameOld, char *filenameNew) {
    int len = strlen(filenameOld) + strlen(filenameNew) + 4;
    char *command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "r %s %s", filenameOld, filenameNew);

    return tfsComunicate(command,len);
}

int tfsOpen(char *filename,permission mode){
    int len = strlen(filename) + 5;
    char *command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "o %s %d", filename, mode);
  
    return tfsComunicate(command,len);
}

int tfsClose(int fd){
    char fdinString[12];//max size of an int in characters
    sprintf(fdinString,"%d",fd);
    int len = strlen(fdinString) + 3;
    char *command = (char*) malloc(sizeof(char) * len);
    
    sprintf(command, "x %d", fd);
   
    return tfsComunicate(command,len);
}

int tfsRead(int fd, char *buffer, int len){
    char string[12*2+1];//max size of two int's in characters
    sprintf(string,"%d %d",fd,len);
    int commandlen = strlen(string) + 3;
    char *command = (char*) malloc(sizeof(char) * commandlen);
    sprintf(command, "l %d %d", fd, len);

    write(sockfd, command, commandlen);
    read(sockfd,buffer,len);
    
    return tfsComunicate('\0',0);
}


int tfsWrite(int fd,char *buffer,int len){
    char fdinString[12];//max size of an int in characters
    sprintf(fdinString,"%d",fd);
    int commandlen = strlen(fdinString) + 4;

    if(strlen(buffer) > len) {
        commandlen += len;
    } else
        commandlen += strlen(buffer);

    char *command = (char*) malloc(sizeof(char) * commandlen);
    
    sprintf(command, "w %d ", fd);
    strncat(command,buffer,len);

    return tfsComunicate(command,commandlen);
}

int tfsUnmount() {
    int n;
    if(sockfd < 0) 
        return TECNICOFS_ERROR_NO_OPEN_SESSION;

    if((n = close(sockfd)) != 0)
        return TECNICOFS_ERROR_OTHER;

    sockfd = -1;
    return n;
}



