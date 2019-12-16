#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "mystationUtility.h"
#include "utility.h"
#include "sharedMemory.h"
#include "isleInfo.h"


void getConfigfile(int argc, char **argv, char **configfile) {
    for (int i = 1; i < (argc - 1); i++) {
        if (strcmp(argv[i], "-l") == 0) {
            *configfile = argv[i + 1];
        }
    }
}


void readConfigFile(char *filepath, int configurationInfo[3][6], int *time, int *stattimes) {
    char *line = NULL;
    size_t len = 0;
    ssize_t line_length;
    char *type;
    FILE *fp = callAndCheckPointer(fopen(filepath, "r"), "fopen");

    while ((line_length = getline(&line, &len, fp)) != -1) {
        if (line_length <= 0) continue;
        type = strtok(line, " ");
        if (!strcmp(type, "comptroller")) {
            *time = atoi(strtok(NULL, " "));
            *stattimes = atoi(strtok(NULL, " "));
        } else {
            configurationInfo[getIndexFromType(type)][0] = atoi(strtok(NULL, " "));
            configurationInfo[getIndexFromType(type)][1] = atoi(strtok(NULL, " "));
            configurationInfo[getIndexFromType(type)][2] = atoi(strtok(NULL, " "));
            configurationInfo[getIndexFromType(type)][3] = atoi(strtok(NULL, " "));
            configurationInfo[getIndexFromType(type)][4] = atoi(strtok(NULL, " "));
            configurationInfo[getIndexFromType(type)][5] = atoi(strtok(NULL, " "));
        }
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
    callAndCheckInt(sem_init((sem_t *) (shmPointer + BUSESOUTGOINGMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + BUSESLEFTCOUNTMUTEX_OFFSET), 1, 1), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + BAYSCURRENTINFOMUTEX_OFFSET), 1, 1), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + STATISTICSMUTEX_OFFSET), 1, 1), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGESENT2MUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + MESSAGEREAD2MUTEX_OFFSET), 1, 0), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET), 1, 1), "sem_init");
    callAndCheckInt(sem_init((sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET), 1, 1), "sem_init");
}


void initSharedMemory(char *shmPointer, int configurationInfo[3][6]) {
    memcpy(shmPointer + BAYCAPACITYPERTYPE_OFFSET, &(configurationInfo[0][0]), sizeof(int));
    memcpy(((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET)) + 1, &(configurationInfo[1][0]), sizeof(int));
    memcpy(((int *) (shmPointer + BAYCAPACITYPERTYPE_OFFSET)) + 2, &(configurationInfo[2][0]), sizeof(int));
    *((int *) (shmPointer + INCOMINGBUSESCOUNT_OFFSET)) = 0;
    *((int *) (shmPointer + OUTGOINGBUSESCOUNT_OFFSET)) = 0;
    *((int *) (shmPointer + INCOMINGMANTIME_OFFSET)) = 0;
    *((int *) (shmPointer + OUTGOINGMANTIME_OFFSET)) = 0;
    *((int *) (shmPointer + BUSESLEFTCOUNT_OFFSET)) = configurationInfo[0][1] + configurationInfo[1][1] + configurationInfo[2][1];
    initManyIsleInfo((isleInfo *) (shmPointer + BAYSCURRENTINFO_OFFSET), configurationInfo[0][0] + configurationInfo[1][0] + configurationInfo[2][0]);
}


void destroySemaphores(char *shmPointer) {
    sem_destroy((sem_t *) (shmPointer + STATIONMANAGERINCOMINGMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + STATIONMANAGEROUTGOINGMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + BUSESMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + BUSESOUTGOINGMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + BUSESLEFTCOUNTMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + BAYSCURRENTINFOMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + STATISTICSMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGESENTMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGEREADMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGESENT2MUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + MESSAGEREAD2MUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + INCOMINGBUSESCOUNTMUTEX_OFFSET));
    sem_destroy((sem_t *) (shmPointer + OUTGOINGBUSESCOUNTMUTEX_OFFSET));
}


void forkAndExecBuses(int configurationInfo[3][6], int shmid) {
    pid_t pid;

    for (int i = 0; i < configurationInfo[0][1]; i++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("VOR", configurationInfo[0][2], configurationInfo[0][3], configurationInfo[0][4], configurationInfo[0][5], shmid);
        }
    }
    for (int i = 0; i < configurationInfo[1][1]; i++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("ASK", configurationInfo[1][2], configurationInfo[1][3], configurationInfo[1][4], configurationInfo[1][5], shmid);
        }
    }
    for (int i = 0; i < configurationInfo[2][1]; i++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("PEL", configurationInfo[2][2], configurationInfo[2][3], configurationInfo[2][4], configurationInfo[2][5], shmid);
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

    char *busArgv[14] = {"./bus", "-t", busType, "-n", incpassengersStr, "-c", capacityStr, "-p", parkperiodStr, "-m", mantimeStr, "-s", shmidStr, NULL};

    callAndCheckInt(execv(busArgv[0], busArgv), "execv");
}


void forkAndExecStationManager(int busesCount, int shmid) {
    pid_t pid;

    pid = callAndCheckInt(fork(), "fork");
    if (pid == 0) {
        forkStationManager(busesCount, shmid);
    }
}


void forkStationManager(int busesCount, int shmid) {
    char busesCountStr[100], shmidStr[100];
    sprintf(busesCountStr, "%d", busesCount);
    sprintf(shmidStr, "%d", shmid);

    char *stationManagerArgv[6] = {"./station-manager", "-b", busesCountStr, "-s", shmidStr, NULL};

    callAndCheckInt(execv(stationManagerArgv[0], stationManagerArgv), "execv");
}


void forkAndExecComptroller(int time, int stattimes, int shmid) {
    pid_t pid;

    pid = callAndCheckInt(fork(), "fork");
    if (pid == 0) {
        forkComptroller(time, stattimes, shmid);
    }
}


void forkComptroller(int time, int stattimes, int shmid) {
    char timeStr[100], stattimesStr[100], shmidStr[100];
    sprintf(timeStr, "%d", time);
    sprintf(stattimesStr, "%d", stattimes);
    sprintf(shmidStr, "%d", shmid);

    char *comptrollerArgv[8] = {"./comptroller", "-d", timeStr, "-t", stattimesStr, "-s", shmidStr, NULL};

    callAndCheckInt(execv(comptrollerArgv[0], comptrollerArgv), "execv");
}


void waitForChildren() {
    int status;
    while(wait(&status) > 0);
}


void removeSharedMemory(int shmid) {
    callAndCheckInt(shmctl(shmid, IPC_RMID, 0), "shmctl");
}
