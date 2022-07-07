
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
    char* p = mem_alloc(heap_size);
    if(p == NULL) {
	printf("Test failed\n");
	return -1;
    }
    p[0] = 'a';
    p[heap_size/2] = '-';
    p[heap_size-1] = 'z';
    printf("%c%c%c\nTest succeeded\n", p[0], p[heap_size/2], p[heap_size-1]);

    return 0;
}
