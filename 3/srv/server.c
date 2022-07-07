#include "global.h"
#include "../protocol.h"
#include "socket.h"
#include "thread.h"

#include <errno.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// We only want to actually use these globally in main and wall
thread_block_t* global_tb = NULL;
int global_threads_running = 0;

int main() {
    global_tb = init_threads();
    int sfd = open_socket();

    while(1) {
	global_threads_running = check_threads(global_tb,
					       global_threads_running);
	realloc_threads(global_tb, global_threads_running);
	int cfd = connect_to_client(sfd);
	create_thread(global_tb, global_threads_running, cfd);
	global_threads_running++;
    }
    
}


char* cd_invalid_argc = "cd failed: Exactly one argument must be passed\n";
char* cd_failed = "cd failed: Couldn't change the directory\n";

// Returns NULL for success, otherwise a pointer to an error message
int bi_cd(thread_block_t* tb, char** tokens, int token_count) {
    if(token_count != 2)
	return -1;
    return 0;
}

void bi_get(char** tokens, int token_count) {

}

void bi_put(char** tokens, int token_count) {

}


#define BUF_SIZE 256

void bi_wall(char** tokens, int token_count) {
    char msg[BUF_SIZE];
    msg[0] = S_WALL;

    int msg_len = 0;
    for(char* c = tokens[1];
	!(c >= tokens[token_count-1] && *c == 0);
	c++) {
	msg_len++;
	if(msg_len > BUF_SIZE-1)
	    break;
	if(*c == 0)
	    *c = ' ';
	msg[msg_len] = *c;
    }
    for(int i=msg_len; i<BUF_SIZE-1; i++)
	msg[i] = 0;
    for(int i=0; i<global_threads_running; i++)
	if(write(global_tb[i].cfd, msg, BUF_SIZE) == -1)
	    die("write");
}

void* output_redirection(void* data) {
    int pipefd = ((int*) data)[0];
    int socketfd = ((int*) data)[1];
    char msg[BUF_SIZE];
    msg[0] = S_PRG_RUNNING;
    int read_chars;

    while(1) {
	for(int i=1; i<BUF_SIZE; i++)
	    msg[i] = 0;
	
	if((read_chars = read(pipefd, &(msg[1]), BUF_SIZE -1)) == -1)
	    die("read(output_redirection)");
	if(write(socketfd, msg, BUF_SIZE) == -1)
	    die("write(output_redirection)");
    }
    return NULL;
}

/* The function blocks the caller, until all output of the previously run client was redirected
 *
 *
 */
void wait_for_output_redirection(int pipe_fd) {
    int poll_value;
    struct pollfd fds;
    fds.fd = pipe_fd;
    fds.events = POLLIN;
    fds.revents = POLLIN;
    do {
	// We will continue, when our pipe is not ready anymore,
	//   because we know our program stopped running and doesn't
	//   produce output anymore. When the pipe is not ready, there is nothing to read.
	// The timeout should be very small for this reason
	if((poll_value = poll(&fds, 1, 0)) == -1)
	    die("poll");
	if(poll_value != 0)
	    sched_yield(); // Let's give our output thread a chance
    } while(poll_value != 0);
	    
}

// In contrast to output_redirection we do not want input_redirection to
//  to be a thread that is active for the whole session.
// Instead it should only be active for one program run.
// The reason is, that we want to discard any input from the user, when a program is not running
//   and we do not want this function to read anything, which is not meant for redirection
void* input_redirection(void* data) {
    int pipefd = ((int*) data)[0];
    int socketfd = ((int*) data)[1];
    char msg[BUF_SIZE];
    int read_chars;

    while(1) {
	if((read_chars = read(socketfd, &msg, BUF_SIZE)) == -1)
	    die("read(input_redirection)");
		
	if(msg[0] != C_INPUT)
	    die("invalid op_code(input_redirection)");
	
	if(write(pipefd, &(msg[1]), read_chars) == -1)
	    die("write(input_redirection)");
	
    }
        
    return NULL;
}

/* We create two new processes:
 *     the first one is the running program
 *     the second is used for redirecting the output to the client
 */
void run(thread_block_t* tb, char** tokens, int pipefd[2]) {
    pid_t run_pid;
    int wait_status;
    pthread_t input_channel;
    int input_pipe[2];
    if(pipe(input_pipe) != 0)
	die("pipe");
    int arg[2];
    arg[0] = input_pipe[1];
    arg[1] = tb->cfd;
   
    pthread_create(&input_channel, 0, &input_redirection, arg);
    
    if((run_pid = fork()) == 0) {
	if(dup2(pipefd[1], STDOUT_FILENO) == -1)
	    die("dup2(run_stdout)");
	if(dup2(input_pipe[0], STDIN_FILENO) == -1)
	    die("dup2(run_stdout)");
	execvp(tokens[0], tokens);
	die("execvp(run)");
    }

    waitpid(run_pid, &wait_status, 0);
    wait_for_output_redirection(pipefd[0]);
    pthread_cancel(input_channel);
    pthread_join(input_channel, NULL);
    close(input_pipe[0]);
    close(input_pipe[1]);
}

#define BUILTIN_NUM 5
// The order MATTERS!
const char * builtin[] = {
    "cd",
    "exit",
    "get",
    "put",
    "wall"
};
enum {BI_CD=0, BI_EXIT, BI_GET, BI_PUT, BI_WALL};
int builtin_dict(char* token) {
  for(int i=0; i<BUILTIN_NUM; i++) {
    if(strcmp(builtin[i], token) == 0)
       return i;
  }
  return -1;
}

// Getting a line, where the delim was already replace by \0 characters,
//   place a pointer at the beginning of each token
int split_line(char** tokens, char* line, int len) {
    tokens[0] = &(line[0]);

    int found_delim = 0;
    int token_count = 1;
    for(int i=1; i<len; i++) {
	if(found_delim) {
	    if(line[i] == '\0')
		return token_count;
	    tokens[token_count++] = &(line[i]);
	    found_delim = 0;
	}
	if(line[i] == '\0')
	    found_delim = 1;
    }
    tokens[token_count] = NULL;

    return token_count;
}

void* session(void* data) {
    char msg[BUF_SIZE];
    thread_block_t* tb = (thread_block_t*) data;
    int len;

    // There can't be more tokens than BUF_SIZE/2, as every second character is a delim
    char** tokens = malloc(sizeof(char*) * (BUF_SIZE/2));
    if(tokens == NULL)
	die("malloc");

    int pipefd[2];
    if(pipe(pipefd) != 0)
	die("pipe");
    
    pthread_t output_thread;
    
    int output_args[2];
    output_args[0] = pipefd[0];
    output_args[1] = tb->cfd;
    pthread_create(&output_thread, NULL, &output_redirection, &output_args);

    while(1) {
	for(int i=0; i<BUF_SIZE; i++)
	    msg[i] = 0;
	
	len = read(tb->cfd, msg, BUF_SIZE);
	if(len < 0)
	    break;

	switch(msg[0]) {
	case C_EXIT:
	    printf("Terminated connection with client on socket %d\n", tb->cfd);
	    terminate_thread(tb);
	    free(tokens);
	    pthread_cancel(output_thread);
	    close(pipefd[0]);
	    close(pipefd[1]);
	    return NULL;

	    // We discard input packets from previous sessions
	case C_INPUT:
	    continue; 
	default:
	    break;
	}
	
       	int token_count = split_line(tokens, &(msg[1]), len);
	int choice = builtin_dict(tokens[0]);
	
	switch(choice) {
	case BI_CD:
	    bi_cd(tb, tokens, token_count);
	case BI_EXIT:
	    die("Invalid message: sent exit with incorrent message type.\n");
	case BI_GET:
	    bi_get(tokens, token_count);
	    break;
	case BI_PUT:
	    bi_put(tokens, token_count);
	    break;
	case BI_WALL:
	    bi_wall(tokens, token_count);
	    break;
	default:
	    run(tb, tokens, pipefd);
	    
	    msg[0] = S_PRG_TERM;
	    if(write(tb->cfd, msg, BUF_SIZE) == -1)
		die("write");
	}
    }
    
    terminate_thread(tb);

    return NULL;
}
