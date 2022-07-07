#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "line.h"
#include "process.h"

#define BUILTIN_NUM 3
// The order MATTERS!
const char * builtin[] = {
    "cd",
    "exit",
    "wait",
};
enum {BI_CD=0, BI_EXIT, BI_WAIT};
int builtin_dict(char* input) {
  for(int i=0; i<BUILTIN_NUM; i++) {
    if(strcmp(builtin[i], input) == 0)
       return i;
  }
  return -1;
}

int bi_cd(const char* path) {
  if(chdir(path)) {
    perror("chdir");
    return 1;
  }
  
  return 0;
}

int bi_wait(char** argv) {
  return 0;
}

int main(int argc, char* argv[]) {
  char* line;
  parsed_line_t* parsed_line;
  int RET_CODE = 1;
  
  // using color codes
  const char* prompt = "\x1b[31m./>\x1b[0m ";
  
  if(signal(SIGINT, sigint_handler) == SIG_ERR) {
    perror("signal");
    return 1;
  }

  children_t* children = init_children();
  
  while(1) {
    check_async(children);
    
    fputs(prompt, stdout);
    if((line = get_line()) == NULL)
      return 1;
    
    // Skip, if the user entered an empty line
    if(line[0] == 0)
      continue;
    
    parsed_line = parse_line(line);
    if(parsed_line == NULL)
      return 1;
    
    int i = builtin_dict(parsed_line->line[0]);
    if(i >= 0) {
      if(parsed_line->pipe_num) {
	fprintf(stderr, "Built in commands cannot be executed with pipes\n");
	return 1;
      }
      if(parsed_line->is_async) {
	fprintf(stderr,
		"Built in commands cannot be executed asynchronously\n");
	return 1;
      }
      
      switch(i) {
      case BI_CD:
	if(parsed_line->size != 2) {
	  fprintf(stderr,
		  "'cd' needs to be called with exactly one argument\n");
	  break;
	}
	bi_cd(parsed_line->line[1]);
	break;
      case BI_EXIT:
	RET_CODE = 0;
	goto CLEAN_AND_EXIT;
      case BI_WAIT:
	/*
	if(parsed_line->size <= 1) {
	  fprintf(stderr,
		  "'wait' needs to be called with at least one argument\n");
	  continue;
	}
	for(int i=1; i<parsed_line->size; i++) {
	  char* c;
	  pid_t p = strtol(parsed_line->line[i], &c, 10);
	  if(*c != '\0' || errno == EINVAL || errno == ERANGE) {
	    fprintf(stderr, "Argument %d is not a valid PID\n", i);
	    continue;
	  }
	  if(p == 0) {
	    fprintf(stderr, "Cannot wait for PID 0\n");
	    continue;
	  }
	  int index = -1;
	  for(int i=0; i<child_num; i++) {
	    if(child[i] == p) {
	      index = i;
	      break;
	    }
	  }
	  if(index == -1) {
	    fprintf(stderr, "PID %d is not a child of this process\n", i);
	    continue;
	  }
	}
	*/
	bi_wait(parsed_line->line);
	break;
      }
    } else {
      if(parsed_line->is_async && parsed_line->pipe_num > 0) {
	printf("Not implemented");
      } else if(parsed_line->is_async) {
	run_async(children, parsed_line);
      } else if(parsed_line->pipe_num > 0) {
	run_pipe(parsed_line);
      } else {
	run(parsed_line);
      }
    } 
  }
  
 CLEAN_AND_EXIT:
  free(parsed_line->line);
  free(parsed_line->pipe);
  free(parsed_line);
  free(line);
  process_cleanup(children);
  return RET_CODE;
}
