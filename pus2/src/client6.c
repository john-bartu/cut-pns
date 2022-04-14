#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <net/if.h>

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in6 addr6;
    char buff[256], ipv6[16];

    if (argc != 4) {
        fprintf(stderr, "Invocation: %s <IPv6 ADDRESS> <PORT> <INTERFACE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(ipv6, argv[1]);

    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_scope_id = if_nametoindex(argv[3]);
    addr6.sin6_port = htons(strtol(argv[2], NULL, 10));
    printf("family: %d\n", addr6.sin6_family);

    if (inet_pton(AF_INET6, ipv6, &addr6.sin6_addr) == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (const struct sockaddr *) &addr6, sizeof(addr6)) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    memset(buff, 0, 256);
    read(sockfd, buff, sizeof(buff));
    printf("Response: %s\n", buff);

    close(sockfd);
    exit(EXIT_SUCCESS);
}

