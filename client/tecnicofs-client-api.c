/* Cliente do tipo socket stream.*/
#include "tecnicofs-client-api.h"

#define MAXLINE 512

int sockfd = -1;//initialization to identify that sockfd is not Mounted

/* Creates socket stream */
int tfsMount(char *adress){
    struct sockaddr_un serv_addr;
    int servlen;
    char *total_path;
    total_path = (char*)malloc(sizeof(char)*(strlen(adress)+5));
    if(!total_path)
        perror("tfsMount: failed to allocate");
    strcpy(total_path,"/tmp/");
    strcat(total_path,adress);


    if(sockfd > 0)
        return TECNICOFS_ERROR_OPEN_SESSION;
        
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    
    /* Cleaning up serv_addr */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* Data to serv_addr structure */
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, total_path);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);


    /* Connects the socket */
    if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen)<0)
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    
    return 0;
}

/* Aux function that does the comunication between client and server */
int tfsComunicate(char *command,int len){
    int returnValue;

    if((write(sockfd, command, len)) != len)
        perror("tfsComunicate: failed to write");

    if((read(sockfd, &returnValue, sizeof(int))) < 0)
        perror("tfsComunicate: failed to write");
    return returnValue;
}

/* Create a new file */
int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
    int len = strlen(filename) + 6;/* 6 is counting with spaces and \0 in the end */
    char *command = (char*) malloc(sizeof(char) * len);
    if(!command)
        perror("tfsCreate: failed to allocate");
    sprintf(command, "c %s %d%d", filename, ownerPermissions, othersPermissions);
  
    return tfsComunicate(command,len);
}

/* Deletes an existing file */
int tfsDelete(char *filename) {
    int len = strlen(filename) + 3;
    char *command = (char*) malloc(sizeof(char) * len);
    if(!command)
        perror("tfsDelete: failed to allocate");
    sprintf(command, "d %s", filename);

    return tfsComunicate(command,len);
}

/* Renames an existing file */
int tfsRename(char *filenameOld, char *filenameNew) {
    int len = strlen(filenameOld) + strlen(filenameNew) + 4;
    char *command = (char*) malloc(sizeof(char) * len);
    sprintf(command, "r %s %s", filenameOld, filenameNew);
    if(!command)
        perror("tfsRename: failed to allocate");
    return tfsComunicate(command,len);
}

/* Opens a file to write and/or read */
int tfsOpen(char *filename,permission mode){
    int len = strlen(filename) + 5;
    char *command = (char*) malloc(sizeof(char) * len);
    if(!command)
        perror("tfsOpen: failed to allocate");
    sprintf(command, "o %s %d", filename, mode);
  
    return tfsComunicate(command,len);
}

/* Closes an opened file */
int tfsClose(int fd){
    char fdinString[12];/* max size of an int in characters */
    sprintf(fdinString,"%d",fd);
    int len = strlen(fdinString) + 3;
    char *command = (char*) malloc(sizeof(char) * len);
    if(!command)
        perror("tfsClose: failed to allocate");
    
    sprintf(command, "x %d", fd);
   
    return tfsComunicate(command,len);
}

/* Reads an opened file */
int tfsRead(int fd, char *buffer, int len){
    int returnValue;
    char string[12*2+1];/* max size of two int's in characters */
    sprintf(string,"%d %d",fd,len);
    int commandlen = strlen(string) + 3;
    char *command = (char*) malloc(sizeof(char) * commandlen);
    if(!command)
        perror("tfsRead: failed to allocate");
    sprintf(command, "l %d %d", fd, len);

    /* first comunication to receive the error value */
    if((returnValue = tfsComunicate(command,commandlen)) >= 0)
        if((read(sockfd,buffer,len)) < 0) /* receives buffer */
            perror("tfsRead: failed to read");

    return returnValue;/* comunicates nothing just to wait for the return value*/
}

/* Writes an opened file */
int tfsWrite(int fd,char *buffer,int len){
    char fdinString[12];
    sprintf(fdinString,"%d",fd);
    int commandlen = strlen(fdinString) + 4;

    if(strlen(buffer) > len) {
        commandlen += len;
    } else
        commandlen += strlen(buffer);

    char *command = (char*) malloc(sizeof(char) * commandlen);
    if(!command)
        perror("tfsWrite: failed to allocate");
    
    sprintf(command, "w %d ", fd);
    strncat(command,buffer,len);/* max size of buffer writen is len */

    return tfsComunicate(command,commandlen);
}

int tfsUnmount() {
    int n;
    if(sockfd < 0) 
        return TECNICOFS_ERROR_NO_OPEN_SESSION;

    if((n = close(sockfd)) != 0)
        return TECNICOFS_ERROR_OTHER;

    sockfd = -1;/* indicates that the socket is closed */
    return n;
}
