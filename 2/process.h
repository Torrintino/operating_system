#ifndef PROCESS_H
#define PROCESS_H

#include "line.h"

#include <unistd.h>

typedef struct {
  pid_t* p;
  int size;
  int allocated_size;
} children_t;

void sigint_handler(int sig);

children_t* init_children();
void process_cleanup(children_t* children);

// Check if any asynchronous processes have exited
void check_async(children_t* children);
children_t* add_child(children_t* children, pid_t child);

int run(parsed_line_t* parsed_line);
int run_pipe(parsed_line_t* parsed_line);
children_t* run_async(children_t* children, parsed_line_t* parsed_line);
#endif
