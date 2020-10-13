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
int get_client_socket(int socket_d);
unsigned int send_message(int socket_d, char *message);
unsigned int receive_line(int socket_d, char *dist_buffer);

int main(int argc, char **argv) {

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

    int execute = TRUE;
    char *buffer = (char*) malloc(500);
    while (execute) {
        int client_socket = get_client_socket(server_socket);
        send_message(client_socket, "Hello!\n");
        send_message(client_socket, "This is Echo Server.\n");
        unsigned int receive = TRUE;
        while(receive) {
            send_message(client_socket, "-> ");
            receive = receive_line(client_socket, buffer);

            printf("RECEIVE: %s\n", buffer);

            if (strncmp(buffer, "quit", 4) == 0) {
                execute = FALSE;
            }

            if (strncmp(buffer, "disc", 4) == 0) {
                receive = FALSE;
            }

            char *output = (char*) malloc(strlen(buffer) + 10);

            buffer = strcat(buffer, "\n");
            output = strcpy(output, "ECHO: ");
            send_message(client_socket, strcat(output, buffer));
            free(output);
        }
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
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

int get_client_socket(int socket_d) {
    struct sockaddr_in client_addr;
    socklen_t s_len = sizeof(struct sockaddr);
    int client_socket = accept(socket_d, (struct sockaddr*) &client_addr, &s_len);
    if (client_socket == -1) {
        perror("ERROR: accept");
        exit(-1);
    }

    printf("NEW CONNECT: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

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
    while (recv(socket_d, ptr, 1, 0) != -1) {
        count_size++;

        if (*ptr == '\0') {
            return 0;
        }

        if (*ptr == '\n' || *ptr == '\r') {
            end_chars_count++;
            if (end_chars_count == 2) {
                *(ptr - 1) = '\0';
                return strlen(dist_buffer) + 2;
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
    return -1;
}