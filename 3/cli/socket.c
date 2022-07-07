#include "global.h"
#include "socket.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int connect_to_server() {
    int cfd;
    struct sockaddr_in addr = {
	.sin_family = AF_INET,
	.sin_port = htons(8000),
	.sin_addr.s_addr = inet_addr("127.0.0.1")
    };
	
    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	die("Couldn't open the socket");
	
    if (connect(cfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
	die("Couldn't connect to socket");
    
    return cfd;
}

void send_message(int cfd, char* msg, int msg_len) {
    if(write(cfd, msg, msg_len) < 0)
	die("Couldn't receive message");
}

void close_socket(int cfd) {
    close(cfd);
}
