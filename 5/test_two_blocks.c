
#include "memory.h"

#include <stdio.h>

int main() {
    size_t heap_size = PAGE_SIZE << ORDER_MAX;
    mem_init();
    size_t block_size = heap_size/2;
    char* p1 = mem_alloc(block_size);
    char* p2 = mem_alloc(block_size);
    if(p1 == NULL || p2 == NULL) {
	printf("Test failed\n");
	return -1;
    }
    if(p2 != (p1 + block_size)) {
	printf("Test failed (Incorrect values)\n");
	return -1;
    }
    p1[0] = 'a';
    p1[block_size-1] = 'b';
    p2[0] = 'A';
    p2[block_size-1] = 'B';
    printf("%c%c%c%c\n", p1[0], p1[block_size-1], p2[0], p2[block_size-1]);

    mem_free(p1);
    mem_free(p2);
    printf("Test succeeded\n");
    return 0;
}
