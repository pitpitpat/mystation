#include <stdio.h>
#include <string.h>
#include <time.h>

#include "isleInfo.h"
#include "utility.h"
#include "sharedMemory.h"


void initIsleInfo(isleInfo *II) {
    II->isParked = 0;
    II->disembarkedPassengersCount = 0;
}


void setIsleInfo(isleInfo *II, int isParked, int disembarkedPassengersCount, time_t arrivalTime) {
    II->isParked = isParked;
    II->disembarkedPassengersCount = disembarkedPassengersCount;
    II->arrivalTime = arrivalTime;
}


void markAsEmptyIsleInfo(isleInfo *II) {
    II->isParked = 0;
    II->disembarkedPassengersCount = 0;
}


void initManyIsleInfo(isleInfo *II, int count) {
    for (int i = 0; i < count; i++) {
        initIsleInfo(II + i);
    }
}


isleInfo* getIsleInfoByBayType(char *shmPointer, char *bayType) {
    isleInfo *bayIsleInfo;
    if (!strcmp(bayType, "VOR")) {
        bayIsleInfo = (isleInfo *) (shmPointer + BAYSCURRENTINFO_OFFSET);
    } else if (!strcmp(bayType, "ASK")) {
        bayIsleInfo = ((isleInfo *) (shmPointer + BAYSCURRENTINFO_OFFSET)) + getCapacityByBayType(shmPointer, "VOR");
    } else if (!strcmp(bayType, "PEL")) {
        bayIsleInfo = ((isleInfo *) (shmPointer + BAYSCURRENTINFO_OFFSET)) + getCapacityByBayType(shmPointer, "VOR") + getCapacityByBayType(shmPointer, "ASK");
    }
    return bayIsleInfo;
}


int getEmptyIsleIndex(char *shmPointer, char *bayType) {
    isleInfo *bayIsleInfo = getIsleInfoByBayType(shmPointer, bayType);
    for (int i = 0; i < getCapacityByBayType(shmPointer, bayType); i++) {
        if ((bayIsleInfo + i)->isParked == 0) {
            return i;
        }
    }
    return -1;
}


void setIsleInfoByBayByIndex(char *shmPointer, char *bayType, int isleIndex, int isParked, int disembarkedPassengersCount, time_t arrivalTime) {
    isleInfo *bayIsleInfo = getIsleInfoByBayType(shmPointer, bayType);
    setIsleInfo(bayIsleInfo + isleIndex, isParked, disembarkedPassengersCount, arrivalTime);
}
