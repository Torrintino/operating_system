#include "../ult.h"

#include <stdio.h>


#define ARRAY_SIZE 10
int array[ARRAY_SIZE];
int idx = 0;

void count() {
    printf("Begin counting\n");
    int j = idx++;
    array[j] = 0;
    for(int i=0; i<10; i++)
	array[j]++;
    printf("Finished counting\n");
    ult_exit(0);
}

void myInit() {
    for(int i=1; i<=ARRAY_SIZE; i++)
	ult_spawn(&count);
    
    int status;
    for(int i=1; i<=ARRAY_SIZE; i++) {
	printf("Attempt to join %d\n", i);
	ult_join(i, &status);
	printf("Thread returned with status %d\n", status);
    }
    
    int sum = 0;
    for(int i=0; i<ARRAY_SIZE; i++)
	sum += array[i];
    printf("Count: %d\n", sum);
    ult_exit(0);
}

int main() {    
    printf("Creating threads\n");
    ult_init(&myInit);
    
    return 0;
}
