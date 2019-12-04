#include <stdio.h>
#include <sys/shm.h>

#include "utility.h"
#include "mystationUtility.h"


int main(int argc, char *argv[]) {
    int err = 0, shmid = 0;
    char *configfile = NULL;

    getConfigfile(argc, argv, &configfile);

    readConfigFile(configfile);

    shmid = shmget(IPC_PRIVATE, 10, 0666);

    if(shmid == -1){
        perror("Creation");
    } else {
        printf("Allocated: %d\n", (int)shmid);
    }

    createSemaphores();

    forkAndExecStationManager(shmid);

    forkAndExecBuses(3, shmid);

    waitForChildren();

    removeSemaphores();

    err = shmctl(shmid, IPC_RMID, 0);
    if (err == -1) perror("Removal.");
    else printf("Removed: %d\n", (int)err);

    return 0;
}
