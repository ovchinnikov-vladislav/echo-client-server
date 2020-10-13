#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

unsigned int receive_line(int socket_d, char *dist_buffer);
unsigned int send_message(int socket_d, char *message);

int main(int argc, char **argv) {

    printf("ECHO CLIENT START\n");

    char *ip = (char *) malloc(20);
    strcpy(ip, "127.0.0.1");
    unsigned int port = 8081;

    if (argc < 2) {
        printf("SERVER ADDRESS: 127.0.0.1:8080\n");
    } else {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
    }

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        perror("ERROR: socket()");
        exit(-1);
    }

//    struct timeval timeout;
//    timeout.tv_sec = 0;
//    timeout.tv_usec = 1;
//
//    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) == -1) {
//        perror("ERROR: timeout recv");
//        exit(-1);
//    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    inet_aton(ip, &(server_addr.sin_addr));
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    socklen_t socklen = sizeof(struct sockaddr);

    if (connect(server_socket, (struct sockaddr*) &server_addr, socklen) == -1) {
        perror("ERROR: connect()");
        exit(-1);
    }

    int is_execute = TRUE;
    char *buffer = (char *) malloc(1000);
    char *input = (char *) malloc(1000);
    while (is_execute) {
        memset(buffer, '\0', 1000);
        memset(input, '\0', 1000);
        receive_line(server_socket, buffer);
        printf("%s", buffer);
        gets(input);

        if (strncmp(input, "quit", 4) == 0) {
            is_execute = FALSE;
        }
        send_message(server_socket, strcat(input, "\n\r"));
    }
    free(input);
    free(buffer);

    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
}

unsigned int receive_line(int socket_d, char *dist_buffer) {
    char *ptr = dist_buffer;
    unsigned int start_size = sizeof(dist_buffer);
    unsigned int count_bytes = 0;
    while (recv(socket_d, ptr, 1, 0) != -1) {
        count_bytes++;
        ptr++;
        if (count_bytes == start_size - 2) {
            start_size += (start_size / 2);
            dist_buffer = realloc(dist_buffer, start_size);
        }
    }
    return strlen(dist_buffer);
}

unsigned int send_message(int socket_d, char *message) {
    char *ptr = message;
    int all_size = strlen(message);
    while (all_size > 0) {
        int send_size = send(socket_d, message, all_size, 0);
        if (send_size == -1) {
            return 0;
        }
        all_size -= send_size;
        ptr += send_size;
    }
    return 1;
}