#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h> /* struct ifconf, struct ifreq */
#include <unistd.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>

void printInterfaceInfo(int sockfd, struct ifreq *interface_request);

int main(int argc, char **argv) {
    struct ifreq interface_request;
    int sockfd;
    int retval;

    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Invocation:\n\t%s <Interface name> <add> <ipv4> <mask>\n", argv[0]);
        fprintf(stderr, "\t%s <Interface name> <down>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char name[255];
    char command[255];
    sscanf(argv[1], "%s", name);
    sscanf(argv[2], "%s", command);

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    strcpy(interface_request.ifr_name, argv[1]);
    retval = ioctl(sockfd, SIOCGIFMTU & SIOCSIFHWADDR, interface_request);
    if (retval == -1) {
        perror("INT ERROR");
        exit(EXIT_FAILURE);
    }

    if (strcmp(command, "add") == 0) {

        if (argc != 5) {
            fprintf(stderr, "Invocation: %s <Interface name> <MAC ADDRESS> <add> <ipv4> <mask>\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        printf("add");

    } else if (strcmp(command, "down") == 0) {

        printf("down");

    } else {
        fprintf(stderr, "command should be add|down\n");
        printf("Failure: %s\n", command);
        exit(EXIT_FAILURE);
    }

    /* Set Exit */
    close(sockfd);
    exit(EXIT_SUCCESS);
}


