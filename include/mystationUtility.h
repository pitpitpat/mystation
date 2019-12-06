#ifndef MYSTATION_UTILITY_H
#define MYSTATION_UTILITY_H

#include <semaphore.h>

void getConfigfile(int, char **, char **);
void readConfigFile(char *, int *);
void createSemaphores();
void removeSemaphores();
void forkBus(int, int, char *);
void forkAndExecBuses(int *, int);
void forkStationManager(int);
void forkAndExecStationManager(int);
void waitForChildren();

#endif
