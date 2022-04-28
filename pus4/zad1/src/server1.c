#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 256

int main(int argc, char **argv) {

    int listenfd, connfd;
    ssize_t bytes;
    struct sockaddr_in servaddr;
    char buff[BUFF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT NUMBER>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(strtol(argv[1], NULL, 10));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 5) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    connfd = accept(listenfd, NULL, 0);
    if (connfd == -1) {
        perror("accept()");
        exit(EXIT_FAILURE);
    }

    for (;;) {

        bytes = recv(connfd, (void *) buff, BUFF_SIZE, 0);
        if (bytes == -1) {
            perror("recv() failed");
            exit(EXIT_FAILURE);
        } else if (bytes == 0) {
            close(connfd);
            close(listenfd);
            exit(EXIT_SUCCESS);
        }

        write(STDOUT_FILENO, buff, bytes);

        if (send(connfd, buff, bytes, 0) == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
        }
    }

}
