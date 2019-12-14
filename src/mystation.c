#include <stdio.h>
#include <sys/shm.h>

#include "utility.h"
#include "mystationUtility.h"
#include "sharedMemory.h"
#include "isleInfo.h"


int main(int argc, char *argv[]) {
    int shmid = 0;
    char *configfile = NULL;
    int busesPerType[3] = {1, 2, 3};
    int bayCapacityPerType[3];

    getCurrentTime();

    getConfigfile(argc, argv, &configfile);
    readConfigFile(configfile, bayCapacityPerType);

    int baysCurrentInfoSize = ((bayCapacityPerType[0] + bayCapacityPerType[1] + bayCapacityPerType[2]) * sizeof(isleInfo));
    shmid = callAndCheckInt(shmget(IPC_PRIVATE, SHAREDMEMORY_SIZE + baysCurrentInfoSize, 0666), "shmget");
    char *shmPointer = (char *) attachToSharedMemory(shmid);

    initSemaphores(shmPointer);
    initSharedMemory(shmPointer, bayCapacityPerType);

    forkAndExecStationManager(shmid);
    forkAndExecBuses(busesPerType, shmid);

    waitForChildren();

    destroySemaphores(shmPointer);
    removeSharedMemory(shmid);
    return 0;
}
