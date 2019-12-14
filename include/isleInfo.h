#ifndef ISLE_INFO_H
#define ISLE_INFO_H

#include <time.h>

typedef struct isleInfo {
    int isParked;
    int disembarkedPassengersCount;
    time_t arrivalTime;
} isleInfo;

void initIsleInfo(isleInfo *);
void setIsleInfo(isleInfo *, int, int, time_t);
void markAsEmptyIsleInfo(isleInfo *);
void initManyIsleInfo(isleInfo *, int);
isleInfo* getIsleInfoByBayType(char *, char *);
int getEmptyIsleIndex(char *, char *);
void setIsleInfoByBayByIndex(char *, char *, int, int, int, time_t);

#endif
