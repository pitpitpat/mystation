#ifndef STATION_MANAGER_UTILITY_H
#define STATION_MANAGER_UTILITY_H

#include <semaphore.h>

void getCommandLineArguments(int, char **, char **);
void serveIncomingBus(char *, int *, int *);
void getAvailableIsleOrServeOutgoingBuses(char *, int *, char *, int *, char *, int *);
void serveOutgoingBusesToFreeAnIsle(char *, int *, char *, int *, char *, int *);
int findEmptyBayAndIsle(char *, int *, char *, char *, int *);
void serveOutgoingBus(char *, int *, int *);
int decreaseIncomingBusesCount(char *);
int decreaseOutgoingBusesCount(char *);
int findEmptyBayAndIsle(char *, int *, char *, char *, int *);
void sleepUntilOneLaneIsOpen(int *, int *);
void printBaysCapacity(int *);
void printBaysCurrentInfo(char *);

#endif
