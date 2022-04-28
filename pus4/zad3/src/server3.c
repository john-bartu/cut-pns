#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define OS 5
#define MIS 4
#define MAX_ATTEMPTS 5

int main(int argc, char **argv) {

    short inc_mode;
    int listenfd, stream_no;
    ssize_t bytes;
    struct sockaddr_in servaddr, clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    struct sctp_initmsg initmsg;
    struct sctp_sndrcvinfo info;
    struct sctp_event_subscribe events;
    char buff[BUFF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <PORT NUMBER> <MODE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    inc_mode = (short) strtol(argv[2], NULL, 10);

    listenfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
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

    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    setsockopt(listenfd, SOL_SCTP, SCTP_EVENTS, &events, sizeof(events));

    for (;;) {
        memset(buff, 0, BUFF_SIZE);
        bytes = sctp_recvmsg(listenfd, buff, BUFF_SIZE,
                             (struct sockaddr *) &clientaddr, &addrlen,
                             &info, 0);
        if (bytes == -1) {
            perror("sctp_recvmsg() failed");
            exit(EXIT_FAILURE);
        } else if (bytes == 0) {
            close(listenfd);
            exit(EXIT_SUCCESS);
        }

        write(STDOUT_FILENO, buff, bytes);

        if (inc_mode == 0) {
            stream_no = info.sinfo_stream;
        } else {
            stream_no = info.sinfo_stream + 1 < MIS ? info.sinfo_stream + 1 : 0;
        }

        if (sctp_sendmsg(listenfd, buff, bytes,
                         (struct sockaddr *) &clientaddr, addrlen,
                         info.sinfo_ppid, info.sinfo_flags,
                         stream_no,
                         info.sinfo_timetolive, info.sinfo_context) == -1) {
            perror("sctp_sendmsg()");
            exit(EXIT_FAILURE);
        }
    }

}
