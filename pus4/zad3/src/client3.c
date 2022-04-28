#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define OS 5
#define MIS 5
#define MAX_ATTEMPTS 5

int main(int argc, char **argv) {

    int sockfd;
    int retval, stream_no = 0;
    ssize_t bytes;
    char *retptr;
    struct addrinfo hints, *result;
    struct sctp_initmsg initmsg;
    struct sctp_sndrcvinfo info;
    struct sctp_event_subscribe events;
    char buff[BUFF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IP ADDRESS> <PORT NUMBER>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
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
    retval = setsockopt(sockfd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
    if (retval != 0) {
        perror("setsockopt()");;
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    setsockopt(sockfd, SOL_SCTP, SCTP_EVENTS, &events, sizeof(events));

    for (;;) {
        memset(buff, 0, BUFF_SIZE);
        retptr = fgets(buff, BUFF_SIZE, stdin);
        if ((retptr == NULL) || (strcmp(buff, "\n") == 0)) {
            break;
        }

        bytes = sctp_sendmsg(sockfd, buff, (size_t) strlen(buff),
                             NULL, 0,
                             0, 0, stream_no, 1, 0);

        printf("SENT] %sstream_no: %d\n\n", buff, stream_no);

        if (bytes == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
        }

        memset(buff, 0, BUFF_SIZE);
        bytes = sctp_recvmsg(sockfd, buff, BUFF_SIZE,
                             NULL, 0,
                             &info, 0);
        if (bytes == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }

        printf("RESPONSE] ");
        fflush(stdout);
        write(STDOUT_FILENO, buff, bytes);

        printf("stream_no: %d\n", info.sinfo_stream);
        printf("ID: %d\n", info.sinfo_assoc_id);
        printf("SSN: %d\n\n", info.sinfo_ssn);

        stream_no = info.sinfo_stream;
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
