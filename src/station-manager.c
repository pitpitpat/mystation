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
    int shmid = atoi(argv[1]);
    int bayCapacityPerType[3];

    sem_t *stationManagerMux = callAndCheckSemOpen(sem_open(STATIONMANAGERMUTEX, O_RDWR));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_RDWR));
    sem_t *messageSentMux = callAndCheckSemOpen(sem_open(MESSAGESENTMUTEX, O_RDWR));
    sem_t *messageReadMux = callAndCheckSemOpen(sem_open(MESSAGEREADMUTEX, O_RDWR));

    char *shmPointer = (char *) attachToSharedMemory(shmid);
    memcpy(bayCapacityPerType, shmPointer + BAYCAPACITYPERTYPEOFFSET, BAYCAPACITYPERTYPESIZE);

    int loop = 6;
    while(loop--) {
        printf("SManager: Down(busesMux)\n");
        sem_wait(busesMux);

        printf("SManager: Sleep 3 sec\n");
        sleep(3);

        printf("SManager: Up(stationManagerMux)\n");
        sem_post(stationManagerMux);


        printf("\n");
        sem_wait(messageSentMux);
        printf("SManager: Read %s\n", shmPointer + BUSTYPEOFFSET);
        printf("SManager: Bay capacity %d\n", bayCapacityPerType[getIndexFromBusType(shmPointer + BUSTYPEOFFSET)]);
        sem_post(messageReadMux);
        printf("\n");
    }

    callAndCheckInt(sem_close(stationManagerMux), "sem_close");
    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
