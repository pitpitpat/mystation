#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "stationManagerUtility.h"
#include "utility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);
    int bayCapacityPerType[3];
    sem_t *stationManagerIncomingMux, *stationManagerOutgoingMux, *busesMux, *messageSentMux, *messageReadMux, *incomingBusesCountMux, *outgoingBusesCountMux;
    int *incomingBusesCount, *outgoingBusesCount, *incomingManTime, *outgoingManTime;
    int incomingBusesCountValue, outgoingBusesCountValue;

    char *shmPointer = (char *) attachToSharedMemory(shmid);
    getSemaphores(shmPointer, &stationManagerIncomingMux, &stationManagerOutgoingMux, &busesMux, &messageSentMux, &messageReadMux, &incomingBusesCountMux, &outgoingBusesCountMux);

    incomingBusesCount = (int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET);
    outgoingBusesCount = (int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET);
    incomingManTime = (int *) (shmPointer + INCOMINGMANTIME_OFFSET);
    outgoingManTime = (int *) (shmPointer + OUTGOINGMANTIME_OFFSET);
    memcpy(bayCapacityPerType, shmPointer + BAYCAPACITYPERTYPE_OFFSET, BAYCAPACITYPERTYPE_SIZE);

    int busesLeft = 6;
    while(1) {
        printf("     AFTER SLEEP INCOMING: %d OUTGOING: %d\n", *incomingManTime, *outgoingManTime);
        sem_wait(busesMux);

        if (*incomingManTime == 0) {

            sem_wait(incomingBusesCountMux);
            incomingBusesCountValue = *incomingBusesCount;
            if (incomingBusesCountValue > 0) (*incomingBusesCount)--;
            sem_post(incomingBusesCountMux);

            if (incomingBusesCountValue > 0) {
                serveIncomingBus(shmPointer, stationManagerIncomingMux, messageSentMux, messageReadMux, bayCapacityPerType);
            } else {
                printf("SManager: Sleep outgoingManTime %d sec\n", *outgoingManTime);
                sleep(*outgoingManTime);
                *outgoingManTime = 0;

                sem_wait(outgoingBusesCountMux);
                (*outgoingBusesCount)--;
                sem_post(outgoingBusesCountMux);

                serveOutgoingBus(shmPointer, stationManagerOutgoingMux, messageSentMux, messageReadMux, bayCapacityPerType);
                busesLeft--;
            }

        } else if (*outgoingManTime == 0) {

            sem_wait(outgoingBusesCountMux);
            outgoingBusesCountValue = *outgoingBusesCount;
            if (outgoingBusesCountValue > 0) (*outgoingBusesCount)--;
            sem_post(outgoingBusesCountMux);

            if (outgoingBusesCountValue > 0) {
                serveOutgoingBus(shmPointer, stationManagerOutgoingMux, messageSentMux, messageReadMux, bayCapacityPerType);
                busesLeft--;
            } else {
                printf("SManager: Sleep incomingManTime %d sec\n", *incomingManTime);
                sleep(*incomingManTime);
                *incomingManTime = 0;

                sem_wait(incomingBusesCountMux);
                (*incomingBusesCount)--;
                sem_post(incomingBusesCountMux);

                serveIncomingBus(shmPointer, stationManagerIncomingMux, messageSentMux, messageReadMux, bayCapacityPerType);
            }

        }

        if (busesLeft == 0) break;

        printf("    BEFORE SLEEP INCOMING: %d OUTGOING: %d\n", *incomingManTime, *outgoingManTime);
        sleepUntilOneLaneIsOpen(incomingManTime, outgoingManTime);
    }

    printBaysCapacity(shmPointer);

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
