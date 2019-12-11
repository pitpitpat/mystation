#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "stationManagerUtility.h"
#include "sharedMemory.h"
#include "utility.h"


void getSemaphores(char *shmPointer, sem_t **stationManagerIncomingMux, sem_t **stationManagerOutgoingMux, sem_t **busesMux, sem_t **messageSentMux, sem_t **messageReadMux, sem_t **incomingBusesCountMux, sem_t **outgoingBusesCountMux) {
    *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);
    *incomingBusesCountMux = (sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET);
    *outgoingBusesCountMux = (sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET);
}


void serveIncomingBus(char *shmPointer, sem_t *stationManagerIncomingMux, sem_t *messageSentMux, sem_t *messageReadMux, int bayCapacityPerType[3]) {
    char *parkingBay;

    sem_post(stationManagerIncomingMux);

    // bus is writing info
    sem_wait(messageSentMux);

    if (bayCapacityPerType[getIndexFromBusType(shmPointer + BUSTYPE_OFFSET)] > 0) {
        parkingBay = shmPointer + BUSTYPE_OFFSET;
        bayCapacityPerType[getIndexFromBusType(shmPointer + BUSTYPE_OFFSET)]--;
        memcpy(shmPointer + BAYCAPACITYPERTYPE_OFFSET, bayCapacityPerType, BAYCAPACITYPERTYPE_SIZE);
    } else if (((!strcmp(shmPointer + BUSTYPE_OFFSET, "VOR")) || (!strcmp(shmPointer + BUSTYPE_OFFSET, "ASK"))) && bayCapacityPerType[getIndexFromBusType("PEL")] > 0) {
        parkingBay = "PEL";
        bayCapacityPerType[2]--;
        memcpy(shmPointer + BAYCAPACITYPERTYPE_OFFSET, bayCapacityPerType, BAYCAPACITYPERTYPE_SIZE);
    } else {
        parkingBay = "NONE";
    }
    if (strcmp(parkingBay, "NONE") != 0) {
        printf("SManager: Bay %s capacity %d -> %d\n", parkingBay, bayCapacityPerType[getIndexFromBusType(parkingBay)] + 1, bayCapacityPerType[getIndexFromBusType(parkingBay)]);
    } else {
        printf("No parking bay for you!\n");
    }
    memcpy(shmPointer + BAYTYPE_OFFSET, parkingBay, BAYTYPE_SIZE);
    sem_post(messageReadMux);

    // bus is reading the response
    sem_wait(messageSentMux);
}


void serveOutgoingBus(char *shmPointer, sem_t *stationManagerOutgoingMux, sem_t *messageSentMux, sem_t *messageReadMux, int bayCapacityPerType[3]) {
    sem_post(stationManagerOutgoingMux);

    // bus is writing bus type
    sem_wait(messageSentMux);

    printf("SManager: Read %s\n", shmPointer + BUSTYPE_OFFSET);
    sem_post(messageReadMux);

    // bus is reading the response
    sem_wait(messageSentMux);
}


void sleepUntilOneLaneIsOpen(int *incomingManTime, int *outgoingManTime) {
    if (*incomingManTime > 0 && *outgoingManTime > 0) {
        if (*incomingManTime <= *outgoingManTime) {
            sleep(*incomingManTime);
            *outgoingManTime -= *incomingManTime;
            *incomingManTime = 0;
        } else {
            sleep(*outgoingManTime);
            *incomingManTime -= *outgoingManTime;
            *outgoingManTime = 0;
        }
    }
}


void printBaysCapacity(char *shmPointer) {
    printf("SManager: VOR capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET)));
    printf("SManager: ASK capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET + sizeof(int))));
    printf("SManager: PEL capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET + 2 * sizeof(int))));
}
