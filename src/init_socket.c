#include <simple_socket.h>
#include <arpa/inet.h>
#include <stdio.h>

int init_server(char *address) {
    char *ip, *port;

    if (sscanf(address, "%s:%s", ip, port) != 2) {
        perror("Invalid address format");
        return -1;
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0) {
        perror("Faid to create socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind socket");
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Failed to listen");
        return -1;
    }

    return sockfd;
}

int init_client(char *address) {
    char *ip, *port;

    if (sscanf(address, "%s:%s", ip, port) != 2) {
        perror("Invalid address format");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Failed to connect");
        return -1;
    }
    
    return sockfd;
}