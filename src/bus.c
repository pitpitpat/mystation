#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "busUtility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    int parkingIsleIndex, passengersBoardedCount;
    char parkingBayType[4];
    char *arguments[6];

    getCommandLineArguments(argc, argv, arguments);
    char *busType = arguments[0];
    int incpassengers = atoi(arguments[1]);
    int capacity = atoi(arguments[2]);
    int parkperiod = atoi(arguments[3]);
    int mantime = atoi(arguments[4]);
    int shmid = atoi(arguments[5]);

    char *shmPointer = (char *) attachToSharedMemory(shmid);
    sem_t *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    sem_t *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    sem_t *busesOutgoingMux = (sem_t *) (shmPointer + BUSESOUTGOINGMUTEX_OFFSET);
    sem_t *incomingBusesCountMux = (sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET);
    sem_t *outgoingBusesCountMux = (sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET);

    ///////////////////// Entrance /////////////////////

    sem_wait(incomingBusesCountMux);
    (*((int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET)))++;
    sem_post(incomingBusesCountMux);

    sem_post(busesMux);
    sem_wait(stationManagerIncomingMux);

    getServiceForEntranceByStationManager(shmPointer, busType, incpassengers, mantime, parkingBayType, &parkingIsleIndex);

    maneuver(mantime);

    ///////////////////// DISEMBARK AND BOARD NEW PASSENGERS /////////////////////

    passengersBoardedCount = waitForNewPassengers(parkperiod, capacity);

    ///////////////////// Departure /////////////////////

    sem_wait(outgoingBusesCountMux);
    (*((int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET)))++;
    sem_post(outgoingBusesCountMux);

    sem_post(busesOutgoingMux);
    sem_post(busesMux);
    sem_wait(stationManagerOutgoingMux);

    getServiceForDepartureByStationManager(shmPointer, busType, passengersBoardedCount, parkingBayType, parkingIsleIndex, mantime);

    maneuver(mantime);

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
