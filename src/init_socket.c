#include <simple_socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

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
    }

    if (strncmp(ip, "localhost", 9) == 0) {
        ip[0] = '\0';
        strncpy(ip, "127.0.0.1", 9);
    }
    
    *sockfd_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (*sockfd_ptr < 0) {
        perror("Faid to create socket");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
    }

    if (bind(*sockfd_ptr, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind socket");
    }

    if (listen(*sockfd_ptr, 5) < 0) {
        perror("Failed to listen");
    }

    if (log) {
        curr_time = time(0);
        now = localtime(&curr_time);
        printf("[%02d:%02d:%02d] Server started on %s:%s\n", now->tm_hour, now->tm_min, now->tm_sec, ip, port);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(*sockfd_ptr, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Failed to accept connection");
            continue;
        }

        if (log) {
            curr_time = time(0);
            now = localtime(&curr_time);
            printf("[%02d:%02d:%02d] New connection from %s:%d\n",
                now->tm_hour, now->tm_min, now->tm_sec, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // Создаем поток для обработки соединения
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, &client_fd) != 0) {
            perror("Failed to create thread");
            close(client_fd);
        } else {
            pthread_detach(thread);
        }
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