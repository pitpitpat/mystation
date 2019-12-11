#include <stdio.h>
#include <sys/shm.h>

#include "utility.h"
#include "mystationUtility.h"
#include "sharedMemory.h"


int main(int argc, char *argv[]) {
    int shmid = 0;
    char *configfile = NULL;
    int busesPerType[3] = {1, 2, 3};
    int bayCapacityPerType[3];

    getConfigfile(argc, argv, &configfile);
    readConfigFile(configfile, bayCapacityPerType);

    shmid = callAndCheckInt(shmget(IPC_PRIVATE, SHAREDMEMORY_SIZE, 0666), "shmget");
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
