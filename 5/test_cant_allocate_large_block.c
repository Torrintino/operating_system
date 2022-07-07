
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
    size_t block_size = heap_size/8;
        
    mem_alloc(block_size);
    mem_alloc(block_size);
    mem_alloc(block_size);
    mem_alloc(block_size);
    mem_alloc(block_size);
    char* p = mem_alloc(heap_size/2);
    if(p != NULL) {
	printf("Test failed\n");
	return -1;
    }
    
    printf("Test succeeded\n");

    return 0;
}
