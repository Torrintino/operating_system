
#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define TEST
#ifdef TEST
#define PAGE_SIZE 16
#define ORDER_MAX 3
#endif

#ifndef TEST
/**@brief Order of the largest possible memory block. */
#define ORDER_MAX 10

/**@brief Size of the smallest possible memory block. */
#define PAGE_SIZE (1u << 6)

#endif

/**@brief Size of available memory. */
#define HEAP_SIZE (PAGE_SIZE << ORDER_MAX)

#define PAGE_NUM HEAP_SIZE/PAGE_SIZE
#define TREE_SIZE PAGE_NUM*PAGE_NUM


size_t binary_ceil(size_t size);
size_t order_size(int order);

/**@brief Initializes the memory allocator to a consistent state.
 */
extern void mem_init();

/**@brief Allocates a block of memory, returning a pointer.
 */
extern void* mem_alloc(size_t size);

/**@brief Increases or decreases the amount of allocated memory.
 */
extern void* mem_realloc(void* oldptr, size_t new_size);

/**@brief Frees an allocated block of memory.
 */
extern void mem_free(void* ptr);

/**@brief Prints free pages for every order.
 */
extern void mem_dump(FILE* file);

#endif
