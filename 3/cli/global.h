#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>

static inline void die(const char* msg) {
    perror(msg);
    exit(-1);
}

#endif
