#ifndef THREADS_H
#define THREADS_H


typedef struct {
    pthread_t thread;
    char* workdir;
    int running;
    int cfd;
} thread_block_t;

thread_block_t* init_threads();
thread_block_t* realloc_threads(thread_block_t* tb, int threads_running);
int check_threads(thread_block_t* tb, int threads_running);

// This will probably not get called in the rsh, as it will usually be killed.
// However, I think it is good style to have this function defined, just in case.
void destroy_threads(thread_block_t* tb, int threads_running);

void create_thread(thread_block_t* tb, int pos, int cfd);

// Clean up a single thread
void terminate_thread(thread_block_t* tb);

// Defined in server.c
extern void* session(void* data);

#endif
