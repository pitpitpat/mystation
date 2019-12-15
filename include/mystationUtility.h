#ifndef MYSTATION_UTILITY_H
#define MYSTATION_UTILITY_H

#include <semaphore.h>

void getConfigfile(int, char **, char **);
void readConfigFile(char *, int [3][6], int *, int *);
void initSemaphores(char *);
void initSharedMemory(char *, int [3][6]);
void destroySemaphores(char *);
void forkAndExecBuses(int [3][6], int);
void forkBus(char *, int, int, int, int, int);
void forkAndExecStationManager(int, int);
void forkStationManager(int, int);
void forkAndExecComptroller(int, int, int);
void forkComptroller(int, int, int);
void waitForChildren();
void removeSharedMemory(int);

#endif
