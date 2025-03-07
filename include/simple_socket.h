#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

void init_server(int *sockfd_ptr, char *address, int log);
int init_client(char *address, int log);
void *handle_client(void *arg);

#endif