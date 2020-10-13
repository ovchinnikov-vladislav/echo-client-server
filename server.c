#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

int get_server_socket(unsigned int port);
int get_client_socket(int socket_d, char* client_address[1024]);
unsigned int send_message(int socket_d, char *message);
unsigned int receive_line(int socket_d, char *dist_buffer);

int main(int argc, char **argv) {

    fd_set master;
    fd_set read_fds;
    int fdmax;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    printf("Start Echo Server\n");

    unsigned int port = 8081;

    if (argc < 2) {
        printf("PORT: 8080\n");
    } else {
        port = atoi(argv[1]);
        printf("PORT: %u", port);
    }

    int server_socket = get_server_socket(port);

    if (listen(server_socket, 5) == -1) {
        perror("ERROR: listen()");
        exit(-1);
    }

    FD_SET(server_socket, &master);

    fdmax = server_socket;

    char* clients_address[1024];

    int execute = TRUE;
    char *buffer = (char*) malloc(500);
    while (execute) {
        read_fds = master;

        if (pselect(fdmax + 1, &read_fds, NULL, NULL, NULL, NULL) == -1) {
            perror("pselect");
            exit(-1);
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_socket) {
                    int client_socket = get_client_socket(server_socket, clients_address);
                    printf("NEW CONNECT: %s\n", clients_address[client_socket]);

                    send_message(client_socket, "Hello!\n");
                    send_message(client_socket, "This is Echo Server.\n");
                    send_message(client_socket, "-> ");

                    FD_SET(client_socket, &master);

                    if (client_socket > fdmax) {
                        fdmax = client_socket;
                    }
                } else {
                    int bytes = receive_line(i, buffer);
                    if (bytes == -128128) {
                        printf("RECEIVE FROM %s: %s\n", clients_address[i], buffer);
                        char *result = (char*) malloc(strlen(buffer) + 10);
                        sprintf(result, "ECHO: %s\n-> ", buffer);
                        send_message(i, result);
                    } else if (bytes <= 0) {
                        if (bytes == 0) {
                            printf("close");
                        } else {
                            perror("recv");
                        }
                        shutdown(i, SHUT_RDWR);
                        close(i);
                        free(clients_address[i]);
                        FD_CLR(i, &master);
                    }
                }
            }
        }

    }
    free(buffer);

    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
}

int get_server_socket(unsigned int port) {
    int socket_d = socket(PF_INET, SOCK_STREAM, 0);

    if (socket_d == -1) {
        perror("ERROR: socket()");
        exit(-1);
    }

    int is_true = TRUE;

    if (setsockopt(socket_d, SOL_SOCKET, SO_REUSEADDR, &is_true, sizeof(is_true)) == -1) {
        perror("ERROR: setsockopt()");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = 0;
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (bind(socket_d, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) == - 1) {
        perror("ERROR: bind()");
        exit(-1);
    }

    return socket_d;
}

int get_client_socket(int socket_d, char* client_address[1024]) {
    struct sockaddr_in client_addr;
    socklen_t s_len = sizeof(struct sockaddr);
    int client_socket = accept(socket_d, (struct sockaddr*) &client_addr, &s_len);
    if (client_socket == -1) {
        perror("ERROR: accept");
        exit(-1);
    }

    client_address[client_socket] = (char*) malloc(10);
    sprintf(client_address[client_socket], "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_socket;
}

unsigned int send_message(int socket_d, char *message) {
    unsigned int all_bytes = strlen(message);
    char *ptr = message;
    while (all_bytes > 0) {
        int send_bytes = send(socket_d, message, all_bytes, 0);
        if (send_bytes == -1) {
            printf("SEND BYTES: %d", send_bytes);
            return 0;
        }
        all_bytes -= send_bytes;
        ptr += send_bytes;
    }
    return 1;
}

unsigned int receive_line(int socket_d, char *dist_buffer) {
    int end_chars_count = 0;
    char *ptr = dist_buffer;
    unsigned int start_size = sizeof(dist_buffer);
    int count_size = 0;
    int bytes;
    while ((bytes = recv(socket_d, ptr, 1, 0)) > 0) {
        count_size++;

        if (*ptr == '\0') {
            return bytes;
        }

        if (*ptr == '\n' || *ptr == '\r') {
            end_chars_count++;
            if (end_chars_count == 2) {
                *(ptr - 1) = '\0';
                return -128128;
            }
        } else {
            end_chars_count = 0;
        }
        ptr++;

        if (count_size == start_size - 2) {
            start_size += (start_size / 2);
            dist_buffer = realloc(dist_buffer, start_size);
        }
    }
    return bytes;
}