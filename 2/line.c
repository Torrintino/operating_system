#include "line.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE_CHUNK 80

/* Safely get a arbitrarily long line from stdin
 * Read one character at a time
 * The line is a static data structure, so we have to call fewer characters
 *  and don't need to make duplicate allocations
 * Returns NULL upon error
 */
char* get_line() {
  static int line_size = 0;
  static char* line = NULL;

  if(line_size == 0) {
    line_size += LINE_CHUNK;
    line = malloc(sizeof(char) * line_size);
    if(line == NULL)
      return NULL;
  }
  
  char c;
  int i = 0;

  while((c = getc(stdin)) != '\n') {
    // IMPORTANT NOTE
    // We don't check, whether the input character is valid or not
    // Since we cannot make many assumptions on that
    // We decided to be rather liberal regarding this and delegate the
    //   responsibility to the user and sub-processes
    
    if(i == line_size) {
      line_size += LINE_CHUNK;
      line = realloc(line, line_size);
      if(line == NULL)
	return NULL;
    }
    line[i] = c;
    i++;
  }

  line[i] = 0;
  return line;
}

// Separate the line into tokens
// Must also check, whether the expression is well formed
// Returns NULL upon error
#define TOKEN_CHUNK 16
parsed_line_t* parse_line(char* line) {
  if(line == NULL)
    return NULL;

  // Static variables to reuse allocated memory
  static int allocated_size = 0;
  static parsed_line_t* parsed_line = NULL;
  
  // Allocating memory for data structure 
  if (parsed_line == NULL) {
    parsed_line = malloc(sizeof(parsed_line_t));
    if(parsed_line == NULL)
      goto ALLOC_FAIL_LINE;
    
    parsed_line->line = malloc(sizeof(char*) * TOKEN_CHUNK);
    if(parsed_line->line == NULL)
      goto ALLOC_FAIL_TOKENS;
    allocated_size += TOKEN_CHUNK;
    parsed_line->pipe = malloc(sizeof(unsigned int) * (TOKEN_CHUNK / 2));
    if(parsed_line->pipe == NULL)
      goto ALLOC_FAIL_PIPE;
  }

  parsed_line->pipe_num = 0;
  parsed_line->is_async = 0;

  // IMPORTANT NOTE
  // The pointer returned from strtok points to the line
  // The delimiter gets replaced by a \0 char
  parsed_line->line[0] = strtok(line, " ");
  if(parsed_line->line[0] == NULL)
    goto EMPTY_LINE;
  
  int size=0;
  do {
    size++;
    if(size >= allocated_size) {
      allocated_size += TOKEN_CHUNK;
      parsed_line->line = realloc(parsed_line->line,
				  sizeof(char*) * allocated_size);
      if(parsed_line->line == NULL)
	goto ALLOC_FAIL_TOKENS;
      
      // There can only be less than half as many pipes as there are arguments
      parsed_line->pipe = realloc(parsed_line->line,
				  sizeof(char*) * (allocated_size/2));
      if(parsed_line->pipe == NULL)
	goto ALLOC_FAIL_PIPE;
    }
    parsed_line->line[size] = strtok(NULL, " ");
  } while(parsed_line->line[size] != NULL);

  // The last token must not have a format like 'abc&def' or '&abc'
  // Set async flag, if & is the last character
  // Then delete & to transform a call like './a.out&' to './a.out'
  for(char* c = parsed_line->line[size-1]; *c != 0; c++) {
    if(*c == '&' && *(c+1) != 0)
      goto FORMAT_ERROR_AND;
    if(*c == '&') {
      parsed_line->is_async = 1;
      *c = 0;
      if(parsed_line->line[size-1][0] == 0) {
	size--;
	parsed_line->line[size] = NULL;
      }
    }
    if(*c == '|')
      goto FORMAT_ERROR_PIPE;
  }
  parsed_line->size = size;
  
  // Scan one word at a time for special characters and format errors
  for(int i=0; i<size; i++) {

    for(int j=0; parsed_line->line[i][j] != 0; j++) {
	
      if(parsed_line->line[i][j] == '&')
	goto FORMAT_ERROR_AND;

      // | must be standalone
      if(parsed_line->line[i][j] == '|') {
	if(i == 0)
	  goto FORMAT_ERROR_PIPE;
	  
	if(j != 0 || parsed_line->line[i][j+1] != 0)
	  goto FORMAT_ERROR_PIPE;

	if(i == size-2 && parsed_line->line[i+1][0] == 0)
	  goto FORMAT_ERROR_PIPE;
	  
	if(parsed_line->pipe_num > 0 && i > 0) { // Prohibit '| |'
	  if(parsed_line->pipe[parsed_line->pipe_num - 1] == i-1) 
	    goto FORMAT_ERROR_PIPE;
	}

	parsed_line->pipe[parsed_line->pipe_num] = i;
	parsed_line->pipe_num++;	  
      }
      
    }
    
  }

  for(int i=0; i<parsed_line->pipe_num; i++)
    parsed_line->line[parsed_line->pipe[i]] = NULL;
  parsed_line->line[parsed_line->size] = NULL;

  return parsed_line;

 ALLOC_FAIL_PIPE:
  free(parsed_line->line);
 ALLOC_FAIL_TOKENS:
  free(parsed_line);
 ALLOC_FAIL_LINE:
  fprintf(stderr, "Memory allocation error\n");
  return NULL;

 FORMAT_ERROR_AND:
  fprintf(stderr, "Format error: The '&' can only be the last "
	  "character of the input string\n");
  goto DIE;

 FORMAT_ERROR_PIPE:
  fprintf(stderr, "Format error: The '|' must not be part of a word and "
	  "must stand between two program calls\n");
  goto DIE;

 EMPTY_LINE:
  fprintf(stderr, "Empty line cannot be parsed\n");
  goto DIE;
  
 DIE:
  free(parsed_line->pipe);
  free(parsed_line);
  free(parsed_line->line);
  return NULL;
}

