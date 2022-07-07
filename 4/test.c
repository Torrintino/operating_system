#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ult.h"

#define READ_SIZE 64

// Thread A will only terminate before b in the case of an error
// But then B needs to know, that A is not running anymore
int threada_terminated = 0;
int threadb_terminated = 0;

unsigned long long bytes_read = 0;
time_t start_time;
time_t time_elapsed;
double bytes_per_sec;


static void threadA() {
    char buf[READ_SIZE];
    start_time = time(NULL);
    if(start_time == -1) {
	perror("time");
	exit(-1);
    }
    int random_fd = open("/dev/random", O_RDONLY);
    int null_fd = open("/dev/null", O_WRONLY);
    
    while(!threadb_terminated) {
	if(read(random_fd, buf, READ_SIZE) == -1) {
	    perror("read");
	    ult_exit(-1);
	}
	if(write(null_fd, buf, READ_SIZE) == -1) {
	    perror("write");
	    ult_exit(-1);
	}
	bytes_read += READ_SIZE;
	time_t tmp = time(NULL);
	if(tmp == -1) {
	    perror("time");
	    exit(-1);
	}
	time_elapsed = tmp - start_time + 1;
	bytes_per_sec = bytes_read / (double) time_elapsed;

	if(bytes_read >= ULONG_MAX - READ_SIZE) {
	    fprintf(stderr,
		    "Cannot read any more characters, without creating a buffer overflow\n");
	    ult_exit(-1);
	}
	ult_yield();
    }

    close(random_fd);
    close(null_fd);
    ult_exit(0);
}

/* WARNING: This function behaves in a really strange way on terminal emulators.
 * Using input redirection generates the expected results.
 *
 * The suspected cause of the error is, that the terminal buffers its data and doesn't send
 * it to stdin as one chunk. To solve this, one might need to implement this function in a
 * way, that reads character by character. (On xterm the program is more stable than on 
 * Gnome Terminal)
 */

static void threadB() {
    // Longest possible sequence: "stats\n" 6 characters
    int read_size = 6;
    char buf[read_size];
    char time_buffer_start[26];

    while(!threada_terminated) {
	ult_yield();
	int rc = ult_read(STDIN_FILENO, buf, read_size);
	
	if(rc <= 0) {
	    fprintf(stderr, "Invalid input\n");
	    ult_exit(-1);
	}
	
	if(buf[rc - 1] != '\n') {
	    fprintf(stderr, "Invalid input\n");
	    continue;
	}
	
	buf[rc - 1] = '\0';
	if(strcmp(buf, "exit") == 0) {
	    threadb_terminated = 1;
	    ult_exit(0);
	} else if(strcmp(buf, "stats") == 0) {
	    struct tm* tm_info_start = localtime(&start_time);
	    const char* date_format = "%Y-%m-%d %H:%M:%S";
	    strftime(time_buffer_start, 26, date_format, tm_info_start);
	    printf("Number of bytes read: %llu\n"
		   "Start time: %s\n"
		   "Time elapsed: %ld seconds\n"
		   "Throughput: %f bytes per second\n",
		   bytes_read,
		   time_buffer_start,
		   time_elapsed,
		   bytes_per_sec);
	} else {
	    fprintf(stderr, "Invalid input\n");
	    continue;
	}
    }
}

static void myInit()
{
	int tids[2], i, status;
	
	printf("spawn A\n");
	tids[0] = ult_spawn(threadA);
	printf("spawn B\n");
	tids[1] = ult_spawn(threadB);
	
	for (i = 0; i < 2; ++i)
	{
		printf("waiting for tids[%d] = %d\n", i, tids[i]);
		fflush(stdout);
		
		if (ult_join(tids[i], &status) < 0)
		{
			fprintf(stderr, "join for %d failed\n", tids[i]);
			ult_exit(-1);
		}
		
		printf("(status = %d)\n", status);
	}
	
	ult_exit(0);
}

int main()
{
	ult_init(myInit);
	printf("All threads terminated\n");
	return 0;
}
