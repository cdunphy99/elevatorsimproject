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

bool dedupe(struct passengerGroupArray *toDedupe, struct passengerGroup toCheckFor) {
    for(int i = 0; i < toDedupe->size; i++) {
        if(toDedupe->theArray[i].startFloor == toCheckFor.startFloor && toDedupe->theArray[i].endFloor == toCheckFor.endFloor && toDedupe->theArray[i].generatedTime == toCheckFor.generatedTime) {
            printf("Duplicate found! Not added to list.\n");
            return true;
        }
    }
    return false;
}

void *floorThread(void *argStruct) {
    struct threadArgs *threadArgs = (struct threadArgs *) argStruct;
    int floor = threadArgs->floorNumber;
    int interval = threadArgs->interval;
    struct passengerGroupArray *pendingRequests = threadArgs->pendingRequests;
    while(CURRENTTIME < TOTALTIME) {
        pthread_mutex_lock(&timeMutex);
        if (CURRENTTIME % interval == 0) {
            if (rand() % TOTALFLOORS == 1) {
                pthread_mutex_lock(&mutex);
                struct passengerGroup toAdd = generatePassenger(CURRENTTIME, floor);
                if(dedupe(pendingRequests, toAdd)){
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_unlock(&timeMutex);
                    continue;
                }
                else{
                    addPassengerGroup(toAdd, pendingRequests);
                    printf("Time %d: Call received at F%d with destination F%d\n", CURRENTTIME, toAdd.startFloor, toAdd.endFloor);
                }
                pthread_mutex_unlock(&mutex);
            }
        }
        pthread_mutex_unlock(&timeMutex);
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

void *elevatorScheduler (void *argStruct) {
    struct threadArgs *threadArgs = (struct threadArgs *) argStruct;
    struct passengerGroupArray *pendingRequests = threadArgs->pendingRequests;
    struct elevator *elevator = threadArgs->elevator;

    for(int i = 0; i < pendingRequests->size; i++) {
        printf("fuck\n");
        // max 10 people in the car, compare num passengers to 10
        // compare direction of passengers to current direciton of elevator
        // compare start floor with current floor for passengers boarding
        // compare end floor with current passengers endfloors for passengers leaving
        if(elevator->numPassengersOnElevator + pendingRequests->theArray[i].numPassengers <= 10 && !pendingRequests->theArray[i].completed && elevator->direction == pendingRequests->theArray[i].direction){
            // if they can board, now we see what passengerGroups there are available to board at the current floor at array[i]
            if(elevator->currentFloor == pendingRequests->theArray[i].startFloor) {
                // boarding elevator, coming onto elevator
                elevator->numPassengersOnElevator += pendingRequests->theArray[i].numPassengers;

            }

        }

    }
}

void *timeThread(){
    while(CURRENTTIME <= TOTALTIME){
        sleep(1);
        printf("Current time is %d\n", CURRENTTIME);
        pthread_mutex_lock(&timeMutex);
        CURRENTTIME++;
        pthread_mutex_unlock(&timeMutex);
    }
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
    pthread_t elevatorThread;
    struct threadArgs *args = malloc(sizeof(struct threadArgs));
    elevator = malloc(sizeof(struct elevator));
    args->pendingRequests = &pendingRequests;
    args->elevator = elevator;
    pthread_create(&elevatorThread, NULL, elevatorScheduler, (void*) args);
    pthread_t timeThreadtime;
    pthread_create(&timeThreadtime, NULL, timeThread, NULL);

    for(int i = 0; i < TOTALFLOORS; i++){
        pthread_join(threadArray[i], NULL);
    }
    pthread_join(timeThreadtime, NULL);
    pthread_join(elevatorThread, NULL);
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
