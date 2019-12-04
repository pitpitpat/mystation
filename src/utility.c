#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "utility.h"


void* callAndCheckPointer(void *response, char *errorName) {
    if (response == NULL){
        perror(errorName);
        exit(EXIT_FAILURE);
    }
    return response;
}


int callAndCheckInt(int response, char *errorName) {
    if (response == -1){
        perror(errorName);
        exit(EXIT_FAILURE);
    }
    return response;
}

void * callAndCheckSemOpen(void *response) {
    if (response == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    return response;
}
