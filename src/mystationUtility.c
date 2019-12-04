#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
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


void readConfigFile(char *filepath) {
    char *line = NULL;
    size_t len = 0;
    ssize_t line_length;
    char *type, *capacity, *maxParkTime;
    FILE *fp = callAndCheckPointer(fopen(filepath, "r"), "fopen");

    while ((line_length = getline(&line, &len, fp)) != -1) {
        if (line_length <= 0) continue;
        type = strtok(line, " ");
        printf("%s ", type);
        capacity = strtok(NULL, " ");
        printf("%s ", capacity);
        maxParkTime = strtok(NULL, " ");
        printf("%s\n", maxParkTime);
    }

    if (line != NULL) {
        free(line);
    }

    fclose(fp);
}


void createSemaphores() {
    sem_t *stationManagerMux = callAndCheckSemOpen(sem_open(STATIONMANAGERMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_CREAT, S_IRUSR | S_IWUSR, 0));
    callAndCheckInt(sem_close(stationManagerMux), "sem_close");
    callAndCheckInt(sem_close(busesMux), "sem_close");
    printf("Coordinator: Created and opened semaphores\n");
}


void removeSemaphores() {
    sem_unlink(STATIONMANAGERMUTEX);
    sem_unlink(BUSESMUTEX);
    printf("Coordinator: Deleted semaphores\n");
}


void forkBus(int busIndex, int shmid) {
    char busIndexStr[100], shmidStr[100];
    sprintf(busIndexStr, "%d", busIndex);
    sprintf(shmidStr, "%d", shmid);

    char *coachArgv[4] = {"./bus", busIndexStr, shmidStr, NULL};

    callAndCheckInt(execv(coachArgv[0], coachArgv), "execv");
}


void forkAndExecBuses(int count, int shmid) {
    pid_t pid;

    for (int busIndex = 0; busIndex < count; busIndex++) {
        pid = callAndCheckInt(fork(), "fork");
        if (pid == 0) {
            forkBus(busIndex, shmid);
        }
    }
}


void forkStationManager(int shmid) {
    char shmidStr[100];
    sprintf(shmidStr, "%d", shmid);

    char *coachArgv[3] = {"./station-manager", shmidStr, NULL};

    callAndCheckInt(execv(coachArgv[0], coachArgv), "execv");
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
    while(wait(&status) > 0);
}
