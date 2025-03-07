#include <simple_socket.h>
#include <unistd.h>
#include <stdlib.h>

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    // Обработка соединения
    close(client_fd);
    return NULL;
}