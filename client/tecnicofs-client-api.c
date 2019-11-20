/* Cliente do tipo socket stream.*/
#include "unix.h"

#include <stdio.h>
#define MAXLINE 512
/*Lê string de fp e envia para sockfd. Lê string de sockfd e envia para stdout*/
str_cli(FILE *fp,int sockfd){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE+1];
    while(fgets(sendline, MAXLINE, fp) != NULL){
        /* Envia string para sockfd.Note-se que o \0 não é enviado */
        n = strlen(sendline);
        if (write(sockfd, sendline, n) != n)
            err_dump("str_cli:write error on socket");
            /* Tenta ler string de sockfd.Note-se que tem de terminar a string com \0 */
            n = readline(sockfd, recvline, MAXLINE);
            if (n<0) err_dump("str_cli:readline error");
            recvline[n] = 0;
            /* Envia a string para stdout */
            fputs(recvline, stdout);
        }
    if (ferror(fp))
        err_dump("str_cli: error reading file");
}


main(void) {
    int sockfd, servlen;
    struct sockaddr_un serv_addr;
    /* Cria socket stream */
    if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
        err_dump("client: can't open stream socket");
    /* Primeiro uma limpeza preventiva */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* Dados para o socket stream: tipo + nome queidentifica o servidor */
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIXSTR_PATH);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);


    /* Estabelece uma ligação. Só funciona se o socket tiver sido criado eo nome associado*/
    if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen)<0)
    err_dump("client: can't connect to server");
    /* Envia as linhas lidas do teclado para o socket */
    str_cli(stdin, sockfd);
    /* Fecha o socket e termina */
    close(sockfd);
    exit(0);
}