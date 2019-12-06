#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "sharedSemaphores.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    int busIndex = atoi(argv[1]);
    int shmid = atoi(argv[2]);
    char *busType = argv[3];

    sem_t *stationManagerMux = callAndCheckSemOpen(sem_open(STATIONMANAGERMUTEX, O_RDWR));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_RDWR));
    sem_t *messageSentMux = callAndCheckSemOpen(sem_open(MESSAGESENTMUTEX, O_RDWR));
    sem_t *messageReadMux = callAndCheckSemOpen(sem_open(MESSAGEREADMUTEX, O_RDWR));

    char *shmPointer = (char *) attachToSharedMemory(shmid);

    printf("Bus %d: Up(busesMux)\n", busIndex);
    sem_post(busesMux);

    printf("Bus %d: Down(stationManagerMux)\n", busIndex);
    sem_wait(stationManagerMux);


    printf("\n");
    printf("Bus %d: Write %s\n", busIndex, busType);
    strcpy(shmPointer, busType);
    sem_post(messageSentMux);
    sem_wait(messageReadMux);
    printf("\n");


    callAndCheckInt(sem_close(stationManagerMux), "sem_close");
    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
