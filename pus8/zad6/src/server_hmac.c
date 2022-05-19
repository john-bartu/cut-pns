#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

void encrypt(char *plaintext, unsigned char *ciphertext, unsigned char *key, unsigned char *iv);

void decrypt(char *plaintext, const unsigned char *ciphertext, unsigned char *key, unsigned char *iv);

void printHex(char *title, char *text) {
    fprintf(stdout, "%s:\n", title);
    fprintf(stdout, "\tPLN:`%s`\n", text);
    fprintf(stdout, "\tHEX: `");
    for (int i = 0; i < strlen(text); i++)
        fprintf(stdout, "%02x ", (unsigned char) text[i]);
    fprintf(stdout, "`\n");
}

void GenerateHmac(char *message, unsigned char *key, unsigned char *hmac) {
    /* Wartosc zwracana przez funkcje: */
    int retval;

    int i;

    /* Rozmiar tekstu i szyfrogramu: */
    unsigned int message_len, hmac_len;

    int key_len = 16;

    /* Kontekst: */
    HMAC_CTX *ctx;

    const EVP_MD *md;


    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /*
     * Zaladowanie nazw funkcji skrotu do pamieci.
     * Wymagane przez EVP_get_digestbyname():
     */
    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("md5");
    if (!md) {
        fprintf(stderr, "Unknown message hmac: md5\n");
        exit(EXIT_FAILURE);
    }

    message_len = strlen(message);

    ctx = HMAC_CTX_new();

    retval = HMAC_Init_ex(ctx, key, key_len, md, NULL);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    unsigned char *message_ptr = (unsigned char *) message;

    retval = HMAC_Update(ctx, message_ptr, message_len);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    retval = HMAC_Final(ctx, hmac, &hmac_len);

    hmac[hmac_len] = '\0';

    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    HMAC_CTX_free(ctx);

    /* Usuniecie nazw funkcji skrotu z pamieci. */
    EVP_cleanup();

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();
}

int main(int argc, char **argv) {

    int sockfd; /* Deskryptor gniazda. */
    int retval; /* Wartosc zwracana przez funkcje. */

    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez recvfrom(): */
    unsigned char ciphertext[160];
    char plaintext[160];

    unsigned char keyA[] = "keyA";
    unsigned char keyB[] = "abcdef012345";
    unsigned char iv[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                          0x00, 0x01, 0x02, 0x03, 0x04, 0x05
    };

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char addr_buff[256];


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
    server_addr.sin_family = AF_INET;
    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Numer portu: */
    server_addr.sin_port = htons(atoi(argv[1]));
    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len = sizeof(server_addr);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(sockfd, (struct sockaddr *) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is waiting for UDP datagram...\n");

    client_addr_len = sizeof(client_addr);

    memset(ciphertext, 0, sizeof(ciphertext));

    /* Oczekiwanie na dane od klienta: */
    retval = recvfrom(
            sockfd,
            ciphertext, sizeof(ciphertext),
            0,
            (struct sockaddr *) &client_addr, &client_addr_len
    );
    if (retval == -1) {
        perror("recvfrom()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "UDP datagram received from %s:%d.\n",
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
            ntohs(client_addr.sin_port)
    );

//    ciphertext[strlen((char *) ciphertext) - 3] = 0;

    printHex("Cipher", ciphertext);

    decrypt(plaintext, ciphertext, keyB, iv);

    printHex("Message", plaintext);

    printf("\n-----UNPACK-----\n");

    char message[40];
    char hmac[40];
    unsigned char server_hmac[40];


    int delimiterIndex = 0;
    char *msg_ptr;
    msg_ptr = plaintext;

    while (plaintext[delimiterIndex] != '\n') {
        delimiterIndex++;
        msg_ptr++;
    }

    strncpy(hmac, plaintext, delimiterIndex);
    hmac[delimiterIndex] = '\0';

    strcpy(message, ++msg_ptr);

    printHex("Client HMAC", hmac);
    printHex("Client TEXT", message);

    GenerateHmac(message, keyA, server_hmac);
    printHex("SERVER HMAC", server_hmac);

    if (strcmp(hmac, server_hmac) == 0) {
        fprintf(stdout, "HMAC correct!");
    } else {
        fprintf(stdout, "HMAC Incorrect!");

    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}

void encrypt(char *plaintext, unsigned char *ciphertext, unsigned char *key, unsigned char *iv) {
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER *cipher;
    int retval, tmp;
    int plaintext_len, ciphertext_len;

    ERR_load_crypto_strings();

    ctx = EVP_CIPHER_CTX_new();

    EVP_CIPHER_CTX_init(ctx);

    cipher = EVP_aes_128_cbc();
    EVP_CIPHER_block_size(cipher);

    retval = EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    plaintext_len = strlen(plaintext);

    retval = EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len,
                               (unsigned char *) plaintext, plaintext_len);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    retval = EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &tmp);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    EVP_CIPHER_CTX_free(ctx);

    ciphertext_len += tmp;

    printf("\n");
}

void decrypt(char *plaintext, const unsigned char *ciphertext, unsigned char *key, unsigned char *iv) {
    int plaintext_len, ciphertext_len, tmp, retval;
    const EVP_CIPHER *cipher;
    EVP_CIPHER_CTX *ctx;

    cipher = EVP_aes_128_cbc();

    ctx = EVP_CIPHER_CTX_new();

    ciphertext_len = strlen(ciphertext);

    retval = EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    EVP_CIPHER_CTX_set_padding(ctx, 1);

    retval = EVP_DecryptUpdate(ctx, (unsigned char *) plaintext, &plaintext_len,
                               ciphertext, ciphertext_len);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    retval = EVP_DecryptFinal_ex(ctx, (unsigned char *) plaintext + plaintext_len, &tmp);
    if (!retval) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    plaintext_len += tmp;
    plaintext[plaintext_len] = '\0';

    EVP_CIPHER_CTX_free(ctx);
    ERR_free_strings();
}