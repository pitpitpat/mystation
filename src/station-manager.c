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
    int stationManagerIncomingValue, stationManagerOutgoingValue;

    char *parkingBay;

    sem_t *stationManagerIncomingMux = callAndCheckSemOpen(sem_open(STATIONMANAGERINCOMINGMUTEX, O_RDWR));
    sem_t *stationManagerOutgoingMux = callAndCheckSemOpen(sem_open(STATIONMANAGEROUTGOINGMUTEX, O_RDWR));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_RDWR));
    sem_t *messageSentMux = callAndCheckSemOpen(sem_open(MESSAGESENTMUTEX, O_RDWR));
    sem_t *messageReadMux = callAndCheckSemOpen(sem_open(MESSAGEREADMUTEX, O_RDWR));

    char *shmPointer = (char *) attachToSharedMemory(shmid);
    memcpy(bayCapacityPerType, shmPointer + BAYCAPACITYPERTYPEOFFSET, BAYCAPACITYPERTYPESIZE);

    int busesLeft = 6;
    while(1) {
        if (busesLeft <= 0) break;
        printf("SManager: Down(busesMux)\n");
        sem_wait(busesMux);

        printf("SManager: Sleep 3 sec\n");
        sleep(3);

        printf("sem_getvalue: %d\n", sem_getvalue(stationManagerIncomingMux, &stationManagerIncomingValue));
        perror("sem_getvalue");
        printf("sem_getvalue: %d\n", sem_getvalue(stationManagerOutgoingMux, &stationManagerOutgoingValue));
        perror("sem_getvalue");

        printf("Sem stationManagerIncomingValue value %d\n", stationManagerIncomingValue);
        printf("Sem stationManagerOutgoingValue value %d\n", stationManagerOutgoingValue);

        if (stationManagerIncomingValue < 0) {
            printf("SManager: ---- Communicating incoming with bus ----\n\n");

            busesLeft--;

            printf("SManager: Up(stationManagerIncomingMux)\n");
            sem_post(stationManagerIncomingMux);

            printf("\n");

            sem_wait(messageSentMux);
            printf("SManager: Read %s\n", shmPointer + BUSTYPEOFFSET);
            if (bayCapacityPerType[getIndexFromBusType(shmPointer + BUSTYPEOFFSET)] > 0) {
                parkingBay = shmPointer + BUSTYPEOFFSET;
                bayCapacityPerType[getIndexFromBusType(shmPointer + BUSTYPEOFFSET)]--;
                memcpy(shmPointer + BAYCAPACITYPERTYPEOFFSET, bayCapacityPerType, BAYCAPACITYPERTYPESIZE);
            } else if (((!strcmp(shmPointer + BUSTYPEOFFSET, "VOR")) || (!strcmp(shmPointer + BUSTYPEOFFSET, "ASK"))) && bayCapacityPerType[getIndexFromBusType("PEL")] > 0) {
                parkingBay = "PEL";
                bayCapacityPerType[2]--;
                memcpy(shmPointer + BAYCAPACITYPERTYPEOFFSET, bayCapacityPerType, BAYCAPACITYPERTYPESIZE);
            } else {
                parkingBay = "NONE";
            }

            if (strcmp(parkingBay, "NONE") != 0) {
                printf("SManager: Bay %s capacity %d -> %d\n", parkingBay, bayCapacityPerType[getIndexFromBusType(parkingBay)] + 1, bayCapacityPerType[getIndexFromBusType(parkingBay)]);
            } else {
                printf("No parking bay for you!\n");
            }

            memcpy(shmPointer + BAYOFFSET, parkingBay, BAYSIZE);
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

    printf("SManager: VOR capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPEOFFSET)));
    printf("SManager: ASK capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPEOFFSET + sizeof(int))));
    printf("SManager: PEL capacity = %d\n", *((int *) (shmPointer + BAYCAPACITYPERTYPEOFFSET + 2 * sizeof(int))));

    callAndCheckInt(sem_close(stationManagerIncomingMux), "sem_close");
    callAndCheckInt(sem_close(stationManagerOutgoingMux), "sem_close");
    callAndCheckInt(sem_close(busesMux), "sem_close");
    callAndCheckInt(sem_close(messageSentMux), "sem_close");
    callAndCheckInt(sem_close(messageReadMux), "sem_close");
    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
