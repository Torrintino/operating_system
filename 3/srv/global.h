#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline void die(const char* msg) {
    perror(msg);
    exit(-1);
}

#endif
