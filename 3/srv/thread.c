#include "global.h"
#include "thread.h"

#define THREAD_CHUNK 8
thread_block_t* init_threads() {
    thread_block_t* tb = malloc(sizeof(thread_block_t) * THREAD_CHUNK);
    if(tb == NULL)
	die("malloc");
    return tb;
}

thread_block_t* realloc_threads(thread_block_t* tb, int threads_running) {
    static int allocated_size = THREAD_CHUNK;

    if(threads_running == allocated_size) {
	allocated_size += THREAD_CHUNK;
	tb = realloc(tb, sizeof(thread_block_t*) * allocated_size);
	if(tb == NULL)
	    die("realloc");
    }

    return tb;
}

int check_threads(thread_block_t* tb, int threads_running) {
    for(int i=0; i<threads_running; i++) {
	if(!tb[i].running) {
	    pthread_join(tb[i].thread, NULL);
	    threads_running--;

	    // Consistency check
	    if(threads_running > 0 && i < threads_running) {
		tb[i].thread = tb[threads_running].thread;
		tb[i].workdir = tb[threads_running].workdir;
	    }

	    // When we moved the last item to the current position, we need to check this
	    //   position again. This MUST be the line of the current IF statement.
	    i--;
	}
    }
    
    return threads_running;
}

void destroy_threads(thread_block_t* tb, int threads_running) {
    for(int i=0; i<threads_running; i++) {
	free(tb[i].workdir);
	close(tb[i].cfd);
    }
    free(tb);
}

void terminate_thread(thread_block_t* tb) {
    tb->running = 0;
    free(tb->workdir);
    close(tb->cfd);
}

void create_thread(thread_block_t* tb, int pos, int cfd) {
    tb[pos].cfd = cfd;
    tb[pos].running = 1;
    tb[pos].workdir = getcwd(NULL, 0);
    if(tb[pos].workdir == NULL)
	die("getcwd");
    
    if(pthread_create(&tb[pos].thread,
		      NULL,
		      &session,
		      (void*) &tb[pos]))
	die("pthread_create");
}
