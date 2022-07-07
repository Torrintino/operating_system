
#include "../ult.h"

#include <stdio.h>

#define CHILD_NUM 10
int tid = 1;

void join_main() {
    int own_tid = tid;
    int status;
    tid++;
    if(tid == CHILD_NUM + 1)
	tid = 0;
    printf("Thread %d attempting to join %d\n", own_tid, tid);
    ult_join(tid, &status);
    printf("main returned with status %d\n", status);
}

void myInit() {
    int status;
    for(int i=0; i<CHILD_NUM; i++)
	ult_spawn(&join_main);
    printf("Thread %d attempting to join %d\n", 0, CHILD_NUM);
    ult_join(CHILD_NUM, &status);
    printf("thread returned with status %d\n", status);
    
    ult_exit(0);
}

int main() {
    ult_init(&myInit);
    return 0;
}
