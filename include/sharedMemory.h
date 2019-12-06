#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define BUSTYPESIZE (4 * sizeof(char))
#define BAYCAPACITYPERTYPESIZE (3 * sizeof(int))

#define BUSTYPEOFFSET (0)
#define BAYCAPACITYPERTYPEOFFSET ((0) + BUSTYPESIZE)

void * attachToSharedMemory(int);

#endif
