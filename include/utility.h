#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>


void* callAndCheckPointer(void *, char *);
int callAndCheckInt(int, char *);
void* callAndCheckSemOpen(void *);
int getIndexFromType(char *);
int getCapacityByBayType(char *, char *);
void formatTime(time_t, char[25]);
double calculateDivision(double, double);

#endif
