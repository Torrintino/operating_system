#include "global.h"
#include "socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int open_socket() {
    int sfd;
    int sockopt=1;
    struct sockaddr_in srv_addr = {
	.sin_family = AF_INET,
	.sin_port = htons(8000),
	.sin_addr.s_addr = INADDR_ANY
    };
    socklen_t sad_sz = sizeof ( struct sockaddr_in);
    
    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
       die("socket");
    
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &sockopt, sizeof(sockopt)))
	die("setsockopt");
    
    if (bind(sfd, ( struct sockaddr*) &srv_addr, sad_sz) < 0)
	die("bind");
    
    if (listen(sfd, 1) < 0)
	die("listen");
    
    return sfd;
}

int connect_to_client(int sfd) {
    socklen_t sad_sz = sizeof ( struct sockaddr_in);
    struct sockaddr_in cli_addr;
    int cfd;
        
    cfd = accept(sfd, ( struct sockaddr*) &cli_addr, &sad_sz);
    if (cfd < 0)
	die("accept");
    printf("Connected to client on socket %d\n", cfd);

    return cfd;
}

void receive_message(int cfd, char* buf, int buf_size) {
    if(read(cfd, buf, buf_size) < 0)
	die("read");
}

void close_sockets(int sfd, int cfd) {
    close(cfd);
    close(sfd);
}
