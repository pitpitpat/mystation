#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


int getIndexFromBusType(char *busType) {
    if (!strcmp(busType, "VOR")) {
        return 0;
    } else if (!strcmp(busType, "ASK")) {
        return 1;
    } else if (!strcmp(busType, "PEL")) {
        return 2;
    } else {
        perror("getIndexFromBusType: Invalid busType given.");
        exit(EXIT_FAILURE);
    }
}
