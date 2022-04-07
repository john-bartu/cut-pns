#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <ctype.h>
#include "libpalindrome.h"

int main(int argc, char** argv) {

    int             sockfd; /* Deskryptor gniazda. */
    int             retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom() i sendto(): */
    char            buff_in[256], buff_out[256];

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(sockfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    memset(buff_in, 0, 256);
    fprintf(stdout, "Server is listening for incoming connection...\n");

    while (1) {
        /* Oczekiwanie na dane od klienta: */
        client_addr_len = sizeof(client_addr);
        retval = recvfrom(
                sockfd,
                buff_in, sizeof(buff_in),
                0,
                (struct sockaddr*)&client_addr, &client_addr_len
        );
        if (retval == -1) {
            perror("recvfrom()");
            exit(EXIT_FAILURE);
        }

        if (strlen(buff_in) == 0) {
            exit(EXIT_SUCCESS);
        }

        fprintf(stdout, "UDP datagram received from %s:%d. Echoing message...\n",
                inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
                ntohs(client_addr.sin_port)
        );

        sleep(2);

        switch (is_palindrome(buff_in, (int) strlen(buff_in))) {
            case -1:
                strcpy(buff_out, strcat(buff_in, " is invalid input"));
                break;
            case 0:
                strcpy(buff_out, strcat(buff_in, " is not a palindrome\0"));
                break;
            case 1:
                strcpy(buff_out, strcat(buff_in, " is a palindrome\0"));
                break;
            default:
                perror("is_palindrome()");
                exit(EXIT_FAILURE);
        }

        /* Wyslanie odpowiedzi (echo): */
        retval = sendto(
                sockfd,
                buff_out, sizeof(buff_out),
                0,
                (struct sockaddr*)&client_addr, client_addr_len
        );
        if (retval == -1) {
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        memset(buff_in, 0, 256);
    }

//    close(sockfd);
//    exit(EXIT_SUCCESS);
}
