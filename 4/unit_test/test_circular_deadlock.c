
#include "../ult.h"

#include <stdio.h>

int tid = -1;

void join_main() {
    int own_tid = tid + 2;
    int status;
    tid++;
    printf("Thread %d attempting to join %d\n", own_tid, tid);
    ult_join(tid, &status);
    printf("main returned with status %d\n", status);
}

int main() {
    int status;
    ult_init(&join_main);
    for(int i=1; i<10; i++)
	ult_spawn(&join_main);
    printf("Thread %d attempting to join %d\n", 0, 10);
    ult_join(tid, &status);
    printf("thread returned with status %d\n", status);
    return 0;
}
