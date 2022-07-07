#include "memory.h"

#include <stdio.h>

int main(int argc, char* argv[]) {
    int t[] = {0, 1, 2, 3, 5, 16, 63, 1234};
    int r[] = {0, 1, 2, 4, 8, 16, 64, 2048};

    int succeeded = 0;
    for(int i=0; i<8; i++) {
	int actual = binary_ceil(t[i]);
	if(actual != r[i]) {
	    fprintf(stderr, "%d = binary_ceil(%d) != %d\n", actual, t[i], r[i]);
	} else {
	    succeeded++;
	}
    }

    if(succeeded == 8) {
	printf("Test succeeded\n");
    } else {
	printf("Test failed\n");
    }

    return 0;
}
