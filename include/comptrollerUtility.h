#ifndef COMPTROLLER_UTILITY_H
#define COMPTROLLER_UTILITY_H

void getCommandLineArguments(int, char **, char **);
void printCurrentInfo(char *);
void printStatistics(char *);
void sleepUntilNextPrint(int[2]);
int checkAllBusesServed(char *);

#endif
