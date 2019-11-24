/* Cliente do tipo socket stream.*/
#include "tecnicofs-client-api.h"

#define MAXLINE 512

int sockfd = -1, servlen;
struct sockaddr_un serv_addr;

void err_dump(char * str){
    printf("%s\n",str);
    exit(EXIT_FAILURE);
}

/*Lê string de fp e envia para sockfd. Lê string de sockfd e envia para stdout*/
void str_cli(FILE *fp,int sockfd){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE+1];
    while(fgets(sendline, MAXLINE, fp) != NULL){
        /* Envia string para sockfd.Note-se que o \0 não é enviado */
        n = strlen(sendline);
        if (write(sockfd, sendline, n) != n)
            err_dump("str_cli:write error on socket");

        /* Tenta ler string de sockfd.Note-se que tem de terminar a string com \0 */
        n = read(sockfd, recvline, MAXLINE);
        if (n<0) err_dump("str_cli:readline error");
        recvline[n] = 0;
        /* Envia a string para stdout */
        fputs(recvline, stdout);
    }
    if (ferror(fp))
        err_dump("str_cli: error reading file");
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

    char *command;
    char permissions[2];
    int len = strlen(filename) + 6;
    command = (char*) malloc(sizeof(char) * len);
    strcpy(command, "c ");
    strcat(command, filename);
    strcat(command, " ");
    sprintf(permissions, "%d", ownerPermissions * 10 + othersPermissions);
    strcat(command, permissions);
    write(sockfd, command, len);
    return 0;
}

int tfsUnmount() {

    if(sockfd < 0) {
        printf("%d\n", sockfd);
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }

    if(close(sockfd) != 0)
        err_dump("tfsUnmount:failed to close socket");

    return 0;
}



