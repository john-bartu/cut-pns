#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <netdb.h>

int main(int argc, char **argv) {

    int listenfd, connfd;
    struct sockaddr_in6 client_addr, server_addr;
    socklen_t client_addr_len, server_addr_len;
    char addr_buff[256];
    char buff[] = "Laboratorium PUS";

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listenfd = socket(PF_INET6, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(strtol(argv[1], NULL, 10));

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
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (connfd == -1) {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        fprintf(
                stdout, "TCP connection accepted from %s:%d - ",
                inet_ntop(AF_INET, &client_addr.sin6_addr, addr_buff, sizeof(addr_buff)),
                ntohs(client_addr.sin6_port)
        );

        if (IN6_IS_ADDR_V4MAPPED(&client_addr.sin6_addr)) {
            printf("IPv4 mapped to IPv6\n");
        } else {
            printf("IPv6\n");
        }

        write(connfd, buff, strlen(buff));
    }


//    retval = read(connfd, buff, sizeof(buff));
//    if (retval == 0) {
//        fprintf(stdout, "Connection terminated by client "
//                        "(received FIN, entering CLOSE_WAIT state on connected socked)...\n");
//    }

//    exit(EXIT_SUCCESS);
}
