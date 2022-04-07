#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>

int input_is_empty(char *);
struct sockaddr_in get_remote_addr(char *, char*);

int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    char            buff_out[256];          /* Bufor dla funkcji recvfrom(). */
    char            buff_in[256];           /* Bufor dla funkcji recvfrom(). */

    if (argc != 3) {
        fprintf(
                stderr,
                "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]
        );
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    remote_addr = get_remote_addr(argv[1], argv[2]);

    int conn = connect(sockfd, (const struct sockaddr*) &remote_addr, sizeof(remote_addr));
    if (conn == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("Input: ");
        fgets(buff_out, sizeof(buff_out), stdin);
        strtok(buff_out, "\n");

        if (input_is_empty(buff_out)) {
            retval = send(sockfd, NULL, 0, 0);
            if (retval == -1) {
                perror("send()");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }

        printf("Sending message to %s.\nWaiting for server response...\n", argv[1]);
        retval = send(sockfd, buff_out, strlen(buff_out), 0);
        if (retval == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
        }

        retval = read(sockfd, buff_in, sizeof(buff_in));
        if (retval == -1) {
            perror("recvfrom()");
            exit(EXIT_FAILURE);
        }
        buff_in[retval] = '\0';
        printf("Server response: '%s'\n\n", buff_in);

        memset(buff_in, 0, 256);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}

int input_is_empty(char *input) {
    return strlen(input) == 1 && input[0] == '\n';
}

struct sockaddr_in get_remote_addr(char *ip_addr, char *port) {
    struct sockaddr_in remote_addr;
    int retval;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    retval = inet_pton(AF_INET, ip_addr, &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }
    remote_addr.sin_port = htons(atoi(port)); /* Numer portu. */

    return remote_addr;
}