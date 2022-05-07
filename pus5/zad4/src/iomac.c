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

    if (argc != 4) {
        fprintf(stderr,
                "Invocation: %s <Interface name> <MAC ADDRESS>, <MTU>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    /* Create socket with name from argument */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    strcpy(interface_request.ifr_name, argv[1]);

    printf("BEFORE: \n");
    printInterfaceInfo(sockfd, &interface_request);

    interface_request.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    /* Parse argument to MAC Address */
    retval = sscanf(
            argv[2],
            "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[0],
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[1],
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[2],
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[3],
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[4],
            (unsigned int *) &interface_request.ifr_hwaddr.sa_data[5]
    );
    if (retval != 6) {
        fprintf(stderr, "Invalid address format!\n");
        exit(EXIT_FAILURE);
    }

    /* Set MAC Address */
    retval = ioctl(sockfd, SIOCSIFHWADDR, &interface_request);
    if (retval == -1) {
        perror("MAC SAVE");
        exit(EXIT_FAILURE);
    }

    /* Set MTU */
    interface_request.ifr_mtu = atoi(argv[3]);
    retval = ioctl(sockfd, SIOCSIFMTU, &interface_request);
    if (retval == -1) {
        perror("MTU SAVE");
        exit(EXIT_FAILURE);
    }

    printf("AFTER: \n");
    printInterfaceInfo(sockfd, &interface_request);

    /* Set Exit */
    close(sockfd);
    exit(EXIT_SUCCESS);
}


void printInterfaceInfo(int sockfd, struct ifreq *interface_request) {
    int retval, i;
    u_int8_t macAddress[6];

    /* Get MTU */
    retval = ioctl(sockfd, SIOCGIFMTU, interface_request);
    if (retval == -1) {
        perror("MTU READ");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "\tMTU: %d \n", interface_request->ifr_mtu);

    /* Get MAC Address */
    retval = ioctl(sockfd, SIOCGIFHWADDR, interface_request);
    if (retval == -1) {
        perror("MAC READ");
        exit(EXIT_FAILURE);
    }

    /* Copy MAC Address */
    memcpy(
            macAddress,
            interface_request->ifr_hwaddr.sa_data,
            sizeof(macAddress)
    );

    fprintf(stdout, "\tMAC: ");

    for (i = 0; i < 6; i++) {
        fprintf(stdout, "%02X", macAddress[i]);
        if (i != 5) fprintf(stdout, ":");
    }

    printf("\n");
}