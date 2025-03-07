#ifndef SIMPLE_SOCKET_H
#define SIMPLE_SOCKET_H

int init_server(int ip, int port);
int init_client(int ip, int port);
int close_socket(int sockfd);
#endif