#ifndef LINE_H
#define LINE_H


typedef struct {
  char** line; // line[i] is the ith token, poiting into the given line
  unsigned int size; // the number of tokens in line

  // pipe[i] is the index of the ith pipe in line
  // meaning line[pipe[i]] connects program line[pipe[i] + 1] with the
  //  program before the pipe
  unsigned int pipe_num; // number of pipes
  unsigned int* pipe;
  int is_async; // is set to 1, if the last character was a '&'
} parsed_line_t;

char* get_line();
parsed_line_t* parse_line(char* line);

#endif
