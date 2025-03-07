#include <simple_socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <fcntl.h>

atomic_int server_running = 1;
pthread_t server_thread;

/*
    Initialize a server socket

    Parameters:
        sockfd_ptr - Pointer to store the socket file descriptor
        address - The address of the server in the format "ip:port"
        log - The log function, 1 - on, 0 - off
*/
void init_server(int *sockfd_ptr, char *address, int log) {
    char ip[16];
    char port[6];
    struct tm *now;
    time_t curr_time;

    if (sscanf(address, "%15[^:]:%5s", ip, port) != 2) {
        perror("Invalid address format");
        return;
    }

    if (strncmp(ip, "localhost", 9) == 0) {
        ip[0] = '\0';
        strncpy(ip, "127.0.0.1", 9);
    }
    
    *sockfd_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (*sockfd_ptr < 0) {
        perror("Failed to create socket");
        return;
    }

    int flags = fcntl(*sockfd_ptr, F_GETFL, 0);
    fcntl(*sockfd_ptr, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return;
    }

    if (bind(*sockfd_ptr, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind socket");
        close(*sockfd_ptr);
        return;
    }

    ServerArgs *args = (ServerArgs*)malloc(sizeof(ServerArgs));
    args->sockfd_ptr = sockfd_ptr;
    args->log = log;

    if (pthread_create(&server_thread, NULL, handle_server, args) != 0) {
        perror("Failed to create server thread");
        free(args);
        return;
    }

    if (log) {
        curr_time = time(0);
        now = localtime(&curr_time);
        printf("[%02d:%02d:%02d] Server started on %s:%s\n", now->tm_hour, now->tm_min, now->tm_sec, ip, port);
    }
}

/*
    Initialize a client socket

    Parameters:
        address - The address of the server in the format "ip:port"
        log - The log function, 1 - on, 0 - off
*/
int init_client(char *address, int log) {
    char ip[16];
    char port[6];
    struct tm *now;
    time_t curr_time;

    if (sscanf(address, "%15[^:]:%5s", ip, port) != 2) {
        perror("Invalid address format");
        return -1;
    }

    if (strncmp(ip, "localhost", 9) == 0) {
        ip[0] = '\0';
        strncpy(ip, "127.0.0.1", 9);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
        perror("setsockopt");
        close(sockfd);
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Failed to connect");
        return -1;
    }

    if (log) {
        curr_time = time(0);
        now = localtime(&curr_time);
        printf("[%02d:%02d:%02d] Connected to %s:%s\n", now->tm_hour, now->tm_min, now->tm_sec, ip, port);
    }

    return sockfd;
}