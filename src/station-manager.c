#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "stationManagerUtility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    char *arguments[2];
    int bayCapacityPerType[3];
    int *incomingManTime, *outgoingManTime;
    int incomingBusesCount, outgoingBusesCount;

    getCommandLineArguments(argc, argv, arguments);
    int busesLeft = atoi(arguments[0]);
    int shmid = atoi(arguments[1]);

    char *shmPointer = (char *) attachToSharedMemory(shmid);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    incomingManTime = (int *) (shmPointer + INCOMINGMANTIME_OFFSET);
    outgoingManTime = (int *) (shmPointer + OUTGOINGMANTIME_OFFSET);
    memcpy(bayCapacityPerType, shmPointer + BAYCAPACITYPERTYPE_OFFSET, BAYCAPACITYPERTYPE_SIZE);

    while(1) {
        sem_wait(busesMux);

        if (*incomingManTime == 0) {

            incomingBusesCount = decreaseIncomingBusesCount(shmPointer);
            if (incomingBusesCount > 0) {
                serveIncomingBus(shmPointer, bayCapacityPerType, &busesLeft);
            } else {
                sleep(*outgoingManTime);
                *outgoingManTime = 0;

                decreaseOutgoingBusesCount(shmPointer);
                serveOutgoingBus(shmPointer, bayCapacityPerType, &busesLeft);
            }

        } else if (*outgoingManTime == 0) {

            outgoingBusesCount = decreaseOutgoingBusesCount(shmPointer);
            if (outgoingBusesCount > 0) {
                serveOutgoingBus(shmPointer, bayCapacityPerType, &busesLeft);
            } else {
                sleep(*incomingManTime);
                *incomingManTime = 0;

                decreaseIncomingBusesCount(shmPointer);
                serveIncomingBus(shmPointer, bayCapacityPerType, &busesLeft);
            }

        }

        if (busesLeft == 0) {
            sleep(*outgoingManTime);
            break;
        }

        sleepUntilOneLaneIsOpen(incomingManTime, outgoingManTime);
    }

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
