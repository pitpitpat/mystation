#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#include "mystationUtility.h"
#include "utility.h"
#include "sharedMemory.h"


void getConfigfile(int argc, char **argv, char **configfile) {
    for (int i = 1; i < (argc - 1); i++) {
        if (strcmp(argv[i], "-l") == 0) {
            *configfile = argv[i + 1];
        }
    }
}


void readConfigFile(char *filepath, int bayCapacityPerType[3]) {
    char *line = NULL;
    size_t len = 0;
    ssize_t line_length;
    char *type;
    int capacity, maxParkTime;
    FILE *fp = callAndCheckPointer(fopen(filepath, "r"), "fopen");

    while ((line_length = getline(&line, &len, fp)) != -1) {
        if (line_length <= 0) continue;
        type = strtok(line, " ");
        capacity = atoi(strtok(NULL, " "));
        maxParkTime = atoi(strtok(NULL, " "));
        bayCapacityPerType[getIndexFromBusType(type)] = capacity;
    }

    if (line != NULL) {
        free(line);
    }

    fclose(fp);
}


void initSemaphores(char *shmPointer) {
    callAndCheckInt(sem_init((sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + BUSESMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET), 1, 1), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET), 1, 1), "sem_init");
    printf("Mystation: Initialized semaphores\n\n");
}


void initSharedMemory(char *shmPointer, int bayCapacityPerType[3]) {
    memcpy(shmPointer + BAYCAPACITYPERTYPE_OFFSET, bayCapacityPerType, BAYCAPACITYPERTYPE_SIZE);
    *((int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET)) = 0;
    *((int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET)) = 0;
    *((int *) (shmPointer + INCOMINGMANTIME_OFFSET)) = 0;
    *((int *) (shmPointer + OUTGOINGMANTIME_OFFSET)) = 0;
}


void destroySemaphores(char *shmPointer) {
    sem_destroy((sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + BUSESMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET));
    printf("\nMystation: Destroyed semaphores\n");
}


void forkAndExecBuses(int busesPerType[3], int shmid) {
    pid_t pid;

    for (int busIndex = 0; busIndex < busesPerType[0]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("VOR", 30, 45, 8, 2, shmid);
        }
    }
    for (int busIndex = 0; busIndex < busesPerType[1]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("ASK", 20, 30, 6, 10, shmid);
        }
    }
    for (int busIndex = 0; busIndex < busesPerType[2]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("PEL", 10, 10, 4, 4, shmid);
        }
    }
}


void forkBus(char *busType, int incpassengers, int capacity, int parkperiod, int mantime, int shmid) {
    char incpassengersStr[20], capacityStr[20], parkperiodStr[20], mantimeStr[20], shmidStr[20];
    sprintf(incpassengersStr, "%d", incpassengers);
    sprintf(capacityStr, "%d", capacity);
    sprintf(parkperiodStr, "%d", parkperiod);
    sprintf(mantimeStr, "%d", mantime);
    sprintf(shmidStr, "%d", shmid);

    char *busArgv[8] = {"./bus", busType, incpassengersStr, capacityStr, parkperiodStr, mantimeStr, shmidStr, NULL};

    callAndCheckInt(execv(busArgv[0], busArgv), "execv");
}


void forkAndExecStationManager(int shmid) {
    pid_t pid;

    pid = callAndCheckInt(fork(), "fork");
    if (pid == 0) {
        forkStationManager(shmid);
    }
}


void forkStationManager(int shmid) {
    char shmidStr[100];
    sprintf(shmidStr, "%d", shmid);

    char *stationManagerArgv[3] = {"./station-manager", shmidStr, NULL};

    callAndCheckInt(execv(stationManagerArgv[0], stationManagerArgv), "execv");
}


void waitForChildren() {
    int status;
    while(wait(&status) > 0);
}


void removeSharedMemory(int shmid) {
    callAndCheckInt(shmctl(shmid, IPC_RMID, 0), "shmctl");
    printf("Removed shared memory\n");
}
