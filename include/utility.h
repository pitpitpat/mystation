#ifndef UTILITY_H
#define UTILITY_H

void* callAndCheckPointer(void *, char *);
int callAndCheckInt(int, char *);
void* callAndCheckSemOpen(void *);
int getIndexFromType(char *);
int getCapacityByBayType(char *, char *);
void getCurrentTime();

#endif
