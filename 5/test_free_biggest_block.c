#include "memory.h"

#include <stdio.h>

int main() {
    mem_init();
    void* p = mem_alloc(HEAP_SIZE);
    mem_free(p);
    
    return 0;
}
