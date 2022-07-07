#include "ult.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define _XOPEN_SOURCE
#include <ucontext.h>

#define STACK_SIZE 64*1024

/* thread control block */
struct tcb_s {
    ucontext_t* thread; // Generated threads
    ucontext_t caller;
    ucontext_t dummy;
            
     // The thread that is active, or was active before it yielded
    unsigned int active_thread;

    // All TID's are must be lower than tid_upper_bound (which is not constant)
    unsigned int tid_upper_bound;
    unsigned int threads_running;

    // The following invariant must always be true:
    //     threads_running <= tid_upper_bound <= allocated
    
    char** mem;
    int joining;
    int* joinable;
    int* status;
    unsigned int allocated;
} tcb;


#define THREAD_CHUNK 8
void allocate_memory() {
    int old_size = tcb.allocated;
    
    if(tcb.allocated == 0) {
	tcb.mem = NULL;
	tcb.thread = NULL;
	tcb.joinable = NULL;
    }
    tcb.allocated += THREAD_CHUNK;

    tcb.thread = realloc(tcb.thread, sizeof(ucontext_t) * tcb.allocated);
    if(tcb.thread == NULL) {
	perror("realloc");
	exit(-1);
    }

    tcb.joinable = realloc(tcb.joinable, sizeof(ucontext_t*) * tcb.allocated);
    if(tcb.joinable == NULL) {
	perror("realloc");
	exit(-1);
    }
    
    tcb.status = realloc(tcb.status, sizeof(ucontext_t*) * tcb.allocated);
    if(tcb.status == NULL) {
	perror("realloc");
	exit(-1);
    }

    tcb.mem = realloc(tcb.mem, sizeof(char*) * tcb.allocated);
    if(tcb.mem == NULL) {
	perror("realloc");
	exit(-1);
    }
    
    for(int i=old_size; i<tcb.allocated; i++) {
	tcb.mem[i] = malloc(sizeof(char) * STACK_SIZE);
	if(tcb.mem[i] == NULL) {
	    perror("realloc");
	    exit(-1);
	}
    }
}

void prepare_thread(unsigned int tid, ult_f f) {
    // Prepare TC
    if(getcontext(&(tcb.thread[tid])) == -1) {
	perror("getcontext");
	exit(-1);
    }

    tcb.thread[tid].uc_link = NULL;
    tcb.thread[tid].uc_stack.ss_flags = 0;
    tcb.thread[tid].uc_stack.ss_size = STACK_SIZE;
    tcb.thread[tid].uc_stack.ss_sp = tcb.mem[tid];
    
    makecontext(&(tcb.thread[tid]), f, 0);

    tcb.joinable[tid] = 0;
}

/* This is expected to be only called once per user process.
 * Thread 0 is always the caller thread and may never be joined.
 * Thread 1 will be the first thread executing f.
 * The context for the caller will be saved in thread 0, using swapcontext.
 */
void ult_init(ult_f f) {
    tcb.allocated = 0;
    allocate_memory();

    prepare_thread(0, f);

    tcb.tid_upper_bound = 1;
    tcb.threads_running = 1;
    tcb.active_thread = 0;
    tcb.joining = 0;

    if(swapcontext(&tcb.caller, &(tcb.thread[0])) == -1) {
	perror("setcontext");
	exit(-1);
    }
}

int ult_spawn(ult_f f) {
    tcb.threads_running++;
    
    // For all threads, check if they exited and were joined (joinable == 2).
    //   If such a thread was found, overwrite it.
    //   Otherwise use a new slot and allocate memory, if necessary.
    int pos;
    for(pos=0; pos < tcb.tid_upper_bound; pos++)	{
	if(tcb.joinable[pos] == 2)
	    break;
    }
    if(pos == tcb.allocated)
	allocate_memory();
    if(pos == tcb.tid_upper_bound)
	tcb.tid_upper_bound++;

    prepare_thread(pos, f);
    
    return pos;	
}

void ult_yield() {
    unsigned int old_tid = tcb.active_thread;
    
    for(int i = tcb.active_thread + 1;
	i <= tcb.active_thread + tcb.tid_upper_bound;
	i++) {
	unsigned int tid = i % tcb.tid_upper_bound;
	
	if(tcb.joinable[tid])
	    continue;
	
	tcb.active_thread = tid;
	if(swapcontext(&(tcb.thread[old_tid]), &(tcb.thread[tid])) == -1) {
	    perror("setcontext");
	    exit(-1);
	}
	return;
    }
    
    fprintf(stderr, "Unexpected end: no thread can be scheduled, as "
	    "all threads are joined or waiting to be joined\n"); 
    exit(-1);
}

void ult_exit(int status) {
    tcb.threads_running--;

    if(tcb.threads_running == 0) {
	free(tcb.thread);
	free(tcb.joinable);
	free(tcb.status);
	for(int i=0; i<tcb.allocated; i++)
	    free(tcb.mem[i]);
	free(tcb.mem);
	swapcontext(&tcb.dummy, &tcb.caller);
    }
    
    tcb.status[tcb.active_thread] = status;
    tcb.joinable[tcb.active_thread] = 1;

    ult_yield();
}

int ult_join(int tid, int* status) {
    tcb.joining++;

    if(tid > tcb.tid_upper_bound)
	return -1;
    if(tcb.joinable[tid] == 2)
	return -1;

    if(tcb.joining == tcb.threads_running && !(tcb.joinable[tid])) {
	fprintf(stderr, "Deadlock: All running threads are attempting to join "
		"another thread\n");
	exit(-1);
    }
    
    while(!(tcb.joinable[tid]))
	ult_yield();

    tcb.joinable[tid] = 2;
    *status = tcb.status[tid];
    tcb.joining--;
    return 0;
}

ssize_t ult_read(int fd, void* buf, size_t size) {
    struct pollfd pfd = { .fd=fd, .events = POLLIN};
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    do {
	if(poll(&pfd, 1, 0) == -1) {
	    perror("poll");
	    exit(-1);
	}
	if(pfd.revents & POLLIN)
	    break;
	ult_yield();
    } while(!(pfd.revents & POLLIN));

    ssize_t count;
    if((count = read(fd, buf, size)) == -1) {
	perror("read");
	exit(-1);
    }

    return count;
}
