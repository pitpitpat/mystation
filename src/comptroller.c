#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "utility.h"
#include "comptrollerUtility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    char *arguments[3];
    int timesUntilNextPrint[2] = {0, 0};

    getCommandLineArguments(argc, argv, arguments);
    int time = atoi(arguments[0]);
    int stattimes = atoi(arguments[1]);
    int shmid = atoi(arguments[2]);

    char *shmPointer = (char *) attachToSharedMemory(shmid);

    while(1) {
        if (timesUntilNextPrint[1] == 0) {
            printStatistics(shmPointer);
            timesUntilNextPrint[1] = stattimes;
        }

        if (timesUntilNextPrint[0] == 0) {
            printCurrentInfo(shmPointer);
            timesUntilNextPrint[0] = time;
        }

        if (checkAllBusesServed(shmPointer)) break;

        sleepUntilNextPrint(timesUntilNextPrint);
    }

    callAndCheckInt(shmdt(shmPointer), "shmdt");
    return 0;
}
