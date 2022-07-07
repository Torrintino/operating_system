#include "../ult.h"

#include <stdio.h>

void child() {
    char mem[27];
    for(int i=0; i<26; i++) {
	ult_yield();
	mem[i] = 'a' + i;
    }
    mem[26] = '\0';
    puts(mem);
    ult_exit(0);
}

int main() {
    ult_init(&child);
    ult_spawn(&child);
    int status;
    ult_join(1, &status);
    printf("Child exited with status %d\n", status);
    ult_join(2, &status);
    printf("Child exited with status %d\n", status);
    return 0;
}
