#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

void encrypt(char *plaintext, unsigned char *ciphertext, unsigned char *key, unsigned char *iv);

void decrypt(char *plaintext, const unsigned char *ciphertext, unsigned char *key, unsigned char *iv);

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

void printHex(char *title, char *text) {
    fprintf(stdout, "%s:\n", title);
    fprintf(stdout, "\tPLN:`%s`\n", text);
    fprintf(stdout, "\tHEX: `");
    for (int i = 0; i < strlen(text); i++)
        fprintf(stdout, "%02x ", (unsigned char) text[i]);
    fprintf(stdout, "`\n");
}

int main(int argc, char **argv) {

    int sockfd;                 /* Desktryptor gniazda. */
    int retval;                 /* Wartosc zwracana przez funkcje. */
    struct sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t addr_len;               /* Rozmiar struktury w bajtach. */

    char plaintext[40] = "Laboratorium PUS.";
    unsigned char hmac[40];


    unsigned char *ciphertext[160];
    memset(ciphertext, 0, 160);

    unsigned char *total_message[160];
    memset(total_message, 0, 160);


    unsigned char keyA[] = "keyA";
    unsigned char keyB[] = "abcdef012345";
    unsigned char iv[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                          0x00, 0x01, 0x02, 0x03, 0x04, 0x05
    };

    GenerateHmac(plaintext, keyA, hmac);


    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;

    /* Konwersja adresu IP z postaci kropkowo-dziesietnej: */
    retval = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */

    strcat(total_message, hmac);
    strcat(total_message, "\n");
    strcat(total_message, plaintext);

    encrypt(total_message, ciphertext, keyB, iv);


    printHex("Plaintext", total_message);
    printHex("Cipher", ciphertext);

    fprintf(stdout, "Sending message to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
            sockfd,
            ciphertext, strlen((char *) ciphertext),
            0,
            (struct sockaddr *) &remote_addr, addr_len
    );

    if (retval == -1) {
        perror("sendto()");
        exit(EXIT_FAILURE);
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