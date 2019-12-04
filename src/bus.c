#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "sharedSemaphores.h"


int main(int argc, char *argv[]) {
    int busIndex = atoi(argv[1]);
    int shmid = atoi(argv[2]);

    sem_t *stationManagerMux = callAndCheckSemOpen(sem_open(STATIONMANAGERMUTEX, O_RDWR));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_RDWR));

    printf("Bus %d: shmid %d\n", busIndex, shmid);

    printf("Bus %d: Up(busesMux)\n", busIndex);
    sem_post(busesMux);

    printf("Bus %d: Down(stationManagerMux)\n", busIndex);
    sem_wait(stationManagerMux);

    printf("\nBus %d: Sleeping...\n\n", busIndex);
    sleep(4);

    callAndCheckInt(sem_close(stationManagerMux), "sem_close");

    return 0;
}
