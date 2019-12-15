#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utility.h"
#include "comptrollerUtility.h"
#include "sharedMemory.h"
#include "isleInfo.h"


void getCommandLineArguments(int argc, char **argv, char **arguments) {
    for (int i = 1; i < (argc - 1); i++) {
        if (!strcmp(argv[i], "-d")) {
            arguments[0] = argv[i + 1];
        } else if (!strcmp(argv[i], "-t")) {
            arguments[1] = argv[i + 1];
        } else if (!strcmp(argv[i], "-s")) {
            arguments[2] = argv[i + 1];
        }
    }
}


void printCurrentInfo(char *shmPointer) {
    sem_t *baysCurrentInfoMux = (sem_t *) (shmPointer + BAYSCURRENTINFOMUTEX_OFFSET);
    int totalBusesParked = 0, emptyIslesPerBayType[3] = {0, 0, 0}, totalPassengersDisemberkedCount = 0;
    isleInfo *bayIsleInfo;

    sem_wait(baysCurrentInfoMux);
    bayIsleInfo = getIsleInfoByBayType(shmPointer, "VOR");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "VOR"); i++) {
        if ((bayIsleInfo + i)->isParked == 1) {
            totalBusesParked++;
            totalPassengersDisemberkedCount += (bayIsleInfo + i)->disembarkedPassengersCount;
        } else {
            emptyIslesPerBayType[getIndexFromType("VOR")]++;
        }
    }
    bayIsleInfo = getIsleInfoByBayType(shmPointer, "ASK");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "ASK"); i++) {
        if ((bayIsleInfo + i)->isParked == 1) {
            totalBusesParked++;
            totalPassengersDisemberkedCount += (bayIsleInfo + i)->disembarkedPassengersCount;
        } else {
            emptyIslesPerBayType[getIndexFromType("ASK")]++;
        }
    }
    bayIsleInfo = getIsleInfoByBayType(shmPointer, "PEL");
    for (int i = 0; i < getCapacityByBayType(shmPointer, "PEL"); i++) {
        if ((bayIsleInfo + i)->isParked == 1) {
            totalBusesParked++;
            totalPassengersDisemberkedCount += (bayIsleInfo + i)->disembarkedPassengersCount;
        } else {
            emptyIslesPerBayType[getIndexFromType("PEL")]++;
        }
    }
    sem_post(baysCurrentInfoMux);

    printf("\n---- COMPTROLLER CURRENT STATE INFORMATION ----\n");
    printf("Number of buses parked: %d\n", totalBusesParked);
    printf("Number of empty isles per type VOR: %d ASK: %d PEL: %d\n", emptyIslesPerBayType[0], emptyIslesPerBayType[1], emptyIslesPerBayType[2]);
    printf("Number of passengers disembarked: %d\n", totalPassengersDisemberkedCount);
}


void printStatistics(char *shmPointer) {
    sem_t *statisticsMux = (sem_t *) (shmPointer + STATISTICSMUTEX_OFFSET);
    int totalbusesEntered, totalbusesDeparted, totalbusesDepartedPerType[3], totalPassengersDisemberked, totalPassengersBoarded;
    double totalTimeBusesStayed, totalTimeBusesStayedPerType[3], averageTimeBusesStayed, averageTimeBusesStayedPerType[3];

    sem_wait(statisticsMux);
    memcpy(&totalbusesEntered, shmPointer + TOTALBUSESENTEREDCOUNT_OFFSET, TOTALBUSESENTEREDCOUNT_SIZE);
    memcpy(&totalbusesDeparted, shmPointer + TOTALBUSESDEPARTEDCOUNT_OFFSET, TOTALBUSESDEPARTEDCOUNT_SIZE);
    memcpy(totalbusesDepartedPerType, shmPointer + TOTALBUSESDEPARTEDCOUNTPERTYPE_OFFSET, TOTALBUSESDEPARTEDCOUNTPERTYPE_SIZE);
    memcpy(&totalPassengersDisemberked, shmPointer + TOTALPASSENGERSDISEMBARKEDCOUNT_OFFSET, TOTALPASSENGERSDISEMBARKEDCOUNT_SIZE);
    memcpy(&totalPassengersBoarded, shmPointer + TOTALPASSENGERSBOARDEDCOUNT_OFFSET, TOTALPASSENGERSBOARDEDCOUNT_SIZE);
    memcpy(&totalTimeBusesStayed, shmPointer + TOTALTIMESTAYED_OFFSET, TOTALTIMESTAYED_SIZE);
    memcpy(totalTimeBusesStayedPerType, shmPointer + TOTALTIMESTAYEDPERTYPE_OFFSET, TOTALTIMESTAYEDPERTYPE_SIZE);
    sem_post(statisticsMux);

    averageTimeBusesStayed = calculateDivision(totalTimeBusesStayed, totalbusesDeparted);
    averageTimeBusesStayedPerType[0] = calculateDivision(totalTimeBusesStayedPerType[0], totalbusesDepartedPerType[0]);
    averageTimeBusesStayedPerType[1] = calculateDivision(totalTimeBusesStayedPerType[1], totalbusesDepartedPerType[1]);
    averageTimeBusesStayedPerType[2] = calculateDivision(totalTimeBusesStayedPerType[2], totalbusesDepartedPerType[2]);

    printf("\n---- COMPTROLLER STATISTICS ----\n");
    printf("Total number of buses entered: %d\n", totalbusesEntered);
    printf("Total number of buses departed: %d\n", totalbusesDeparted);
    printf("Total number of buses departed per type VOR: %d ASK: %d PEL: %d\n", totalbusesDepartedPerType[0], totalbusesDepartedPerType[1], totalbusesDepartedPerType[2]);
    printf("Total number of passengers disembarked: %d\n", totalPassengersDisemberked);
    printf("Total number of passengers boarded: %d\n", totalPassengersBoarded);
    printf("Average time buses stayed in station: %f\n", averageTimeBusesStayed);
    printf("Average time buses stayed in station per type VOR: %f ASK: %f PEL: %f\n", averageTimeBusesStayedPerType[0], averageTimeBusesStayedPerType[1], averageTimeBusesStayedPerType[2]);
}


void sleepUntilNextPrint(int timesUntilNextPrint[2]) {
    if (timesUntilNextPrint[0] <= timesUntilNextPrint[1]) {
        sleep(timesUntilNextPrint[0]);
        timesUntilNextPrint[1] -= timesUntilNextPrint[0];
        timesUntilNextPrint[0] = 0;
    } else {
        sleep(timesUntilNextPrint[1]);
        timesUntilNextPrint[0] -= timesUntilNextPrint[1];
        timesUntilNextPrint[1] = 0;
    }
}


int checkAllBusesServed(char *shmPointer) {
    sem_t *baysCurrentInfoMux = (sem_t *) (shmPointer + BAYSCURRENTINFOMUTEX_OFFSET);
    int allBusesServed = 0;

    sem_wait(baysCurrentInfoMux);
    if (*((int *) (shmPointer + BUSESLEFTCOUNT_OFFSET)) == 0) {
        allBusesServed = 1;
    }
    sem_post(baysCurrentInfoMux);

    return allBusesServed;
}
