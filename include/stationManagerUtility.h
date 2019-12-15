#ifndef STATION_MANAGER_UTILITY_H
#define STATION_MANAGER_UTILITY_H

#include <semaphore.h>

#define REFERENCE_LEDGER_FILEPATH "./reference-ledger.txt"

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
void insertEntryToReferenceLedger(time_t, pid_t, char *, char *, int, int, char *);
void printBaysCapacity(int *);
void printBaysCurrentInfo(char *);

#endif
