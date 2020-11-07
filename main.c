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
FILE *OUTFILE;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timeWait = PTHREAD_COND_INITIALIZER;
bool waiting = false;

bool dedupe(struct passengerGroupArray *toDedupe, struct passengerGroup toCheckFor) {
    for (int i = 0; i < toDedupe->size; i++) {
        if (toDedupe->theArray[i].startFloor == toCheckFor.startFloor &&
            toDedupe->theArray[i].endFloor == toCheckFor.endFloor &&
            toDedupe->theArray[i].generatedTime == toCheckFor.generatedTime) {
            printf("Duplicate found! Not added to list.\n");
            fprintf(OUTFILE, "Duplicate found! Not added to list.\n");
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
    while (CURRENTTIME < TOTALTIME) {
        pthread_mutex_lock(&timeMutex);
        if (CURRENTTIME % 5 == 0) {
            if (rand() % TOTALFLOORS == 0) {
                pthread_mutex_lock(&mutex);
                struct passengerGroup toAdd = generatePassenger(CURRENTTIME, floor);
                if (dedupe(pendingRequests, toAdd)) {
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_unlock(&timeMutex);
                    continue;
                } else {
                    addPassengerGroup(toAdd, pendingRequests);
                    printf("Time %d: Call received at F%d with destination F%d, %d passengers\n", CURRENTTIME, toAdd.startFloor,
                           toAdd.endFloor, toAdd.numPassengers);
                    fprintf(OUTFILE, "Time %d: Call received at F%d with destination F%d, %d passengers\n", CURRENTTIME,
                            toAdd.startFloor,
                            toAdd.endFloor, toAdd.numPassengers);
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
    OUTFILE = fopen("elevatorOutput.txt", "w");
//    struct passengerGroup test1 = generatePassenger(0);
//    struct passengerGroup test2 = generatePassenger(5);
//    addPassengerGroup(test1, &passengers);
//    addPassengerGroup(test2, &passengers);
//    for(int i = 0; i < passengers.size; i++){
//        printf("%d", passengers.theArray[i].numPassengers);
//    }
}

void waitFor(int howLong) {
    pthread_mutex_lock(&timeMutex);
    waiting = true;
    for (int i = 0; i < howLong; i++) {
        CURRENTTIME++;
        sleep(1);
        printf("Waited for %d/%d\n", i + 1, howLong);
        //fprintf(OUTFILE, "Waited for %d/%d\n", i + 1, howLong);
    }
    printf("Done waiting, current time is %d\n", CURRENTTIME);
    fprintf(OUTFILE, "Done waiting, current time is %d\n", CURRENTTIME);
    waiting = false;
    pthread_cond_broadcast(&timeWait);
    pthread_mutex_unlock(&timeMutex);
}

void goUp(struct elevator *elevator) {
    if (elevator->direction == false) {
        printf("ALERT: Elevator changing directions to up\n");
        fprintf(OUTFILE, "ALERT: Elevator changing directions to up\n");

    }
    elevator->direction = true;
    elevator->currentState = 2;
    printf("Going up at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Going up at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    waitFor(5);
    elevator->currentFloor++;
    printf("Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
}

void goDown(struct elevator *elevator) {
    if (elevator->direction == true) {
        printf("ALERT: Elevator changing directions to down\n");
        fprintf(OUTFILE, "ALERT: Elevator changing directions to down\n");
    }
    elevator->direction = false;
    elevator->currentState = 3;
    printf("Going down at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Going down at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    waitFor(5);
    elevator->currentFloor--;
    printf("Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
}

int getInProgress(struct passengerGroupArray *pendingRequests, bool direction) {
    int toReturn = 0;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].direction == direction &&
            !pendingRequests->theArray[i].completed) {
            toReturn++;
        }
    }
    return toReturn;
}

bool getPending(struct passengerGroupArray *pendingRequests) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!pendingRequests->theArray[i].completed ||
            pendingRequests->theArray[i].inProgress) {
            toReturn = true;
        }
    }
    return toReturn;
}

bool getPendingAbove(struct passengerGroupArray *pendingRequests, bool direction, int floor) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!pendingRequests->theArray[i].completed &&
            !pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].startFloor > floor) {
            toReturn = true;
        }
    }
    return toReturn;
}

bool getPendingBelow(struct passengerGroupArray *pendingRequests, bool direction, int floor) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!pendingRequests->theArray[i].completed &&
            !pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].startFloor < floor) {
            toReturn = true;
        }
    }
    return toReturn;
}

bool getInProgressDirection(struct passengerGroupArray *pendingRequests) {
    for (int i = 0; i < pendingRequests->size; i++) {
        if (pendingRequests->theArray[i].inProgress && !pendingRequests->theArray[i].completed) {
            return pendingRequests->theArray[i].direction;
        }
    }
}

void printCurrentPassengers(struct passengerGroupArray *pendingRequests) {
    for (int i = 0; i < pendingRequests->size; i++) {
        if (pendingRequests->theArray[i].inProgress) {
            printf("CURRENT PASSENGERS: %d passengers picked up at F%d at time %d, destination F%d\n",
                   pendingRequests->theArray[i].numPassengers, pendingRequests->theArray[i].startFloor,
                   pendingRequests->theArray[i].timePickedUp, pendingRequests->theArray[i].endFloor);
            fprintf(OUTFILE, "CURRENT PASSENGERS: %d passengers picked up at F%d at time %d, destination F%d\n",
                    pendingRequests->theArray[i].numPassengers, pendingRequests->theArray[i].startFloor,
                    pendingRequests->theArray[i].timePickedUp, pendingRequests->theArray[i].endFloor);
        }
    }
}

bool shouldStop(struct passengerGroupArray *pendingRequests) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!getPending(pendingRequests)) {
            printf("no pending requests, should stop!\n");
            toReturn = true;
        }
    }
    return toReturn;
}

bool anyInProgressGoingDirection(struct passengerGroupArray *pendingRequests, bool direction) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].direction == direction) {
            toReturn = true;
        }
    }
    return toReturn;
}

bool whichDirection(struct passengerGroupArray *pendingRequests, struct elevator *elevator) {
    if (elevator->direction == false) {
        if(elevator->currentFloor - 1 <= 0){
            elevator->direction = true;
            return true;
        }
        if (getPendingBelow(pendingRequests, true, elevator->currentFloor) || anyInProgressGoingDirection(pendingRequests, false)) {
            printf("continuing in current direction\n");
            return elevator->direction;
        } else if (getPendingAbove(pendingRequests, false, elevator->currentFloor) || !anyInProgressGoingDirection(pendingRequests, false)) {
            printf("getpendingabove: %d\n", getPendingAbove(pendingRequests, true, elevator->currentFloor));
            printf("Elevator changed direction in whichdirection, now going up\n");
            elevator->direction = true;
            return true;
        } else {
            return false;
        }
    } else if (elevator->direction == true) {
        if(elevator->currentFloor + 1 > TOTALFLOORS) {
            elevator->direction = false;
            return false;
        }
        if (getPendingAbove(pendingRequests, false, elevator->currentFloor) || anyInProgressGoingDirection(pendingRequests, true)) {
            printf("continuing in current direction\n");
            return elevator->direction;
        } else if (getPendingBelow(pendingRequests, true, elevator->currentFloor) || !anyInProgressGoingDirection(pendingRequests, true)) {
            printf("getpendingbelow: %d\n", getPendingBelow(pendingRequests, true, elevator->currentFloor));
            printf("Elevator changed direction in whichdirection, now going down\n");
            elevator->direction = false;
            return false;
        } else {
            return true;
        }
    }
}

bool shouldPickPassengerGroupUp(struct passengerGroup *toCheck, struct elevator *elevator) {
    bool toReturn = false;
    if (toCheck->startFloor == elevator->currentFloor) {
        if (!toCheck->inProgress && !toCheck->completed) {
            if (elevator->numPassengersOnElevator > 0) {
                if (toCheck->direction == elevator->direction) {
                    if(toCheck->numPassengers + elevator->numPassengersOnElevator <= 10) {
                        toReturn = true;
                    }
                }
            } else if (elevator->numPassengersOnElevator == 0) {
                toReturn = true;
            }
        }
    }
    return toReturn;
}

void *elevatorScheduler(void *argStruct) {
    struct threadArgs *threadArgs = (struct threadArgs *) argStruct;
    struct passengerGroupArray *pendingRequests = threadArgs->pendingRequests;
    struct elevator *elevator = threadArgs->elevator;
    elevator->currentFloor = 1;
    elevator->direction = true;
    elevator->numPassengersOnElevator = 0;
    bool passengerOperation;
    while (CURRENTTIME < TOTALTIME) {
        passengerOperation = false;
        for (int i = 0; i < pendingRequests->size; i++) {
            // max 10 people in the car, compare num passengers to 10
            // compare direction of passengers to current direciton of elevator
            // compare start floor with current floor for passengers boarding
            // compare end floor with current passengers endfloors for passengers leaving

            if (shouldPickPassengerGroupUp(&pendingRequests->theArray[i], elevator)) {
                // if they can board, now we see what passengerGroups there are available to board at the current floor at array[i]
                // boarding elevator, coming onto elevator
//                    if (elevator->direction != pendingRequests->theArray[i].direction) {
//                        elevator->direction = pendingRequests->theArray[i].direction;
//                    }

                elevator->numPassengersOnElevator += pendingRequests->theArray[i].numPassengers;
                pendingRequests->theArray[i].timePickedUp = CURRENTTIME;
                pendingRequests->theArray[i].inProgress = true;
                printf("Time %d: %d passengers boarding on floor %d, destination F%d\n",
                       CURRENTTIME, pendingRequests->theArray[i].numPassengers, elevator->currentFloor,
                       pendingRequests->theArray[i].endFloor);
                fprintf(OUTFILE, "Time %d: %d passengers boarding on floor %d, destination F%d\n",
                        CURRENTTIME, pendingRequests->theArray[i].numPassengers, elevator->currentFloor,
                        pendingRequests->theArray[i].endFloor);

                passengerOperation = true;
            }
            if (elevator->currentFloor == pendingRequests->theArray[i].endFloor &&
                pendingRequests->theArray[i].inProgress &&
                elevator->numPassengersOnElevator - pendingRequests->theArray[i].numPassengers >= 0 &&
                !pendingRequests->theArray[i].completed) {
                elevator->numPassengersOnElevator -= pendingRequests->theArray[i].numPassengers;
                pendingRequests->theArray[i].timeDroppedOff = CURRENTTIME;
                pendingRequests->theArray[i].completed = true;
                pendingRequests->theArray[i].inProgress = false;
                printf("Time %d: %d passengers left on floor %d\n", CURRENTTIME, pendingRequests->theArray[i].numPassengers,
                       elevator->currentFloor);
                fprintf(OUTFILE, "Time %d: %d passengers left on floor %d\n", CURRENTTIME, pendingRequests->theArray[i].numPassengers,
                        elevator->currentFloor);
                printf("Current number of passengers: %d\n", elevator->numPassengersOnElevator);
                fprintf(OUTFILE, "Current number of passengers: %d\n", elevator->numPassengersOnElevator);
                passengerOperation = true;
            }
        }
        printf("Current number of passengers: %d\n", elevator->numPassengersOnElevator);
        fprintf(OUTFILE, "Current number of passengers: %d\n", elevator->numPassengersOnElevator);

        if (passengerOperation) {
            waitFor(2);
            printf("passenger operation done, time increased by 2\n");
            fprintf(OUTFILE, "passenger operation done, time increased by 2\n");
        }

        if (!shouldStop(pendingRequests)) {
            elevator->direction = whichDirection(pendingRequests, elevator);
            if (elevator->direction == true) {
                goUp(elevator);
            } else if (elevator->direction == false) {
                goDown(elevator);
            }
        } else {
            printf("Elevator standing still\n");
            fprintf(OUTFILE, "Elevator standing still\n");
            waitFor(1);
        }

        printCurrentPassengers(pendingRequests);
        printf("\n\n");
        fprintf(OUTFILE, "\n\n");
    }
}


void *timeThread() {
    while (CURRENTTIME <= TOTALTIME) {
        while (waiting) {
            pthread_cond_wait(&timeWait, &timeMutex);
            waiting = false;
        }
        //pthread_mutex_lock(&timeMutex);
        printf("Current time is %d\n", CURRENTTIME);
        fprintf(OUTFILE, "Current time is %d\n", CURRENTTIME);
        sleep(1);
        CURRENTTIME++;
        pthread_mutex_unlock(&timeMutex);
    }
    return NULL;
}

void run() {
    struct elevator *elevator;
    struct passengerGroupArray pendingRequests;
    init(&pendingRequests);
    pthread_t *threadArray = malloc(sizeof(pthread_t) * TOTALFLOORS);
    for (int i = 0; i < TOTALFLOORS; i++) {
        struct threadArgs *args = malloc(sizeof(struct threadArgs));
        args->floorNumber = i + 1;
        args->interval = TIMEINTERVAL;
        args->pendingRequests = &pendingRequests;
        pthread_create(&threadArray[i], NULL, floorThread, (void *) args);
    }
    pthread_t elevatorThread;
    struct threadArgs *args = malloc(sizeof(struct threadArgs));
    elevator = malloc(sizeof(struct elevator));
    args->pendingRequests = &pendingRequests;
    args->elevator = elevator;
    pthread_create(&elevatorThread, NULL, elevatorScheduler, (void *) args);
    pthread_t timeThreadtime;
    pthread_create(&timeThreadtime, NULL, timeThread, NULL);
    for (int i = 0; i < TOTALFLOORS; i++) {
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
    fclose(OUTFILE);
    return 0;
}
