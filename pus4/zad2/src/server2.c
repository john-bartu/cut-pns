#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/sctp.h>
#include <time.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define OS 2
#define MIS 2
#define MAX_ATTEMPTS 5

int main(int argc, char **argv) {

    int listenfd, connfd;
    ssize_t bytes;
    struct sockaddr_in servaddr;
    struct sctp_initmsg initmsg;
    char buff[BUFF_SIZE];
    time_t t;
    struct tm tmp;

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

    memset(&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = OS;
    initmsg.sinit_max_instreams = MIS;
    initmsg.sinit_max_attempts = MAX_ATTEMPTS;
    setsockopt(listenfd, SOL_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

    for (;;) {
        printf("accepting connections...\n");
        connfd = accept(listenfd, NULL, 0);
        if (connfd == -1) {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        t = time(NULL);
        tmp = *localtime(&t);
        sprintf(buff, "Date: %d/%d/%d", tmp.tm_mday, tmp.tm_mon + 1, tmp.tm_year + 1900);
        bytes = sctp_sendmsg(connfd, buff, (size_t) strlen(buff),
                             NULL, 0,
                             0, 0, 0, 1, 0);
        if (bytes == -1) {
            perror("sctp_sendmsg()");
            exit(EXIT_FAILURE);
        }
        printf("\"%s\" sent\n", buff);

        strftime(buff, BUFF_SIZE, "Time: %H:%M", &tmp);
        bytes = sctp_sendmsg(connfd, buff, (size_t) strlen(buff),
                             NULL, 0,
                             0, 0, 1, 1, 0);
        if (bytes == -1) {
            perror("sctp_sendmsg()");
            exit(EXIT_FAILURE);
        }
        printf("\"%s\" sent\n\n", buff);
    }
}