#ifndef BUS_UTILITY_H
#define BUS_UTILITY_H

void getCommandLineArguments(int, char **, char **);
int getRandomInteger(int);
int waitForNewPassengers(int, int);
void getServiceForEntranceByStationManager(char *, char *, int, int, char *, int *);
void getServiceForDepartureByStationManager(char *, char *, int, char *, int, int);
void maneuver(int);

#endif
