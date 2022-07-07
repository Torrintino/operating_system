#include "process.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int waiting_for_process = 0; // Whether or not a foreground process is being run
pid_t* fg = NULL; // current foreground processes
int fg_num = 0;

#define PROCESS_CHUNK 4
children_t* init_children() {
  children_t* children = malloc(sizeof(children_t));
  if(children == NULL) {
    perror("malloc");
    return NULL;
  }
  
  children->p = malloc(sizeof(pid_t*) * PROCESS_CHUNK);
  if(children->p == NULL) {
    perror("malloc");
    free(children);
    return NULL;
  }

  fg = malloc(sizeof(pid_t));
  if(fg == NULL) {
    perror("malloc");
    free(children->p);
    free(children);
    return NULL;
  }
  
  children->allocated_size = PROCESS_CHUNK;
  children->size = 0;
  return children;

  
}

void process_cleanup(children_t* children) {
  free(children->p);
  free(children);
  free(fg);
}

children_t* add_child(children_t* children, pid_t child) {
  if(children->size == children->allocated_size) {
    children->allocated_size += PROCESS_CHUNK;
    children->p = realloc(children->p, children->allocated_size);
    if(children->p == NULL) {
      perror("malloc");
      free(children);
      return NULL;
    }
  }
  children->p[children->size] = child;
  children->size++;
  
  return children;
}

void sigint_handler(int sig) {
  printf("\n");
  if(waiting_for_process) {
    for(int i=0; i<fg_num; i++)
      kill(fg[i], SIGINT);
  } else {
    // We don't clean up memory for two reasons:
    // 1. Per default after SIGINT our program can't clean up anyways
    //    and the OS does that for us
    // 2. Doing so would require a goto statement and is dangerous
    exit(1);
  }
}

int run_pipe(parsed_line_t* parsed_line) {
  static int allocated_fg_size = 1;

  if(parsed_line->pipe_num <= 0) {
    fprintf(stderr, "Bad function call (run_pipe): the command has no pipes\n");
    return 1;
  }
  /*
  if(allocated_pipe_size < parsed_line->pipe_num) {
    allocated_pipe_size = (sizeof(int)) * parsed_line->pipe_num;
    pipe_fd = realloc(pipe_fd, allocated_pipe_size);
    if(pipe_fd == NULL) {
      perror("realloc");
      return 1;
    }
    }*/

  if(allocated_fg_size < parsed_line->pipe_num + 1) {
    allocated_fg_size = sizeof(int) * (parsed_line->pipe_num + 1);
    fg = realloc(fg, allocated_fg_size);
    if(fg == NULL) {
      perror("realloc");
      return 1;
    }
  }
  fg_num = parsed_line->pipe_num + 1;
    
  int pipe_fd[2];
  for(int i=0; i<=parsed_line->pipe_num; i++) {
    if(i < parsed_line->pipe_num) {
      if(pipe(pipe_fd)) {
	perror("pipe");
	return 1;
      }
    }
    
    if((fg[i] = fork()) == 0) {
      if(i != 0)
	dup2(STDIN_FILENO, pipe_fd[0]);
      if(i != parsed_line->pipe_num)
	dup2(STDOUT_FILENO, pipe_fd[1]);

      if(i == 0) {
 	execvp(parsed_line->line[0], parsed_line->line);
	
	perror("execvp");
	return 1;
	
      } else {
        int index = parsed_line->pipe[i] + 1;
        execvp(parsed_line->line[index], &(parsed_line->line[index]));
	perror("execvp");
	return 1;
      }
      
    }
  }

  int ret = 0;
  waiting_for_process = 1;
  for(int i=0; i<=parsed_line->pipe_num; i++) {
    int status;
    waitpid(fg[i], &status, 0);
    
    if(WEXITSTATUS(status)) {
      fprintf(stderr, "Process %d failed with return status %d\n",
	      fg[i], status);
      ret = 1;
    }
  }
  waiting_for_process = 0;
  
  return ret;
}

int run(parsed_line_t* parsed_line) {
  int status;

  fg_num = 1;
  if((fg[0] = fork()) == 0) {
    execvp(parsed_line->line[0], parsed_line->line);
    perror("execvp");
    return 1;
  }
  
  // Is there a race condition here?
  waiting_for_process = 1;
  waitpid(fg[0], &status, 0);
  waiting_for_process = 0;
  if(WEXITSTATUS(status)) {
    fprintf(stderr, "Process %s failed with return status %d\n",
	    parsed_line->line[0], status);
    return 1;
  }

  return 0;
}

children_t* run_async(children_t* children, parsed_line_t* parsed_line) {
  pid_t pid;
  if((pid = fork()) == 0) {
    execvp(parsed_line->line[0], &(parsed_line->line[0]));
    perror("execvp");
    return children;
  }
  return add_child(children, pid);
}

void check_async(children_t* children) {
  int status;
  for(int i=0; i<children->size; i++) {
    if(waitpid(children->p[i], &status, WNOHANG)) {
      if(WEXITSTATUS(status)) {
	fprintf(stderr, "[%d] failed with return status %d\n",
	    children->p[i], status);
      } else {
	printf("[%d] exited\n", children->p[i]);
      }
      children->size--;
      children->p[i] = children->size;
      
      // Since we moved an element, we need to check it again, now at the correct index
      // If this was already the last element, we also decreased the size, so the loop
      //   will terminate. 
      i--;
    }
  }
}
