#include "memory.h"
#include "bitset.h"

/**@brief Heap memory. */
static char heap[HEAP_SIZE];

unsigned int used_sections;
struct section {
    int avail; // The largest block, which is available
    struct section* lb;
    struct section* rb;
} section[TREE_SIZE];

void mem_init() {
    section[0].lb = NULL;
    section[0].rb = NULL;
    section[0].avail = HEAP_SIZE;
    used_sections = 1;
}

// If size is a power of two, return size
// Otherwise return size rounded up to the lowest power of two,
//     which is larger than size.
size_t binary_ceil(size_t size) {
    if(size == 0) {
	fprintf(stderr, "Invalid size\n");
	return 0;
    }

    int pow = 0;
    int is_odd = 0;
    while(size > 1) {
	if(size & 1)
	    is_odd = 1;
	pow++;
	size /= 2;
    }
    if(is_odd)
	pow++;
    return 1 << pow;
}

// For given order, return the size of the block
size_t order_size(int order) {
    size_t size = PAGE_SIZE;
    for(; order > 0; order--)
	size *= 2;
    return size;
}


int init_buddy(struct section* s, int order) {
    if(used_sections + 2 > TREE_SIZE) {
	fprintf(stderr,
		"Used up all space for system data structures\n");
	return -1;
    }
    
    s->lb = &(section[used_sections]);
    s->rb = &(section[used_sections + 1]);
    used_sections += 2;
    
    s->lb->avail = order_size(order - 1);
    s->rb->avail = order_size(order - 1);
    
    s->lb->lb = NULL;
    s->lb->rb = NULL;
    s->rb->lb = NULL;
    s->rb->rb = NULL;
    
    return 0;
}

int max(int a, int b) {
    return a > b? a : b;
}

void* mem_alloc_rec(size_t size, struct section* s, int order, int pos) {
    if(order_size(order) == size || order == 0) {
	s->avail = 0;
	return &(heap[pos]);
    } else {
	void* p;
	
	if(s->lb == NULL) {
	    if(init_buddy(s, order) == -1)
		return NULL;
	}
	    
	if(s->lb->avail >= size) {
	    p = mem_alloc_rec(size, s->lb, order - 1, pos);
	} else if(s->rb->avail >= size) {
	    // We make following assumptions:
	    //     Whenever s->lb gets allocated, so does s->rb, so it
	    //         can't be NULL.
	    p = mem_alloc_rec(size, s->rb, order - 1, pos + s->avail);

	} else {
	    fprintf(stderr, "There is no block available, which satisfies the"
		    " request block size. This is a library error.\n");
	    return NULL;
	}
	if(p != NULL)
	    s->avail = max(s->lb->avail, s->rb->avail);
	return p;
    }
}

void* mem_alloc(size_t size) {
    if(size == 0) {
	fprintf(stderr, "Size must be larger than 0\n");
	return NULL;
    }
    
    size = binary_ceil(size);
    if(size > order_size(ORDER_MAX)) {
	fprintf(stderr, "Requested size is larger than the heap\n");
	return NULL;
    }
    if(section[0].avail < size) {
	fprintf(stderr, "There is no block available, which satisfies the"
		" request block size\n");
	return NULL;
    }
    
    return mem_alloc_rec(size, section, ORDER_MAX, 0);
}

void* mem_realloc(void* oldptr, size_t new_size) {
	/* TODO: increase the size of an existing block through merging or
	 *       allocate a new one with identical contents */
	return oldptr;
}


void mem_free_rec(char* p, struct section* s, int order, int pos) {
    int block_size = order_size(order);
    int p_index = p - heap;

    // Check if the index of the pointer is in the right or left block
    if(p_index >= pos + block_size/2) {
	if(s->rb == NULL) {
	    // The pointer can't point to the right block, if it was not split
	    fprintf(stderr,
		    "Attempt to free a block, which was not allocated\n");
	    return;
	}
	mem_free_rec(p, s->rb, order - 1, block_size/2);
    } else {
	if(s->lb == NULL) {
	    // In this case, the current block is not split
	    //     so the given pointer must be here
	    
	    if(p_index != pos){
		fprintf(stderr,
			"Attempt to free a block, which was not allocated\n");
		return;
	    }

	    s->avail = block_size;
	    return;
	}

	mem_free_rec(p, s->lb, order - 1, pos);
    }

    s->avail = max(s->lb->avail, s->rb->avail);
    if(s->lb->avail == block_size/2 && s->rb->avail == block_size/2) {
	// Now we can merge the two blocks, because non is allocated
	// We overwrite the sections with the last ones in our array
	//     too free up unused system resources
	int lb_index = section - s->lb;
	int rb_index = section - s->rb;
	section[lb_index] = section[--used_sections];
	section[rb_index] = section[--used_sections];
	s->lb = NULL;
	s->rb = NULL;
	s->avail = block_size;
    }
}

void mem_free(void* ptr) {
    if(ptr == NULL)
	return;
    if((char*) ptr < heap || (char*) ptr >= &(heap[HEAP_SIZE-1])) {
	fprintf(stderr, "Attempt to free a block, which was not allocated\n");
	return;
    }
    
    mem_free_rec(ptr, section, ORDER_MAX, 0);
}

void mem_dump(FILE* file) {
	/* TODO: print the current state of the allocator */
}
