#include "memory.h"

#include <stdio.h>

#define TEST_CASE_NUM 5

int main() {
    int t[] = {0, 1, 2, 3, 4};
    int r[] = {16, 32, 64, 128, 256};

    int succeeded = 0;
    for(int i=0; i<TEST_CASE_NUM; i++) {
	int actual = order_size(t[i]);
	if(actual != r[i]) {
	    fprintf(stderr, "%d = order_size(%d) != %d\n", actual, t[i], r[i]);
	} else {
	    succeeded++;
	}
    }

    if(succeeded == TEST_CASE_NUM) {
	printf("Test succeeded\n");
    } else {
	printf("Test failed\n");
    }

    return 0;
}
