#include <stdio.h>
#include <string.h>
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

    shmid = callAndCheckInt(shmget(IPC_PRIVATE, BUSTYPESIZE + BAYCAPACITYPERTYPESIZE, 0666), "shmget");
    char *shmPointer = (char *) attachToSharedMemory(shmid);
    memcpy(shmPointer + BAYCAPACITYPERTYPEOFFSET, bayCapacityPerType, BAYCAPACITYPERTYPESIZE);

    createSemaphores();

    forkAndExecStationManager(shmid);
    forkAndExecBuses(busesPerType, shmid);

    waitForChildren();

    removeSemaphores();
    callAndCheckInt(shmctl(shmid, IPC_RMID, 0), "shmctl");
    printf("Removed shared memory\n");
    return 0;
}
