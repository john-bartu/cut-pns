
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define OS 3
#define MIS 4
#define MAX_ATTEMPTS 5

int main(int argc, char **argv) {

    int sockfd;
    int retval;
    ssize_t bytes;
    struct addrinfo hints, *result;
    struct sctp_initmsg initmsg;
    struct sctp_status status;
    int status_len = sizeof(status);
    struct sctp_sndrcvinfo info;
    struct sctp_event_subscribe events;
    char buff[BUFF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IP ADDRESS> <PORT NUMBER>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    retval = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    if (result == NULL) {
        fprintf(stderr, "Could not connect!\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(result->ai_family, result->ai_socktype, IPPROTO_SCTP);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = OS;
    initmsg.sinit_max_instreams = MIS;
    initmsg.sinit_max_attempts = MAX_ATTEMPTS;
    setsockopt(sockfd, SOL_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    setsockopt(sockfd, SOL_SCTP, SCTP_EVENTS, &events, sizeof(events));

    getsockopt(sockfd, SOL_SCTP, SCTP_STATUS, &status, (socklen_t *) &status_len);
    printf("ID: %d\n", status.sstat_assoc_id);
    printf("state: %d\n", status.sstat_state);
    printf("instreams: %d\n", status.sstat_instrms);
    printf("ostreams: %d\n\n", status.sstat_outstrms);

    for (int i = 0; i < status.sstat_instrms; i++) {

        printf("receiving...\n");
        memset(buff, 0, BUFF_SIZE);
        bytes = sctp_recvmsg(sockfd, buff, BUFF_SIZE,
                             NULL, 0,
                             &info, 0);
        if (bytes == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }

        printf("> %s\n", buff);
    }

    printf("exiting\n");

    close(sockfd);
    exit(EXIT_SUCCESS);
}
