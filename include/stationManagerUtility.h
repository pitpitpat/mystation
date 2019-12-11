#ifndef STATION_MANAGER_UTILITY_H
#define STATION_MANAGER_UTILITY_H

#include <semaphore.h>

void getSemaphores(char *, sem_t **, sem_t **, sem_t **, sem_t **, sem_t **, sem_t **, sem_t **);
void serveIncomingBus(char *, sem_t *, sem_t *, sem_t *, int *);
void serveOutgoingBus(char *, sem_t *, sem_t *, sem_t *, int *);
void sleepUntilOneLaneIsOpen(int *, int *);
void printBaysCapacity(char *);

#endif
