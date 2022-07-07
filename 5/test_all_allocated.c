
#include "memory.h"

#include <stdio.h>


#ifndef TEST
#define TEST
#define PAGE_SIZE 16
#define ORDER_MAX 5
#endif

int main() {
    size_t heap_size = PAGE_SIZE << ORDER_MAX;
    mem_init();
    char* p1 = mem_alloc(heap_size);
    if(p1 == NULL) {
	printf("Test failed\n");
	return -1;
    }
    char* p2 = mem_alloc(1);
    if(p2 != NULL) {
	printf("Test failed\n");
	return -1;
    }
    printf("Test succeeded\n");

    return 0;
}
