#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in client_addr, server_addr;
    socklen_t client_addr_len, server_addr_len;
    char addr_buff[256];
    char buff[] = "Laboratorium PUS";

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(strtol(argv[1], NULL, 10));
    server_addr_len = sizeof(server_addr);

    if (bind(listenfd, (struct sockaddr *) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 2) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

    while (1) {
        client_addr_len = sizeof(client_addr);
        printf("Accepting...\n");
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (connfd == -1) {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        printf("Accepted\n");

        fprintf(
                stdout, "TCP connection accepted from %s:%d\n",
                inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
                ntohs(client_addr.sin_port)
        );


        write(connfd, buff, strlen(buff));
    }
}
