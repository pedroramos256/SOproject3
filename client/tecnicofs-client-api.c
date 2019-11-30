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
    /*
    if(read(sockfd, &returnValue, sizeof(int)) == TECNICOFS_ERROR_FILE_NOT_FOUND)
        err_dump("tfsDelete: file already exists");

    else if(read(sockfd, &returnValue, sizeof(int)) == TECNICOFS_ERROR_PERMISSION_DENIED)
        err_dump("tfsDelete: permission denied");*/
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
    int returnValue;
    char *command;

    int len = strlen(filename) + 5;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "o %s %d", filename, mode);
    write(sockfd, command, len);

    read(sockfd, &returnValue, sizeof(int));

    return returnValue; 
}

int tfsClose(int fd){
    int returnValue;
    char *command;
    char fdinString[12];//max size of an int in characters
    sprintf(fdinString,"%d",fd);
    int len = strlen(fdinString) + 3;
    command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "x %d", fd);
    
    write(sockfd, command, len);
   
    read(sockfd, &returnValue, sizeof(int));    

    return returnValue; 
}

int tfsRead(int fd, char *buffer, int len){
    int returnValue;
    char *command;
    char fdinString[12];//max size of an int in characters
    char leninString[12];//max size of an int in characters
    sprintf(fdinString,"%d",fd);
    sprintf(leninString,"%d",len);
    int commandlen = strlen(fdinString) + strlen(leninString) + 4;
    command = (char*) malloc(sizeof(char) * commandlen);
    sprintf(command, "l %d %d", fd, len);

    write(sockfd, command, commandlen);

    read(sockfd,buffer,len+1);
    write(sockfd,'\0',0);
    read(sockfd, &returnValue, sizeof(int)); 
    printf("%d\n",returnValue);
    return returnValue;
}


int tfsWrite(int fd,char *buffer,int len){
    int returnValue;
    char *command;
    char fdinString[12];//max size of an int in characters
    sprintf(fdinString,"%d",fd);
    int commandlen = strlen(fdinString) + 4;
    command = (char*) malloc(sizeof(char) * commandlen);

    if(strlen(buffer) > len){
        commandlen += len;
    } else{
        commandlen += strlen(buffer);
    }

    sprintf(command, "w %d %s", fd, buffer);

    write(sockfd, command, commandlen);

    read(sockfd,&returnValue,sizeof(int));

    return returnValue;
}



int tfsUnmount() {
    int n;
    if(sockfd < 0) 
        return TECNICOFS_ERROR_NO_OPEN_SESSION;

    if((n = close(sockfd)) != 0)
        return TECNICOFS_ERROR_OTHER;

    return n;
}



