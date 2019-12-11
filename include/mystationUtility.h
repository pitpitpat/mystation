#ifndef MYSTATION_UTILITY_H
#define MYSTATION_UTILITY_H

#include <semaphore.h>

void getConfigfile(int, char **, char **);
void readConfigFile(char *, int *);
void initSemaphores(char *);
void initSharedMemory(char *, int *);
void destroySemaphores(char *);
void forkAndExecBuses(int *, int);
void forkBus(char *, int, int, int, int, int);
void forkAndExecStationManager(int);
void forkStationManager(int);
void waitForChildren();
void removeSharedMemory(int);

#endif
