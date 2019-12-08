#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#include "mystationUtility.h"
#include "utility.h"
#include "sharedSemaphores.h"


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


void createSemaphores() {
    sem_t *stationManagerIncomingMux = callAndCheckSemOpen(sem_open(STATIONMANAGERINCOMINGMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(stationManagerIncomingMux), "sem_close");

    sem_t *stationManagerOutgoingMux = callAndCheckSemOpen(sem_open(STATIONMANAGEROUTGOINGMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(stationManagerOutgoingMux), "sem_close");

    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(busesMux), "sem_close");

    sem_t *messageSent = callAndCheckSemOpen(sem_open(MESSAGESENTMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(messageSent), "sem_close");

    sem_t *messageRead = callAndCheckSemOpen(sem_open(MESSAGEREADMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(messageRead), "sem_close");

    printf("Mystation: Created and opened semaphores\n");
}


void removeSemaphores() {
    sem_unlink(STATIONMANAGERINCOMINGMUTEX);
    sem_unlink(STATIONMANAGEROUTGOINGMUTEX);
    sem_unlink(BUSESMUTEX);
    sem_unlink(MESSAGESENTMUTEX);
    sem_unlink(MESSAGEREADMUTEX);
    printf("Mystation: Deleted semaphores\n");
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


void forkAndExecBuses(int busesPerType[3], int shmid) {
    pid_t pid;

    for (int busIndex = 0; busIndex < busesPerType[0]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("VOR", 30, 45, 8, 6, shmid);
        }
    }
    for (int busIndex = 0; busIndex < busesPerType[1]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("ASK", 20, 30, 6, 6, shmid);
        }
    }
    for (int busIndex = 0; busIndex < busesPerType[2]; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus("PEL", 10, 10, 4, 6, shmid);
        }
    }
}


void forkStationManager(int shmid) {
    char shmidStr[100];
    sprintf(shmidStr, "%d", shmid);

    char *stationManagerArgv[3] = {"./station-manager", shmidStr, NULL};

    callAndCheckInt(execv(stationManagerArgv[0], stationManagerArgv), "execv");
}


void forkAndExecStationManager(int shmid) {
    pid_t pid;

    pid = callAndCheckInt(fork(), "fork");
    if (pid == 0) {
        forkStationManager(shmid);
    }
}


void waitForChildren() {
    int status;
    while(wait(&status) > 0) {
        printf("status: %d\n", status);
    };
}
