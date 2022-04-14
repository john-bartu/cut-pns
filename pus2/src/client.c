#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <unistd.h>     /* close() */
#include <string.h>
#include <netdb.h>

int main(int argc, char **argv) {
    int sockfd, retval;
    struct sockaddr_storage addr_storage;
    struct addrinfo hints, *result;
    char buff[256];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4/IPv6 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    if ((retval = getaddrinfo(argv[1], argv[2], &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }
    addr_storage.ss_family = result->ai_family;
    printf("family: %d\n", result->ai_family);

    if (result->ai_family != AF_INET && result->ai_family != AF_INET6) {
        printf("Invalid family\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd,  result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    socklen_t addr_storage_len = sizeof(addr_storage);
    getsockname(sockfd, (struct sockaddr*) &addr_storage, &addr_storage_len);

    char host[NI_MAXHOST], serv[NI_MAXSERV];
    getnameinfo((struct sockaddr*) &addr_storage, addr_storage_len, host, NI_MAXHOST, serv,
            NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);

    printf("Hostname: %s, server: %s\n", host, serv);

    memset(buff, 0, 256);
    read(sockfd, buff, sizeof(buff));
    printf("Response: %s\n", buff);

    close(sockfd);
    exit(EXIT_SUCCESS);
}
