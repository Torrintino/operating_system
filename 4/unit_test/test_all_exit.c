
#include "../ult.h"

#include <stdio.h>

void child() {
    printf("Child exits\n");
    ult_exit(0);
}

void myInit() {
    int tid = ult_spawn(&child);
    int status;
    ult_join(tid, &status);
    printf("Child returned with status %d\n", status);
    printf("Main exits\n");
    ult_exit(0);
}


int main() {
    ult_init(&myInit);
    return 0;
}
