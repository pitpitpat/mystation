#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>

#define STATIONMANAGERINCOMINGMUTEX_SIZE sizeof(sem_t)
#define STATIONMANAGEROUTGOINGMUTEX_SIZE sizeof(sem_t)
#define BUSESMUTEX_SIZE sizeof(sem_t)
#define MESSAGESENTMUTEX_SIZE sizeof(sem_t)
#define MESSAGEREADMUTEX_SIZE sizeof(sem_t)
#define INCOMINGBUSESCOUNTMUTEX_SIZE sizeof(sem_t)
#define OUTGOINGBUSESCOUNTMUTEX_SIZE sizeof(sem_t)
#define INCOMINGBUSESCOUNT_SIZE sizeof(int)
#define OUTGOINGBUSESCOUNT_SIZE sizeof(int)
#define BAYCAPACITYPERTYPE_SIZE (3 * sizeof(int))
#define BUSTYPE_SIZE (4 * sizeof(char))
#define BAYTYPE_SIZE (4 * sizeof(char))
#define INCOMINGMANTIME_SIZE sizeof(int)
#define OUTGOINGMANTIME_SIZE sizeof(int)

#define STATIONMANAGERINCOMINGMUTEX_OFFSET (0)
#define STATIONMANAGEROUTGOINGMUTEX_OFFSET (STATIONMANAGERINCOMINGMUTEX_OFFSET + STATIONMANAGERINCOMINGMUTEX_SIZE)
#define BUSESMUTEX_OFFSET (STATIONMANAGEROUTGOINGMUTEX_OFFSET + STATIONMANAGEROUTGOINGMUTEX_SIZE)
#define MESSAGESENTMUTEX_OFFSET (BUSESMUTEX_OFFSET + BUSESMUTEX_SIZE)
#define MESSAGEREADMUTEX_OFFSET (MESSAGESENTMUTEX_OFFSET + MESSAGESENTMUTEX_SIZE)
#define INCOMINGBUSESCOUNTMUTEX_OFFSET (MESSAGEREADMUTEX_OFFSET + MESSAGEREADMUTEX_SIZE)
#define OUTGOINGBUSESCOUNTMUTEX_OFFSET (INCOMINGBUSESCOUNTMUTEX_OFFSET + INCOMINGBUSESCOUNTMUTEX_SIZE)
#define INCOMINGBUSESCOUNT_OFFSET (OUTGOINGBUSESCOUNTMUTEX_OFFSET + OUTGOINGBUSESCOUNTMUTEX_SIZE)
#define OUTGOINGBUSESCOUNT_OFFSET (INCOMINGBUSESCOUNT_OFFSET + INCOMINGBUSESCOUNT_SIZE)
#define BAYCAPACITYPERTYPE_OFFSET (OUTGOINGBUSESCOUNT_OFFSET + OUTGOINGBUSESCOUNT_SIZE)
#define BUSTYPE_OFFSET (BAYCAPACITYPERTYPE_OFFSET + BAYCAPACITYPERTYPE_SIZE)
#define BAYTYPE_OFFSET (BUSTYPE_OFFSET + BUSTYPE_SIZE)
#define INCOMINGMANTIME_OFFSET (BAYTYPE_OFFSET + BAYTYPE_SIZE)
#define OUTGOINGMANTIME_OFFSET (INCOMINGMANTIME_OFFSET + INCOMINGMANTIME_SIZE)

#define SHAREDMEMORY_SIZE (STATIONMANAGERINCOMINGMUTEX_SIZE + STATIONMANAGEROUTGOINGMUTEX_SIZE + BUSESMUTEX_SIZE + MESSAGESENTMUTEX_SIZE + MESSAGEREADMUTEX_SIZE + INCOMINGBUSESCOUNTMUTEX_SIZE + OUTGOINGBUSESCOUNTMUTEX_SIZE + INCOMINGBUSESCOUNT_SIZE + OUTGOINGBUSESCOUNT_SIZE + BAYCAPACITYPERTYPE_SIZE + BUSTYPE_SIZE + BAYTYPE_SIZE + INCOMINGMANTIME_SIZE + OUTGOINGMANTIME_SIZE)

void * attachToSharedMemory(int);

#endif
