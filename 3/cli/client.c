#include "global.h"
#include "../protocol.h"
#include "socket.h"

#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 256

/* 
 * Whenever a program is executed remotely, this shall be run as a thread.
 * When the remote program terminates, cancel this thread.
 * This function redirects the shell input to the server.
 * The input will also be printed to the users shell.
 */
void* input_redirection(void* data) {
    int socketfd = *((int*)data);
    int read_chars;
    char msg[BUF_SIZE];

    while(1) {
	if((read_chars =read(STDIN_FILENO, &(msg[1]), BUF_SIZE-2)) == -1)
	    die("read");
	msg[read_chars + 1] = 0;
	msg[0] = C_INPUT;
	if(write(socketfd, msg, BUF_SIZE) == -1)
	    die("write");
    }
    
    return NULL;
}

int run_program(char* msg, int msg_size, int cfd) {
    pthread_t input_channel;
    pthread_create(&input_channel, 0, &input_redirection, &cfd);
    
    msg[0] = C_CMD;
    char* txt = &(msg[1]);
    if(write(cfd, msg, msg_size) == -1)
	die("write");

    do {
	if(read(cfd, msg, BUF_SIZE) == -1)
	    die("read");
	switch(msg[0]) {
	case S_ERROR:
	    fprintf(stderr, "Server returned error");
	    pthread_cancel(input_channel);
	    pthread_join(input_channel, NULL);
	    return -1;
	case S_WALL:
	case S_PRG_RUNNING:
	    printf("%s", txt);
	    break;
	case S_PRG_TERM:
	    pthread_cancel(input_channel);
	    pthread_join(input_channel, NULL);
	    break;
	default:
	    fprintf(stderr, "Server returned unviable opcode");
	    return -1;
	}
    } while(msg[0] != S_PRG_TERM);
    
    return 0;
}

int main(int argc, char* argv[]) {
    int cfd = connect_to_server();
    char msg[BUF_SIZE];
    char* lineptr = NULL;
    size_t read_chars;
    const char* prompt = ">";
    
    while(1) {
	for(int i=0; i<BUF_SIZE; i++)
	    msg[i] = 0;
	
	printf("%s", prompt);
	read_chars = getline(&lineptr, &read_chars, stdin);
	if(lineptr[0] == '\n')
	    continue;
	if(read_chars == -1)
	    die("getline");
	if(read_chars >= BUF_SIZE) {
	    fprintf(stderr, "Error: Input line is too long\n");
	    return -1;
	}
	lineptr[read_chars - 1] = 0;
	msg[0] = C_ERROR; // Make sure that msg[0] is not uninitialized

	strtok(lineptr, " ");
	while(strtok(NULL, " ") != NULL);

	memcpy(&(msg[1]), lineptr, read_chars);

	if(strcmp(lineptr, "wall") == 0) {
	    // When we receive Wall, we don't wait for the message, as we cannot
	    //  distinguish it from a Wall message send from another client
	    // We make sure, that every time we receive a message, we check for wall
	    msg[0] = C_CMD;
	    if(write(cfd, msg, BUF_SIZE) == -1)
		die("write");
	    continue;
	} else if(strcmp(lineptr, "put") == 0) {
	    msg[0] = C_PUT_SENDING;
	} else if(strcmp(lineptr, "exit") == 0) {
	    msg[0] = C_EXIT;
	    if(write(cfd, msg, 1) == -1)
		die("write");
	    close(cfd);
	    free(lineptr);
	    return 0;
	} else if(strcmp(lineptr, "get") == 0) {
	    msg[0] = C_GET;
	} else {
	    if(run_program(msg, read_chars, cfd) == -1) {
		fprintf(stderr, "Running a command failed, can't recover");
		return -1;
	    }
	}
    
    }
    
    close_socket(cfd);
    return 0;
}

