#include <simple_socket.h>

int close_socket(int sockfd) {
    return close(sockfd);
}