#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>

int main(int argc, char **argv) {
    int sockfd;
    ssize_t retval;
    struct sockaddr_in addr4;
    socklen_t addr_len;
    char buff[256];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&addr4, 0, sizeof(addr4));
    addr4.sin_family = AF_INET;
    printf("family: %d\n", addr4.sin_family);

    retval = inet_pton(AF_INET, argv[1], &addr4.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    addr4.sin_port = htons(strtol(argv[2], NULL, 10)); /* Numer portu. */
    addr_len = sizeof(addr4); /* Rozmiar struktury adresowej w bajtach. */

    if (connect(sockfd, (const struct sockaddr *) &addr4, addr_len) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    memset(buff, 0, 256);
    read(sockfd, buff, sizeof(buff));
    fprintf(stdout, "Response: %s\n", buff);

    close(sockfd);
    exit(EXIT_SUCCESS);
}
