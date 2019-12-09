#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>

#include "utility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    char *busType = argv[1];
    int incpassengers = atoi(argv[2]);
    int capacity = atoi(argv[3]);
    int parkperiod = atoi(argv[4]);
    int mantime = atoi(argv[5]);
    int shmid = atoi(argv[6]);
    int busIndex = getpid();
    int passengersBoardedCount;

    char *shmPointer = (char *) attachToSharedMemory(shmid);

    sem_t *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    sem_t *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    sem_t *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    sem_t *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);

    printf("Bus %d: Up(busesMux)\n", busIndex);
    sem_post(busesMux);

    printf("Bus %d: Down(stationManagerIncomingMux)\n", busIndex);
    sem_wait(stationManagerIncomingMux);

    printf("Bus %d: ---- Communicating incoming with station-manager ----\n\n", busIndex);

    printf("Bus %d: Write %s\n", busIndex, busType);
    strcpy(shmPointer + BUSTYPE_OFFSET, busType);
    sem_post(messageSentMux);

    sem_wait(messageReadMux);
    printf("Bus %d: Read parking bay %s\n", busIndex, shmPointer + BAYTYPE_OFFSET);

    printf("\n");

    // manouver in

    printf("Bus %d: Waiting %d sec for passengers\n", busIndex, parkperiod);
    srand(time(0));
    passengersBoardedCount = (rand() % capacity) + 1;
    sleep(parkperiod);

    printf("Bus %d: Down(stationManagerOutgoingMux)\n", busIndex);
    sem_wait(stationManagerOutgoingMux);

    printf("Bus %d: ---- Communicating outgoing with station-manager ----\n\n", busIndex);

    printf("Bus %d: Sent asdasdasdasa\n", busIndex);

    printf("\n");

    // manouver out

    printf("Bus %d: Left the station\n", busIndex);

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
