#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);
    int bayCapacityPerType[3];
    int stationManagerIncomingValue, stationManagerOutgoingValue;
    char *parkingBay;

    char *shmPointer = (char *) attachToSharedMemory(shmid);

    sem_t *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    sem_t *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    sem_t *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    sem_t *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);

    memcpy(bayCapacityPerType, shmPointer + BAYCAPACITYPERTYPE_OFFSET, BAYCAPACITYPERTYPE_SIZE);

    int busesLeft = 6;
    while(1) {
        if (busesLeft <= 0) break;
        printf("SManager: Down(busesMux)\n");
        sem_wait(busesMux);

        printf("SManager: Sleep 2 sec\n");
        sleep(2);

        sem_getvalue(stationManagerIncomingMux, &stationManagerIncomingValue);
        sem_getvalue(stationManagerOutgoingMux, &stationManagerOutgoingValue);

        if (stationManagerIncomingValue < 0) {
            printf("SManager: ---- Communicating incoming with bus ----\n\n");

            printf("SManager: Up(stationManagerIncomingMux)\n");
            sem_post(stationManagerIncomingMux);

            printf("\n");

            sem_wait(messageSentMux);
            printf("SManager: Read %s\n", shmPointer + BUSTYPE_OFFSET);
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

            printf("\n");
        }

        if (stationManagerOutgoingValue < 0) {
            printf("SManager: ---- Communicating outgoing with bus ----\n\n");

            busesLeft--;

            printf("SManager: Up(stationManagerIncomingMux)\n");
            sem_post(stationManagerOutgoingMux);

            printf("\n");
        }

    }

    printf("SManager: VOR capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET)));
    printf("SManager: ASK capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET + sizeof(int))));
    printf("SManager: PEL capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET + 2 * sizeof(int))));

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
