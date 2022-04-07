#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFER_SIZE 80

char *generate_main_response();
char *str_append(const char *, const char *);
char *str_append_len(const char *old, size_t old_len, const char *new, size_t new_len);
void *handle_request(void*);


char* static_page;
int main(int argc, char **argv) {
    size_t rc;
    int sock_d, client_sock_d;
    struct sockaddr_in6 addr;
    char addr_buff[256];

    char *server_port = argv[1];


    // Wygenerowanie statycznej strony tylko raz i przechowywanie w pamięci
    static_page = generate_main_response();

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

    /* Ustawienie portu serwera */
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(strtol(server_port, NULL, 10));

    rc = bind(sock_d, (struct sockaddr *) &addr, sizeof(addr));
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

    /* Główna pętla serwera */
    while(1) {
        printf("Waiting on accept()...\n");
        client_sock_d = accept(sock_d, NULL, NULL);
        if (client_sock_d < 0) {
            if (errno != EWOULDBLOCK) {
                perror("accept()");
                exit(EXIT_FAILURE);
            }
        }

        fprintf(stdout, "%s:%d connected\n",
                inet_ntop(AF_INET, &addr.sin6_addr, addr_buff, sizeof(addr_buff)),
                ntohs(addr.sin6_port)
        );

        pthread_t new_thread;
        int *client_sock_d_copy = (int*)malloc(sizeof(int));
        *client_sock_d_copy= client_sock_d;

        int res = pthread_create(&new_thread, NULL, handle_request, (void*) client_sock_d_copy);
        if (res < 0) {
            perror("pthread_create()");
            exit(EXIT_FAILURE);
        }
    };
}

void *handle_request(void* new_sock) {
    size_t len, rc;
    int sock_d, is_image_get, end_request = 0, k = 0;
    char buffer[BUFFER_SIZE], *request = "";

    char *extensions[] = {".png", ".gif", ".jpg", ".jpeg", ".bmp"};
    char *content_types[] = {"image/png", "image/gif", "image/jpeg", "image/jpeg", "image/bmp"};

    sock_d = *(int *) new_sock;

    /* Odczyt pytania */
    while (!end_request) {
        rc = recv(sock_d, buffer, sizeof(buffer), 0);
        if (rc < 0) {
            if (errno != EWOULDBLOCK) {
                perror("recv()");
            }
            break;
        } else if (rc == 0) {
            printf("Connection closed\n");
            break;
        }

        len = rc;
        request = str_append(request, buffer);

        if (len > 4) {
            for (int j = 4; j < len; j++) {
                /* Detekcja końcówki zapytania */
                if (buffer[j - 3] == 13 &&
                    buffer[j - 2] == 10 &&
                    buffer[j - 1] == 13 &&
                    buffer[j] == 10) {

                    size_t final_len;
                    char *response;
                    char http_method[20], path[2048];

                    /* Pobieranie pierwszej linijki zapytania */
                    char *sub = strchr(request, '\n');
                    int index = (int) (sub - request);
                    char first_line[index];
                    memcpy(first_line, request, index);

                    /* Odczyt metody HTTP oraz ścieżki zapytania */
                    char *first_line_ptr = strtok(first_line, " ");
                    strcpy(http_method, first_line_ptr);
                    first_line_ptr = strtok(NULL, " ");
                    strcpy(path, first_line_ptr);

                    printf("Request path: %s\n", path);

                    char *ext = strrchr(path, '.');

                    is_image_get = 0;
                    if (strcmp(http_method, "GET") == 0) {
                        char *_ext;
                        /* Sprawdzenie czy zapytanie jest w celu pobrania pliku graficznego */
                        while (ext != NULL && k < 5) {
                            _ext = extensions[k];
                            if (strcmp(ext, _ext) == 0) {
                                char *formatted_path, *image_buffer;
                                long image_len;
                                char image_len_str[256];
                                FILE *image_ptr;

                                is_image_get = 1;

                                formatted_path = (char *) malloc(sizeof(char) * strlen(path) - 1);
                                char *path_ptr = path;
                                path_ptr++;
                                strcpy(formatted_path, path_ptr);

                                /* Odczyt pliku oraz tworzenie odpowiedzi */
                                if ((image_ptr = fopen(formatted_path, "rb")) != NULL) {
                                    fseek(image_ptr, 0, SEEK_END);
                                    image_len = ftell(image_ptr);

                                    rewind(image_ptr);
                                    image_buffer = (char *) malloc((image_len + 1) * sizeof(char));
                                    fread(image_buffer, image_len, 1, image_ptr);
                                    fclose(image_ptr);

                                    response = "HTTP/1.1 200 OK\r\n"
                                               "Connection: close\r\n"
                                               "Content-Type: ";
                                    response = str_append(response, content_types[k]);
                                    response = str_append(response, "\r\nContent-Length: ");

                                    sprintf(image_len_str, "%ld", image_len);
                                    response = str_append(response, image_len_str);
                                    response = str_append(response, "\r\n\r\n");
                                    size_t headers_len = strlen(response);

                                    response = str_append_len(response, headers_len, image_buffer,
                                                              image_len);
                                    final_len = headers_len + image_len;

                                    printf("Responding to %s with image\n", path);

                                    break;
                                } else {
                                    response = "HTTP/1.1 404 not found\r\n"
                                               "Connection: close\r\n"
                                               "\r\n";
                                    final_len = strlen(response);
                                }
                            }

                            k++;
                        }

                        if (is_image_get == 0) {
                            /* Tworzenie odpowiedzi w postaci głównej strony HTML */
                            response = static_page;

                            final_len = strlen(response);

                            printf("Responding to %s with HTML\n", path);
                        }
                    } else {
                        response = "HTTP/1.1 404 not found\r\n"
                                   "Connection: close\r\n"
                                   "\r\n";
                        final_len = strlen(response);
                    }

                    /* Wysłanie odpowiedzi */
                    rc = send(sock_d, response, final_len, 0);
                    if (rc < 0) {
                        perror("send() failed");
                    }

                    k = 0;
                    end_request = 1;
                    break;
                }
            }
        }
        memset(buffer, 0, BUFFER_SIZE);
    }

    free(new_sock);
    return 0;
}

/* Generowanie odpowiedzi głównej strony HTML */
char *generate_main_response() {
    char *response = "HTTP/1.1 200 OK\r\n"
                     "Connection: close\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n"
                     "<html><body><center>\r\n";

    DIR *folder;
    struct dirent *entry;
    int files = 0;

    folder = opendir("./bin/img");
    if (folder == NULL) {
        perror("Unable to read directory");
    }

    while ((entry = readdir(folder))) {
        files++;
        if (strcmp(entry->d_name, ".") != 0
            && strcmp(entry->d_name, "..") != 0) {

            char img_tag_str[256];
            sprintf(img_tag_str, "<img src=\"/bin/img/%s\"><br/>\r\n", entry->d_name);
            response = str_append(response, img_tag_str);
        }
    }
    closedir(folder);

    response = str_append(response,
                          "</center></body></html>\r\n\r\n");

    return response;
}

/* Funkcja do konkatenacji stringów */

char *str_append(const char *old, const char *new) {
    const size_t old_len = strlen(old), new_len = strlen(new);
    const size_t out_len = old_len + new_len + 1;

    char *out = malloc(out_len + 1);

    strcpy(out, old);
    memcpy(out + old_len, new, new_len + 1);

    return out;
}

/* Funkcje do konkatenacji stringu z content, ponieważ w
 * content mogą być bajty zerowe,
 * a w C koniec stringu to bajt zerowy */

char *str_append_len(const char *old, size_t old_len, const char *new, size_t new_len) {
    const size_t out_len = old_len + new_len;

    char *out = malloc(out_len);

    memcpy(out, old, old_len);
    memcpy(out + old_len, new, new_len);

    return out;
}