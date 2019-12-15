#include <stdio.h>
#include <sys/shm.h>

#include "utility.h"
#include "mystationUtility.h"
#include "sharedMemory.h"
#include "isleInfo.h"


int main(int argc, char *argv[]) {
    char *configfile;
    int totalBusesCount, totalIslesCount, baysCurrentInfoSegmentSize, shmid, configurationInfo[3][6];

    getConfigfile(argc, argv, &configfile);
    readConfigFile(configfile, configurationInfo);

    totalBusesCount = configurationInfo[0][1] + configurationInfo[1][1] + configurationInfo[2][1];
    totalIslesCount = configurationInfo[0][0] + configurationInfo[1][0] + configurationInfo[2][0];
    baysCurrentInfoSegmentSize = totalIslesCount * sizeof(isleInfo);

    shmid = callAndCheckInt(shmget(IPC_PRIVATE, SHAREDMEMORY_SIZE + baysCurrentInfoSegmentSize, 0666), "shmget");
    char *shmPointer = (char *) attachToSharedMemory(shmid);

    initSemaphores(shmPointer);
    initSharedMemory(shmPointer, configurationInfo, totalIslesCount);

    forkAndExecStationManager(totalBusesCount, shmid);
    forkAndExecBuses(configurationInfo, shmid);

    waitForChildren();

    destroySemaphores(shmPointer);
    removeSharedMemory(shmid);
    return 0;
}
