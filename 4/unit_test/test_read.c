
#include "../ult.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int pipefd[2];

void consumer() {
    char msg[27];
    ult_read(pipefd[0], msg, 27);
    puts(msg);
    ult_exit(0);
}

void producer() {
    char c[27];
    
    for(int i=0; i<26; i++) {
	c[i] = 'a' + i;
	ult_yield();
    }
    c[26] = '\0';
    write(pipefd[1], c, 27);
    ult_exit(0);
}

void myInit() {
    if(pipe(pipefd) == -1) {
	perror("pipe");
	exit(-1);
    }
    int status;

    printf("Spawning consumer\n");
    int consumer_tid = ult_spawn(&consumer);
    printf("Spawning producer\n");
    int producer_tid = ult_spawn(&producer);
    
    ult_join(producer_tid, &status);
    printf("producer exited with status %d\n", status);
    
    ult_join(consumer_tid, &status);
    printf("consumer exited with status %d\n", status);

    close(pipefd[0]);
    close(pipefd[1]);
    ult_exit(0);
}

int main() {
    ult_init(&myInit);
    
    return 0;
}
