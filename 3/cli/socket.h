#ifndef SOCKET_H
#define SOCKET_H

int connect_to_server();
void send_message(int cfd, char* msg, int msg_len);
void close_socket(int cfd);

#endif
