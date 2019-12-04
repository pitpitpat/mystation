#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "utility.h"
#include "sharedSemaphores.h"


int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);

    sem_t *stationManagerMux = callAndCheckSemOpen(sem_open(STATIONMANAGERMUTEX, O_RDWR));
    sem_t *busesMux = callAndCheckSemOpen(sem_open(BUSESMUTEX, O_RDWR));

    printf("Station-Manager: shmid %d\n", shmid);

    int loop = 3;
    while(loop--) {
        printf("Station-Manager: Down(busesMux)\n");
        sem_wait(busesMux);

        printf("Station-Manager: sleep 3 sec\n");
        sleep(3);

        printf("Station-Manager: Up(stationManagerMux)\n");
        sem_post(stationManagerMux);

        printf("\nStation-Manager: Sleeping...\n\n");
        sleep(4);
    }

    callAndCheckInt(sem_close(stationManagerMux), "sem_close");

    return 0;
}
