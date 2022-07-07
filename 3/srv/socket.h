#ifndef SOCKET_H
#define SOCKET_H

#define PORT 9000

int open_socket();
int connect_to_client(int sfd);
void receive_message(int cfd, char* buf, int buf_size);
void close_sockets(int sfd, int cfd);

#endif
