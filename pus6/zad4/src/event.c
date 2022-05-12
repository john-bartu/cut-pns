#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <net/if.h>

/* Struktura przechowujaca informacje na temat interfejsu: */
struct interface {
    char name[IF_NAMESIZE];
    int index;
    unsigned char prefix;
    char address[INET6_ADDRSTRLEN];
    char broadcast[INET6_ADDRSTRLEN];
    char *scope;
};

#define BUFF_SIZE 16384

void attributeParser(struct rtattr *table[], int max, struct rtattr *received, int len) {
    memset(table, 0, sizeof(struct rtattr *) * (max + 1));

    while (RTA_OK(received, len)) {  // czytaj dopoki jest attrybut
        if (received->rta_type <= max) {
            table[received->rta_type] = received;
        }
        received = RTA_NEXT(received, len);    // nastepny
    }
}

int main(int argc, char **argv) {

    int sockfd;
    int retval;
    char *recv_buff;
    struct msghdr msg;    /* Struktura dla sendmsg() i recvmsg() */
    struct iovec iov;    /* Dla msghdr */
    struct nlmsghdr *nh_tmp;
    struct sockaddr_nl sa;     /* Struktura adresowa */
    struct ifaddrmsg *ia;    /* Struktura zawierajaca inform. adresowe */
    struct rtattr *attr;  /* Opcjonalne atrybuty */
    unsigned int attr_len;
    unsigned int sequence_number;
    unsigned int msg_len;
    unsigned char family; /* AF_INET or AF_INET6 */
    struct interface in_info;

    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <IP VERSION>\n"
                        "<IP VERSION> = 4 or 6\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    family = atoi(argv[1]);
    if (family == 4) {
        family = AF_INET;
    } else if (family == 6) {
        family = AF_INET6;
    } else {
        fprintf(stderr, "<IP VERSION> must be 4 or 6!\n");
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda: */
    sockfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    /*
     * Gniazdo surowe z reguly wymaga, aby proces byl uprzywilejowany (EUID == 0)
     * lub posiadal CAP_NET_RAW capability. W przypadku PF_NETLINK nie jest to
     * konieczne.
     */

    msg.msg_name = (void *) &sa;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;


    /* Wyzerowanie struktury adresowej.
     * Jadro jest odpowiedzialne za ustalenie identyfikatora gniazda.
     * Ponieważ jest to pierwsze (i jedyne) gniazdo procesu,
     * identyfikatorem będzie PID procesu. */
    memset(&sa, 0, sizeof(struct sockaddr_nl));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
    sa.nl_pid = getpid();

    if (bind(sockfd, (struct sockaddr *) &sa, sizeof(struct sockaddr_nl)) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    /* Alokacja pamieci buforu dla danych odbieranych: */
    recv_buff = malloc(sizeof(char) * BUFF_SIZE);
    if (recv_buff == NULL) {
        fprintf(stderr, "malloc() failed!\n");
        exit(EXIT_FAILURE);
    }

    iov.iov_base = (void *) recv_buff;
    iov.iov_len = BUFF_SIZE;

    /* Odebranie odpowiedzi: */
    msg_len = 0;
    while (1) {
        ssize_t status = recvmsg(sockfd, &msg, 0);

        if (status < 0) {
            perror("recvmsg()");
            exit(EXIT_FAILURE);
        }

        /* Odpowiedz moze zawierac wiecej niz jeden naglowek. Iteracja: */
        struct nlmsghdr *nh;

        for (nh = (struct nlmsghdr *) recv_buff; status >= (ssize_t) sizeof(*nh);) {
            int len = nh->nlmsg_len;
            int l = len - sizeof(*nh);

            if ((l < 0) || (len > status)) {
                printf("Invalid message length: %i\n", len);
                continue;
            }


            if (nh->nlmsg_type == RTM_NEWROUTE) { // dodanie nowej drogi
                printf("Routing table: new route add\n");
            } else if (nh->nlmsg_type == RTM_DELROUTE) { // usunięcie drogi
                printf("Routing table: route deleted\n");
            } else {
                char *ifName;
                char *ifUpp;
                char *ifRunn;
                uint32_t newMtu;
                char *mac_addr;
                char address[255];
                struct ifinfomsg *ifi;
                struct rtattr *ifla_attr[IFLA_MAX + 1];


                ifi = (struct ifinfomsg *) NLMSG_DATA(nh);
                attributeParser(ifla_attr, IFLA_MAX, IFLA_RTA(ifi), nh->nlmsg_len);  // pobranie wszystkich atrybutów

                if (ifla_attr[IFLA_IFNAME]) {
                    ifName = (char *) RTA_DATA(ifla_attr[IFLA_IFNAME]); // sprawdzenie nazwy interfejsu
                }

                if (ifi->ifi_flags & IFF_UP) { // Sprawdzenie flagi UP na interfejsie
                    ifUpp = (char *) "UP";
                } else {
                    ifUpp = (char *) "DOWN";
                }

                if (ifi->ifi_flags & IFF_RUNNING) { //  Sprawdzenie flagi RUNNING na interfejsie
                    ifRunn = (char *) "RUNNING";
                } else {
                    ifRunn = (char *) "STOPPED";
                }

                memset(&in_info, 0, sizeof(struct interface));

                // Wiadomosc interfejsu
                struct rtattr *ifa_attr[IFA_MAX + 1];
                ia = (struct ifaddrmsg *) NLMSG_DATA(nh);
                attributeParser(ifa_attr, IFA_MAX, IFA_RTA(ia), nh->nlmsg_len);

                /* Odczyt IF Name */
                if (ifla_attr[IFLA_IFNAME]) {
                    strcpy(in_info.name, (char *) RTA_DATA(ifla_attr[IFLA_IFNAME]));
                }

                /* Odczyt MTU */
                if (ifla_attr[IFLA_MTU]) {
                    newMtu = *(uint32_t *) RTA_DATA (ifla_attr[IFLA_MTU]);
                }

                /* Odczyt MAC */
                if (ifla_attr[IFLA_ADDRESS]) {
                    mac_addr = (char *) RTA_DATA(ifla_attr[IFLA_ADDRESS]);
                }

                /* W przypadku IPv6 w in_info.name zapamietujemy nazwe interfejsu: */
                if (ia->ifa_family == AF_INET6) {
                    if (if_indextoname(ia->ifa_index, in_info.name) == NULL) {
                        perror("if_indextoname()");
                        exit(EXIT_FAILURE);
                    }
                }

                in_info.index = ia->ifa_index;
                in_info.prefix = ia->ifa_prefixlen;

                /* Naglowek pierwszego atrybutu: */
                attr = (struct rtattr *) ((char *) ia + NLMSG_ALIGN(sizeof(struct ifaddrmsg)));

                attr_len = NLMSG_PAYLOAD(nh, sizeof(struct ifaddrmsg));

                /* Sprawdzanie istnienie typu atrybutu i przypisanie do pól */
                if (ifa_attr[IFA_LOCAL]) {
                    strcpy(in_info.name, (char *) RTA_DATA(ifa_attr[IFLA_IFNAME]));
                }

                if (ifa_attr[IFA_LABEL]) {
                    strcpy(in_info.name, (char *) RTA_DATA(ifa_attr[IFA_LABEL]));
                }

                if (ifa_attr[IFA_ADDRESS]) {
                    inet_ntop(family, RTA_DATA(ifa_attr[IFA_ADDRESS]), in_info.address, INET6_ADDRSTRLEN);
                }

                if (ifa_attr[IFA_BROADCAST]) {
                    inet_ntop(family, RTA_DATA(ifa_attr[IFA_BROADCAST]), in_info.broadcast, INET6_ADDRSTRLEN);
                }

                switch (ia->ifa_scope) {
                    case RT_SCOPE_UNIVERSE:
                        in_info.scope = "global";
                        break;
                    case RT_SCOPE_SITE:
                        in_info.scope = "site";
                        break;
                    case RT_SCOPE_HOST:
                        in_info.scope = "host";
                        break;
                    case RT_SCOPE_LINK:
                        in_info.scope = "link";
                        break;
                    case RT_SCOPE_NOWHERE:
                        in_info.scope = "nowhere";
                        break;
                    default:
                        in_info.scope = "unknown";
                }

                switch (nh->nlmsg_type) {
                    case RTM_DELADDR:
                        printf("Interface %s: del address %s, scope %s\n",
                               in_info.name, in_info.address, in_info.scope);
                        break;

                    case RTM_NEWADDR:
                        if (ifa_attr[IFA_BROADCAST])
                            printf("Interface %s: add address: %s, scope %s, broadcast %s\n",
                                   in_info.name, in_info.address, in_info.scope, in_info.broadcast);
                        else
                            printf("Interface %s: add address: %s, scope %s\n",
                                   in_info.name, in_info.address, in_info.scope);
                        break;

                    case RTM_DELLINK:
                        printf("Network interface %s was removed\n", in_info.name);
                        break;

                    case RTM_NEWLINK:
                        printf("Network interface %s, state: %s %s\n", in_info.name, ifUpp, ifRunn);
                        printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                               mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
                        printf("MTU: %u\n", *(uint32_t *) RTA_DATA (ifla_attr[IFLA_MTU]));
                        break;

                    default:
                        printf("Unhandled event id: %d\n", nh->nlmsg_type);
                }
            } /* while */


            status -= NLMSG_ALIGN(len);
            nh = (struct nlmsghdr *) ((char *) nh + NLMSG_ALIGN(len));    // nastepny komunikat
        }

        memset(&in_info, 0, sizeof(struct interface));

        printf("\n");
        fflush(stdout);
        sleep(1);
    }

    free(recv_buff);
    close(sockfd);
    exit(EXIT_SUCCESS);
}