#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define TRUE 1
#define FALSE 0

void fatal(char *message);
void* ec_malloc(unsigned int size);
int prepare_socket_server(unsigned int port);
int get_connect(int server_socket);
unsigned int send_message(int client_socket, char *message);
unsigned int receive_line(int client_socket, char *dist_buffer);

int main(int argc, char **argv) {
    unsigned int port = 8080;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    int server_socket = prepare_socket_server(port);

    if (listen(server_socket, 5) == -1) {
        fatal("listen()");
    }

    int execute = TRUE;
    while (execute) {
        int client_socket = get_connect(server_socket);
        send_message(client_socket, "Hello!\n");
        send_message(client_socket, "This is ECHO SERVER.\n");
        int cont_receive = FALSE;
        do {
            send_message(client_socket, "-> ");
            char *buffer = (char*) ec_malloc(10);
            cont_receive = receive_line(client_socket, buffer);
            printf("RECEIVE: %s\n", buffer);
            send_message(client_socket, strcat(buffer, "\n"));
            printf("SENDING: %s\n", buffer);
            if (strncmp(buffer, "quit", 4) == 0) {
                execute = FALSE;
            }
            if (strncmp(buffer, "disc", 4) == 0) {
                cont_receive = FALSE;
            }
            free(buffer);
        } while(cont_receive && execute);
    }

    return 0;
}

int prepare_socket_server(unsigned int port) {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        fatal("socket() create socket");
    }

    int is_true = TRUE;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &is_true, sizeof(is_true)) == -1) {
        fatal("setsockopt() setting TRUE SO_REUSEADDR");
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = 0;
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), '\0', sizeof(server_addr.sin_zero));

    if (bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) == -1) {
        fatal("bind() setting sockaddr");
    }

    printf("ECHO SERVER\n");
    printf("BIND ADDRESS: %s:%d\n\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    return server_socket;
}

int get_connect(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr);
    int client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &sin_size);
    if (client_socket == -1) {
        fatal("accept() getting connect");
    }

    printf("NEW CONNECT: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_socket;
}

unsigned int send_message(int client_socket, char *message) {
    int size = strlen(message);
    while (size > 0) {
        int sending_bytes_count = send(client_socket, message, size, 0);
        if (sending_bytes_count == -1) {
            return 0;
        }
        size -= sending_bytes_count;
        message += sending_bytes_count;
    }
    return 1;
}

unsigned int receive_line(int client_socket, char *dist_buffer) {
    unsigned int size = sizeof(dist_buffer);

    unsigned int count_bytes = 0;
    char *ptr = dist_buffer;
    int count_chars = 0;
    while (recv(client_socket, ptr, 1, 0) != -1) {
        if (*ptr == '\n' || *ptr == '\r') {
            count_chars++;
            if (count_chars == 2) {
                *(ptr - 1) = '\0';
                return strlen(dist_buffer);
            }
        } else {
            count_chars = 0;
        }
        count_bytes++;
        ptr++;
        if (count_bytes == size - 2) {
            size += count_bytes / 2;
            ptr = realloc(dist_buffer, size);
            ptr += count_bytes;
        }
    }
    return 0;
}

void fatal(char *message) {
    char *error_message = (char*) malloc(strlen(message) + 50);
    if (error_message == NULL) {
        perror("[!!] ERROR: fatal() malloc");
        exit(-1);
    }

    strcpy(error_message, "[!!] ERROR: ");
    strcat(error_message, message);

    perror(error_message);

    exit(-1);
}

void* ec_malloc(unsigned int size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fatal("ec_malloc() memory allocate");
    }
    return ptr;
}