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
    int *incomingBusesCount, *outgoingBusesCount;

    char *shmPointer = (char *) attachToSharedMemory(shmid);

    sem_t *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    sem_t *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    sem_t *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    sem_t *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);
    sem_t *incomingBusesCountMux = (sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET);
    sem_t *outgoingBusesCountMux = (sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET);

    incomingBusesCount = (int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET);
    outgoingBusesCount = (int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET);



    ///////////////////// Incoming /////////////////////

    sem_wait(incomingBusesCountMux);
    (*incomingBusesCount)++;
    sem_post(incomingBusesCountMux);

    sem_post(busesMux);
    sem_wait(stationManagerIncomingMux);



    printf("\nBus %d: ---- Communicating incoming with station-manager ----\n", busIndex);

    printf("Bus %d: Write %s\n", busIndex, busType);
    strcpy(shmPointer + BUSTYPE_OFFSET, busType);
    memcpy(shmPointer + INCOMINGMANTIME_OFFSET, &mantime, INCOMINGMANTIME_SIZE);
    sem_post(messageSentMux);

    // station-manager is reading message and writing response
    sem_wait(messageReadMux);

    printf("Bus %d: Read parking bay %s\n\n", busIndex, shmPointer + BAYTYPE_OFFSET);
    sem_post(messageSentMux);

    // manouver in
    printf("Bus %d: Manouver in for %d sec\n", busIndex, mantime);
    sleep(mantime);




    ///////////////////// PARK, LEAVE AND BOARD PASSENGERS /////////////////////

    printf("Bus %d: Waiting %d sec for passengers\n", busIndex, parkperiod);
    srand(time(0));
    passengersBoardedCount = (rand() % capacity) + 1;
    sleep(parkperiod);




    ///////////////////// Outgoing /////////////////////

    sem_wait(outgoingBusesCountMux);
    (*outgoingBusesCount)++;
    sem_post(outgoingBusesCountMux);

    sem_post(busesMux);
    sem_wait(stationManagerOutgoingMux);



    printf("\nBus %d: ---- Communicating outgoing with station-manager ----\n", busIndex);

    printf("Bus %d: Write %s\n", busIndex, busType);
    strcpy(shmPointer + BUSTYPE_OFFSET, busType);
    memcpy(shmPointer + OUTGOINGMANTIME_OFFSET, &mantime, OUTGOINGMANTIME_OFFSET);
    sem_post(messageSentMux);

    // station-manager is reading message and writing response
    sem_wait(messageReadMux);

    printf("Bus %d: Read response\n\n", busIndex);
    sem_post(messageSentMux);

    // manouver out
    printf("Bus %d: manouver out for %d sec\n", busIndex, mantime);
    sleep(mantime);


    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
