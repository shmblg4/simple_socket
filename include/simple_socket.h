#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

#include <pthread.h>
#include <stdatomic.h>

#pragma pack (push, 1)
typedef struct {
    int *sockfd_ptr;
    int log;
} ServerArgs;
#pragma pack (pop)

void init_server(int *sockfd_ptr, char *address, int log);
int init_client(char *address, int log);
void *handle_client(void *arg);
void *handle_server(void *arg);
void stop_server(int sockfd);

#endif