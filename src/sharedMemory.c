#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "sharedMemory.h"


void * attachToSharedMemory(int shmid) {
    void *shmPointer = (char *) shmat(shmid, (void *)0, 0);
    if (shmPointer == (void *) -1) {
        perror("Attachment error");
        exit(EXIT_FAILURE);
    }
    return shmPointer;
}
