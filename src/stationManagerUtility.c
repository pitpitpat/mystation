#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "stationManagerUtility.h"
#include "sharedMemory.h"
#include "utility.h"
#include "isleInfo.h"


void getCommandLineArguments(int argc, char **argv, char **arguments) {
    for (int i = 1; i < (argc - 1); i++) {
        if (!strcmp(argv[i], "-b")) {
            arguments[0] = argv[i + 1];
        } else if (!strcmp(argv[i], "-s")) {
            arguments[1] = argv[i + 1];
        }
    }
}


void serveIncomingBus(char *shmPointer, int bayCapacityPerType[3], int *busesLeft) {
    sem_t *stationManagerIncomingMux = (sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET);
    sem_t *messageSentMux = (sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET);
    sem_t *messageReadMux = (sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET);
    char busType[4], bayType[4];
    int busPid, passengersDisembarkedCount, isleIndex;
    time_t now;

    sem_post(stationManagerIncomingMux);

    // bus is writing info
    sem_wait(messageSentMux);

    memcpy(busType, shmPointer + BUSTYPE_OFFSET, BUSTYPE_SIZE);
    memcpy(&busPid, shmPointer + BUSPID_OFFSET, BUSPID_SIZE);
    memcpy(&passengersDisembarkedCount, shmPointer + PASSENGERSCOUNT_OFFSET, PASSENGERSCOUNT_SIZE);
    // printf("SManager: Read busType %s passengersDisembarkedCount %d\n", busType, passengersDisembarkedCount);

    getAvailableIsleOrServeOutgoingBuses(shmPointer, bayCapacityPerType, busType, busesLeft, bayType, &isleIndex);

    now = time(NULL);
    int busesDepartedCountPerType[3] = {0, 0, 0};
    double timeStayedPerType[3] = {0, 0, 0};
    setIsleInfoByBayByIndex(shmPointer, bayType, isleIndex, 1, passengersDisembarkedCount, now);
    insertEntryToReferenceLedger(now, busPid, busType, bayType, isleIndex, passengersDisembarkedCount, "Entered");
    increaseStatistics(shmPointer, 1, 0, busesDepartedCountPerType, passengersDisembarkedCount, 0, 0, timeStayedPerType);

    // printf("SManager: Write bayType %s isleIndex %d   capacity %d -> %d\n", bayType, isleIndex, bayCapacityPerType[getIndexFromType(bayType)] + 1, bayCapacityPerType[getIndexFromType(bayType)]);
    memcpy(shmPointer + BAYTYPE_OFFSET, bayType, BAYTYPE_SIZE);
    memcpy(shmPointer + ISLEINDEX_OFFSET, &isleIndex, ISLEINDEX_SIZE);
    sem_post(messageReadMux);

    // bus is reading the response
    sem_wait(messageSentMux);
}


void getAvailableIsleOrServeOutgoingBuses(char *shmPointer, int bayCapacityPerType[3], char *busType, int *busesLeft, char *bayType, int *isleIndex) {
    int found = findEmptyBayAndIsle(shmPointer, bayCapacityPerType, busType, bayType, isleIndex);
    if (!found) {
        // printf("SManager: Waiting for an outgoing bus to free a spot\n");
        serveOutgoingBusesToFreeAnIsle(shmPointer, bayCapacityPerType, busType, busesLeft, bayType, isleIndex);
    }
}


void serveOutgoingBusesToFreeAnIsle(char *shmPointer, int bayCapacityPerType[3], char *busType, int *busesLeft, char *bayType, int *isleIndex) {
    sem_t *busesOutgoingMux = (sem_t *) (shmPointer + BUSESOUTGOINGMUTEX_OFFSET);
    sem_t *busesMux = (sem_t *) (shmPointer + BUSESMUTEX_OFFSET);
    int found;

    while(1) {
        sem_wait(busesOutgoingMux);
        sem_wait(busesMux);

        decreaseOutgoingBusesCount(shmPointer);
        serveOutgoingBus(shmPointer, bayCapacityPerType, busesLeft);

        found = findEmptyBayAndIsle(shmPointer, bayCapacityPerType, busType, bayType, isleIndex);
        if (found) break;

        sleep(*((int *) (shmPointer + OUTGOINGMANTIME_OFFSET)));
    }
}


int findEmptyBayAndIsle(char *shmPointer, int bayCapacityPerType[3], char *busType, char *bayType, int *isleIndex) {
    if (!strcmp(busType, "VOR")) {
        if (bayCapacityPerType[getIndexFromType("VOR")] > 0) {
            memcpy(bayType, "VOR", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else if (bayCapacityPerType[getIndexFromType("ASK")] > 0) {
            memcpy(bayType, "ASK", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else if (bayCapacityPerType[getIndexFromType("PEL")] > 0) {
            memcpy(bayType, "PEL", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else {
            return 0;
        }
    } else if (!strcmp(busType, "ASK")) {
        if (bayCapacityPerType[getIndexFromType("ASK")] > 0) {
            memcpy(bayType, "ASK", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else if (bayCapacityPerType[getIndexFromType("PEL")] > 0) {
            memcpy(bayType, "PEL", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else {
            return 0;
        }
    } else if (!strcmp(busType, "PEL")) {
        if (bayCapacityPerType[getIndexFromType("PEL")] > 0) {
            memcpy(bayType, "PEL", BAYTYPE_SIZE);
            *isleIndex = getEmptyIsleIndex(shmPointer, bayType);
            bayCapacityPerType[getIndexFromType(bayType)]--;
            return 1;
        } else {
            return 0;
        }
    } else {
        perror("Invalid busType.");
        exit(EXIT_FAILURE);
    }
}


void serveOutgoingBus(char *shmPointer, int bayCapacityPerType[3], int *busesLeft) {
    sem_t *stationManagerOutgoingMux = (sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET);
    sem_t *messageSent2Mux = (sem_t *) (shmPointer + MESSAGESENT2MUTEX_OFFSET);
    sem_t *messageRead2Mux = (sem_t *) (shmPointer + MESSAGEREAD2MUTEX_OFFSET);
    sem_t *busesLeftCountMux = (sem_t *) (shmPointer + BUSESLEFTCOUNTMUTEX_OFFSET);
    char busType[4], bayType[4];
    int busPid, passengersBoardedCount, isleIndex;
    time_t now;

    sem_post(stationManagerOutgoingMux);

    // bus is writing bus type
    sem_wait(messageSent2Mux);

    memcpy(busType, shmPointer + BUSTYPE_OFFSET, BUSTYPE_SIZE);
    memcpy(&busPid, shmPointer + BUSPID_OFFSET, BUSPID_SIZE);
    memcpy(&passengersBoardedCount, shmPointer + PASSENGERSCOUNT_OFFSET, PASSENGERSCOUNT_SIZE);
    memcpy(bayType, shmPointer + BAYTYPE_OFFSET, BAYTYPE_SIZE);
    memcpy(&isleIndex, shmPointer + ISLEINDEX_OFFSET, ISLEINDEX_SIZE);
    // printf("SManager: Read busType %s passengersBoardedCount %d bayType %s isleIndex %d\n", busType, passengersBoardedCount, bayType, isleIndex);

    now = time(NULL);
    int busesDepartedCountPerType[3] = {0, 0, 0};
    isleInfo *busIsleInfo = getIsleInfoByBayTypeByIndex(shmPointer, bayType, isleIndex);
    double timeStayed = difftime(now, busIsleInfo->arrivalTime);
    double timeStayedPerType[3] = {0, 0, 0};
    busesDepartedCountPerType[getIndexFromType(busType)] = 1;
    timeStayedPerType[getIndexFromType(busType)] = timeStayed;

    setIsleInfoByBayByIndex(shmPointer, bayType, isleIndex, 0, 0, now);
    insertEntryToReferenceLedger(now, busPid, busType, bayType, isleIndex, passengersBoardedCount, "Departed");
    increaseStatistics(shmPointer, 0, 1, busesDepartedCountPerType, 0, passengersBoardedCount, timeStayed, timeStayedPerType);

    bayCapacityPerType[getIndexFromType(busType)]++;
    sem_post(messageRead2Mux);

    // bus is reading the response
    sem_wait(messageSent2Mux);

    (*busesLeft)--;
    sem_wait(busesLeftCountMux);
    (*((int *) (shmPointer + BUSESLEFTCOUNT_OFFSET)))--;
    sem_post(busesLeftCountMux);
}


int decreaseIncomingBusesCount(char *shmPointer) {
    sem_t *incomingBusesCountMux = (sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET);
    int incomingBusesCount;

    sem_wait(incomingBusesCountMux);
    incomingBusesCount = *((int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET));
    if (incomingBusesCount > 0) {
        (*((int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET)))--;
    }
    sem_post(incomingBusesCountMux);

    return incomingBusesCount;
}


int decreaseOutgoingBusesCount(char *shmPointer) {
    sem_t *outgoingBusesCountMux = (sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET);
    int outgoingBusesCount;

    sem_wait(outgoingBusesCountMux);
    outgoingBusesCount = *((int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET));
    if (outgoingBusesCount > 0) {
        (*((int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET)))--;
    }
    sem_post(outgoingBusesCountMux);

    return outgoingBusesCount;
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


void insertEntryToReferenceLedger(time_t timestamp, pid_t busPid, char *busType, char *bayType, int isleIndex, int passengersCount, char *status) {
    FILE *fp = callAndCheckPointer(fopen(REFERENCE_LEDGER_FILEPATH, "a"), "fopen");
    char timestampStr[25];
    formatTime(timestamp, timestampStr);
    fprintf(fp, "Timestamp: %s Id: %d Type: %s Bay: %s Isle: %d Passengers: %d Status: %s\n", timestampStr, busPid, busType, bayType, isleIndex, passengersCount, status);
    fclose(fp);
}


void increaseStatistics(char *shmPointer, int busesEnteredCount, int busesDepartedCount, int busesDepartedCountPerType[3], int passengersDisembarkedCount, int passengersBoardedCount, double timeStayed, double timeStayedPerType[3]) {
    sem_t *statisticsMux = (sem_t *) (shmPointer + STATISTICSMUTEX_OFFSET);

    sem_wait(statisticsMux);
    *((int *) (shmPointer + TOTALBUSESENTEREDCOUNT_OFFSET)) += busesEnteredCount;
    *((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNT_OFFSET)) += busesDepartedCount;
    *((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)) += busesDepartedCountPerType[0];
    *(((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)) + 1) += busesDepartedCountPerType[1];
    *(((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)) + 2) += busesDepartedCountPerType[2];
    *((int *) (shmPointer + TOTALPASSENGERSDISEMBARKEDCOUNT_OFFSET)) += passengersDisembarkedCount;
    *((int *) (shmPointer + TOTALPASSENGERSBOARDEDCOUNT_OFFSET)) += passengersBoardedCount;
    *((double *) (shmPointer + TOTALTIMESTAYED_OFFSET)) += timeStayed;
    *((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)) += timeStayedPerType[0];
    *(((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)) + 1) += timeStayedPerType[1];
    *(((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)) + 2) += timeStayedPerType[2];
    sem_post(statisticsMux);
}


void printStatistics(char *shmPointer) {
    printf("busesEntered: %d | busesDeparted: %d | VOR busesDeparted: %d | ASK busesDeparted: %d | PEL busesDeparted: %d | passengersDisembarked: %d | passengersBoarded: %d | timeStayed: %f | VOR timeStayed: %f | ASK timeStayed: %f | PEL timeStayed: %f\n", *((int *) (shmPointer + TOTALBUSESENTEREDCOUNT_OFFSET)), *((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNT_OFFSET)), *((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)), *(((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)) + 1), *(((int *) (shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET)) + 2), *((int *) (shmPointer + TOTALPASSENGERSDISEMBARKEDCOUNT_OFFSET)), *((int *) (shmPointer + TOTALPASSENGERSBOARDEDCOUNT_OFFSET)), *((double *) (shmPointer + TOTALTIMESTAYED_OFFSET)), *((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)), *(((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)) + 1), *(((double *) (shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET)) + 2));
}


void printBaysCapacity(int bayCapacityPerType[3]) {
    printf("SManager: VOR capacity = %d\n", bayCapacityPerType[0]);
    printf("SManager: ASK capacity = %d\n", bayCapacityPerType[1]);
    printf("SManager: PEL capacity = %d\n", bayCapacityPerType[2]);
}


void printBaysCurrentInfo(char *shmPointer) {
    printf("VOR\n");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "VOR"); i++) {
        printf("\tisle: %d isParked: %d disembarkedPassengersCount: %d\n", i, (getIsleInfoByBayType(shmPointer, "VOR") + i)->isParked, (getIsleInfoByBayType(shmPointer, "VOR") + i)->disembarkedPassengersCount);
    }
    printf("ASK\n");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "ASK"); i++) {
        printf("\tisle: %d isParked: %d disembarkedPassengersCount: %d\n", i, (getIsleInfoByBayType(shmPointer, "ASK") + i)->isParked, (getIsleInfoByBayType(shmPointer, "ASK") + i)->disembarkedPassengersCount);
    }
    printf("PEL\n");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "PEL"); i++) {
        printf("\tisle: %d isParked: %d disembarkedPassengersCount: %d\n", i, (getIsleInfoByBayType(shmPointer, "PEL") + i)->isParked, (getIsleInfoByBayType(shmPointer, "PEL") + i)->disembarkedPassengersCount);
    }
}
