
#include "memory.h"

#include <stdio.h>


#define ORDER_MAX 10
#define PAGE_SIZE (1u << 6)

int main() {
    mem_init();
    size_t heap_size = PAGE_SIZE << ORDER_MAX;
    int page_num = heap_size/PAGE_SIZE;
    for(int i=0; i<page_num; i++) {
	char* p = mem_alloc(PAGE_SIZE);
	if(p == NULL) {
	    printf("Test failed at block %d\n", i);
	    return -1;
	}
    }

    char* p = mem_alloc(PAGE_SIZE);
    if(p != NULL) {
	printf("Test failed\n");
	return -1;
    }

    printf("Test succeeded\n");
    return 0;
}
