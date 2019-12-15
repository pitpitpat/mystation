#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "busUtility.h"
#include "sharedMemory.h"
#include "utility.h"


void getCommandLineArguments(int argc, char **argv, char **arguments) {
    for (int i = 1; i < (argc - 1); i++) {
        if (!strcmp(argv[i], "-t")) {
            arguments[0] = argv[i + 1];
        } else if (!strcmp(argv[i], "-n")) {
            arguments[1] = argv[i + 1];
        } else if (!strcmp(argv[i], "-c")) {
            arguments[2] = argv[i + 1];
        } else if (!strcmp(argv[i], "-p")) {
            arguments[3] = argv[i + 1];
        } else if (!strcmp(argv[i], "-m")) {
            arguments[4] = argv[i + 1];
        } else if (!strcmp(argv[i], "-s")) {
            arguments[5] = argv[i + 1];
        }
    }
}


int getRandomInteger(int maxValue) {
    srand(time(NULL));
    return (rand() % maxValue) + 1;
}


int waitForNewPassengers(int maxParkPeriod, int maxPassengerCapacity) {
    int parkingDuration = getRandomInteger(maxParkPeriod);
    int passengersBoardedCount = getRandomInteger(maxPassengerCapacity);
    // printf("Bus %d: Waiting %d sec for passengers\n", getpid(), parkingDuration);
    sleep(parkingDuration);
    return passengersBoardedCount;
}


void getServiceForEntranceByStationManager(char *shmPointer, char *busType, int passengersCount, int mantime, char *parkingBayType, int *parkingIsleIndex) {
    sem_t *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    sem_t *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);
    pid_t busPid = getpid();

    // printf("\n---- Communicating Entrance ----\n");
    // printf("Bus %d: Write busType %s passengersCount %d mantime %d\n", busPid, busType, passengersCount, mantime);
    memcpy(shmPointer + BUSTYPE_OFFSET, busType, BUSTYPE_SIZE);
    memcpy(shmPointer + BUSPID_OFFSET, &busPid, BUSPID_SIZE);
    memcpy(shmPointer + PASSENGERSCOUNT_OFFSET, &passengersCount, PASSENGERSCOUNT_SIZE);
    memcpy(shmPointer + INCOMINGMANTIME_OFFSET, &mantime, INCOMINGMANTIME_SIZE);
    sem_post(messageSentMux);

    // station-manager is reading message and writing response
    sem_wait(messageReadMux);

    memcpy(parkingBayType, shmPointer + BAYTYPE_OFFSET, BAYTYPE_SIZE);
    memcpy(parkingIsleIndex, shmPointer + ISLEINDEX_OFFSET, ISLEINDEX_SIZE);
    // printf("Bus %d: Read parkingBayType %s parkingIsleIndex %d\n\n", busPid, parkingBayType, *parkingIsleIndex);
    sem_post(messageSentMux);
}


void getServiceForDepartureByStationManager(char *shmPointer, char *busType, int passengersBoardedCount, char *parkingBayType, int parkingIsleIndex, int mantime) {
    sem_t *messageSent2Mux = (sem_t *) (shmPointer + MESSAGESENT2MUTEX_OFFSET);
    sem_t *messageRead2Mux = (sem_t *) (shmPointer + MESSAGEREAD2MUTEX_OFFSET);
    pid_t busPid = getpid();

    // printf("\n---- Communicating Departure----\n");
    // printf("Bus %d: Write busType %s passengersBoardedCount %d parkingBayType %s parkingIsleIndex %d mantime %d\n", busPid, busType, passengersBoardedCount, parkingBayType, parkingIsleIndex, mantime);
    memcpy(shmPointer + BUSTYPE_OFFSET, busType, BUSTYPE_SIZE);
    memcpy(shmPointer + BUSPID_OFFSET, &busPid, BUSPID_SIZE);
    memcpy(shmPointer + PASSENGERSCOUNT_OFFSET, &passengersBoardedCount, PASSENGERSCOUNT_SIZE);
    memcpy(shmPointer + BAYTYPE_OFFSET, parkingBayType, BAYTYPE_SIZE);
    memcpy(shmPointer + ISLEINDEX_OFFSET, &parkingIsleIndex, ISLEINDEX_SIZE);
    memcpy(shmPointer + OUTGOINGMANTIME_OFFSET, &mantime, OUTGOINGMANTIME_SIZE);
    sem_post(messageSent2Mux);

    // station-manager is reading message and writing response
    sem_wait(messageRead2Mux);

    sem_post(messageSent2Mux);
}


void maneuver(int duration) {
    // printf("Bus %d: Maneuver for %d sec\n", getpid(), duration);
    sleep(duration);
}
