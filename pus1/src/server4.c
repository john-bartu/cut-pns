#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc, char **argv) {

    int i, rc, desc_ready, close_conn, len, end_server = 0, on = 1;
    int sock_d, new_d, max_d; /* Deskryptor gniazda. */
    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct sockaddr_in6 addr;
    char buffer[80];
    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char addr_buff[256];
    fd_set master_set, working_set;

    char *server_port = argv[1];


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sock_d = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock_d == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    rc = setsockopt(sock_d, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(sock_d);
        exit(-1);
    }

    rc = ioctl(sock_d, FIONBIO, (char *) &on);
    if (rc < 0) {
        perror("ioctl() failed");
        close(sock_d);
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(atoi(server_port));
    rc = bind(sock_d,
              (struct sockaddr *) &addr, sizeof(addr));
    if (rc < 0) {
        perror("bind() failed");
        close(sock_d);
        exit(-1);
    }

    rc = listen(sock_d, 32);
    if (rc < 0) {
        perror("listen() failed");
        close(sock_d);
        exit(-1);
    }

    FD_ZERO(&master_set);
    max_d = sock_d;
    FD_SET(sock_d, &master_set);

    struct timeval timeout;
    timeout.tv_sec = 3 * 60;
    timeout.tv_usec = 0;

    do {
        memcpy(&working_set, &master_set, sizeof(master_set));

        printf("Waiting on select()...\n");
        rc = select(max_d + 1, &working_set, NULL, NULL, &timeout);
        if (rc < 0) {
            perror("  select() failed");
            break;
        } else if (rc == 0) {
            printf("  select() timed out.  End program.\n");
            break;
        }

        fprintf(stdout, "%s:%d connected\n",
                inet_ntop(AF_INET, &addr.sin6_addr, addr_buff, sizeof(addr_buff)),
                ntohs(addr.sin6_port)
        );

        desc_ready = rc;
        for (i = 0; i <= max_d && desc_ready > 0; ++i) {
            if (FD_ISSET(i, &working_set)) {
                desc_ready -= 1;

                if (i == sock_d) {
                    printf("  Listening socket is readable\n");
                    
                    //Trzeba zaakceptowa?? nadoch??dze po????czenia,
                    // kt??re s?? w kolejce, zanim b??dziemy rozsy??a?? wiadomo??ci
                    do {


                        //Akceptujemy nadchodz??ce po????czenie
                        new_d = accept(sock_d, NULL, NULL);
                        if (new_d < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("  accept() failed");
                                end_server = 1;
                            }
                            break;
                        }

                        // Dodajemy po????dzenie do zestawu
                        printf("  New incoming connection - %d\n", new_d);
                        FD_SET(new_d, &master_set);
                        if (new_d > max_d)
                            max_d = new_d;

                        // Sprawdzamy czy jeszcze kto?? czeka na po????czenie
                    } while (new_d != -1);
                }

                    // Je??eli to nie socket nas??ucuj??cy,
                    // to musi by?? to by?? istniej??ce po????czenie
                    // wi??c powinno by?? mo??liwe do czytania
                else {
                    printf("  Descriptor %d is readable\n", i);
                    close_conn = 0;


                    // Pr??bujemy przeczyta??, je??eli nie ma b????d??w
                    if ((rc = recv(i, buffer, sizeof(buffer), 0)) <= 0) {


                        // Je??eli s?? jakei?? b????dy musimy zamkn???? deskryptor,
                        // i usun???? go z master set i ustawi?? nowy ID maksymalne
                        // deskryptora dla ewentualnego nast??pnego klienta
                        close(i);
                        FD_CLR(i, &master_set);
                        if (i == max_d) {
                            printf("  Descriptor %d is disconnected\n", i);

                            while (FD_ISSET(max_d, &master_set) == 0)
                                max_d -= 1;
                        }
                    } else {

                        // Wysy??amy odczytan?? wiadomo???? do wszytkich deskryptor??w
                        // Kt??re nie s?? r??ne deskryptorowi od kt??rego przeczytalismy wiadomo????
                        for (int j = 0; j <= max_d; j++) {
                            if (FD_ISSET(j, &master_set)) {

                                if (j != sock_d && j != i) {
                                    if (send(j, buffer, rc, 0) == -1) {
                                        perror("send()");
                                    }
                                }
                            }
                        }
                    }


                }
            }
        }
    } while (end_server == 0);

    // Zwolnienie zasob??w po wyj??ciu z serwera
    for (i = 0; i <= max_d; ++i) {
        if (FD_ISSET(i, &master_set))
            close(i);
    }
}
