#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "dataStructures.h"
#include "randomGeneration.h"

int TOTALFLOORS;
int TIMEINTERVAL;
int TOTALTIME;
int CURRENTTIME;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timeMutex = PTHREAD_MUTEX_INITIALIZER;

void *floorThread(void *argStruct){
    struct threadArgs *threadArgs = (struct threadArgs *) argStruct;
    int floor = threadArgs->floorNumber;
    int interval = threadArgs->interval;
    struct passengerGroupArray *pendingRequests = threadArgs->pendingRequests;
    while(CURRENTTIME < TOTALTIME) {
        if (CURRENTTIME % interval == 0) {
            if (rand() % TOTALFLOORS == 1) {
                printf("Generating request on floor %d at time %d\n", floor, CURRENTTIME);
                pthread_mutex_lock(&timeMutex);
                pthread_mutex_lock(&mutex);
                addPassengerGroup(generatePassenger(CURRENTTIME), pendingRequests);
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&timeMutex);
            }
        }
        sleep(1);
    }
    return NULL;
}

void init(struct passengerGroupArray *toInit) {
    srand(time(0));
    toInit->size = 0;
    CURRENTTIME = 0;
//    struct passengerGroup test1 = generatePassenger(0);
//    struct passengerGroup test2 = generatePassenger(5);
//    addPassengerGroup(test1, &passengers);
//    addPassengerGroup(test2, &passengers);
//    for(int i = 0; i < passengers.size; i++){
//        printf("%d", passengers.theArray[i].numPassengers);
//    }
}

void run() {
    struct elevator *elevator;
    struct passengerGroupArray pendingRequests;
    init(&pendingRequests);
    pthread_t *threadArray = malloc(sizeof(pthread_t) * TOTALFLOORS);
    for(int i = 0; i < TOTALFLOORS; i++){
        struct threadArgs *args = malloc(sizeof(struct threadArgs));
        args->floorNumber = i + 1;
        args->interval = TIMEINTERVAL;
        args->pendingRequests = &pendingRequests;
        pthread_create(&threadArray[i], NULL, floorThread, (void*)args);
    }
    while(CURRENTTIME <= TOTALTIME){
        sleep(1);
        printf("Current time is %d\n", CURRENTTIME);
        pthread_mutex_lock(&timeMutex);
        CURRENTTIME++;
        pthread_mutex_unlock(&timeMutex);

    }
    for(int i = 0; i < TOTALFLOORS; i++){
        pthread_join(threadArray[i], NULL);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: a.out [var_1] [var_2]\n");
        exit(0);
    }
    TOTALFLOORS = atoi(argv[1]);
    TIMEINTERVAL = atoi(argv[2]);
    TOTALTIME = atoi(argv[3]);
    run();
    return 0;
}
