#include <simple_socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>

extern int server_running;
extern pthread_t server_thread;

void *handle_client(void* arg) {
    int *client_fd = (int*)arg;
    if (*client_fd < 0) {
        free(client_fd);
        return NULL;
    }

    close(*client_fd);
    printf("Client disconnected\n");
    free(client_fd);
    return NULL;
}

void *handle_server(void* arg) {
    ServerArgs *server_args = (ServerArgs*)arg;
    time_t curr_time;
    struct tm *now;

    if (listen(*server_args->sockfd_ptr, 5) < 0) {
        perror("Failed to listen");
        free(server_args);
        return NULL;
    }

    while (server_running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(*server_args->sockfd_ptr, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(*server_args->sockfd_ptr + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("Failed to select");
            break;
        }

        if (activity == 0) {
            continue;
        }

        if (FD_ISSET(*server_args->sockfd_ptr, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int *client_fd = malloc(sizeof(int));
            *client_fd = accept(*server_args->sockfd_ptr, (struct sockaddr*)&client_addr, &client_len);
            if (*client_fd < 0) {
                free(client_fd);
                break;
            }
        
            if (server_args->log) {
                curr_time = time(0);
                now = localtime(&curr_time);
                printf("[%02d:%02d:%02d] New connection from %s:%d\n",
                    now->tm_hour, now->tm_min, now->tm_sec, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }

            pthread_t thread;
            if (pthread_create(&thread, NULL, handle_client, client_fd) != 0) {
                perror("Failed to create thread");
                close(*client_fd);
                free(client_fd);
            } else {
                pthread_detach(thread);
            }
        }
    }

    free(server_args);
    return NULL;
}

/*
    Stop the server
*/
void stop_server(int sockfd) {
    server_running = 0;

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    pthread_join(server_thread, NULL);
}