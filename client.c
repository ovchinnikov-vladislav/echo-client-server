#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

// TODO: Исправить баг с памятью

typedef struct pselect_thread_args {
    int *fdmax;
    fd_set *read_fds;
    fd_set *write_fds;
    char* data;
} pselect_args_t;

unsigned int receive_line(int socket_d, char *dist_buffer);
unsigned int send_message(int socket_d, char *message);
void *pselect_thread(void *args);

int main(int argc, char **argv) {

    pthread_t thread;

    fd_set master;      // главный сет дескрипторов
    fd_set read_fds;    // временный сет для чтения pselect()
    fd_set write_fds;   // временный сет для записи pselect()
    int fdmax = 0;          // максимальное число дескрипторов в определенный момент времени

    FD_ZERO(&master);   // обнуляем сеты
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    printf("ECHO CLIENT START\n");

    char *ip = (char *) malloc(20);
    unsigned int port;

    unsigned int operation;
    char *buffer = (char *) malloc(1000);
    char *input = (char *) malloc(1000);

    struct pselect_thread_args pselectThread;
    struct pselect_thread_args *pselectThreadArgs = &pselectThread;
    pselectThreadArgs->fdmax = &fdmax;
    pselectThreadArgs->write_fds = &write_fds;
    pselectThreadArgs->read_fds = &read_fds;
    pselectThreadArgs->data = buffer;

    printf("Operation:\n");
    printf("1. Connect to new server\n");
    printf("2. Send data to select server\n");
    printf("3. Send data to all server\n");
    printf("4. Exit\n");

    while (TRUE) {
        printf("num operation: ");
        scanf("%i", &operation);

        switch (operation) {
            case 1:
                printf("Input IP: ");
                scanf("%s", ip);
                printf("Input port: ");
                scanf("%i", &port);
                int server_socket = socket(PF_INET, SOCK_STREAM, 0);
                fcntl(server_socket, F_SETFL, O_NONBLOCK);

                if (server_socket == -1) {
                    perror("ERROR: socket()");
                    exit(-1);
                }

                struct sockaddr_in server_addr;
                server_addr.sin_family = PF_INET;
                inet_aton(ip, &(server_addr.sin_addr));
                server_addr.sin_port = htons(port);
                memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

                socklen_t socklen = sizeof(struct sockaddr);

                if (connect(server_socket, (struct sockaddr*) &server_addr, socklen) == -1 && errno != EINPROGRESS) {
                    perror("ERROR: connect()");
                    exit(-1);
                } else {
                    printf("SERVER SOCKET for %s:%i: %i\n", ip, port, server_socket);
                }

                FD_SET(server_socket, &master);         // добавляем в главный сет сокет сервера

                if (server_socket > fdmax) {
                    fdmax = server_socket;                    // максимальное количество дескрипторов равно номеру дескриптора сокета сервера
                }
                break;
            case 2:
                memset(input, '\0', 1000);
                printf("number server socket: ");
                scanf("%i", &server_socket);
                printf("sending data: ");
                scanf("%s", input);
                send_message(server_socket, strcat(input, "\n\r"));
                break;
            case 3:
                memset(input, '\0', 1000);
                printf("sending data: ");
                scanf("%s", input);
                input = strcat(input, "\n\r");
                for (int i = 0; i <= fdmax; i++) {
                    int server_s = i;
                    send_message(server_s, input);
                }
                break;
            case 4:
                printf("Exit from soft");
                exit(0);
            default:
                printf("Undefined operation (repeat)");
        }

        read_fds = master;

        pthread_create(&thread, NULL, pselect_thread, pselectThreadArgs);
       // pthread_join(thread, NULL);
    }
}

void *pselect_thread(void *args) {
    while (TRUE) {
        struct pselect_thread_args *pselectThreadArgs = (struct pselect_thread_args *) args;
        // стартуем вызов pselect()
        if (pselect(*pselectThreadArgs->fdmax + 1, pselectThreadArgs->read_fds, pselectThreadArgs->write_fds, NULL,
                    NULL, NULL) == -1) {
            perror("pselect");
            exit(-1);
        }

        // проходим через существующие соединения, ищем данные для чтения
        for (int i = 0; i <= *pselectThreadArgs->fdmax; i++) {
            if (FD_ISSET(i, pselectThreadArgs->read_fds)) {    // есть какие-то данные от сокетов
                int server_socket = i;
                memset(pselectThreadArgs->data, '\0', 1000);
                receive_line(server_socket, pselectThreadArgs->data);

                printf("\nOutput SOCKET #%i:\n", server_socket);
                printf("%s\n", pselectThreadArgs->data);
            }
        }
    }
}

unsigned int receive_line(int socket_d, char *dist_buffer) {
    char *ptr = dist_buffer;
    unsigned int start_size = sizeof(dist_buffer);
    memset(dist_buffer, 0, start_size);
    unsigned int count_bytes = 0;
    while (recv(socket_d, ptr, 1, 0) != -1) {
        count_bytes++;
        ptr++;
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