#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utility.h"
#include "sharedMemory.h"


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


int getIndexFromType(char *busType) {
    if (!strcmp(busType, "VOR")) {
        return 0;
    } else if (!strcmp(busType, "ASK")) {
        return 1;
    } else if (!strcmp(busType, "PEL")) {
        return 2;
    } else {
        perror("getIndexFromType: Invalid busType given.");
        exit(EXIT_FAILURE);
    }
}


int getCapacityByBayType(char *shmPointer, char *bayType) {
    return *(((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET)) + getIndexFromType(bayType));
}


void formatTime(time_t timestamp, char timestampStr[25]) {
    strftime(timestampStr, 25, "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
}


double calculateDivision(double value1, double value2) {
    if (value2 == 0) {
        return 0;
    } else {
        return value1 / value2;
    }
}
