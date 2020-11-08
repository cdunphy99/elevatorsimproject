#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "dataStructures.h"
#include "randomGeneration.h"

//global variables + thread stuff
int TOTALFLOORS;
int TIMEINTERVAL;
int TOTALTIME;
int CURRENTTIME;
FILE *OUTFILE;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timeWait = PTHREAD_COND_INITIALIZER;
bool waiting = false;

//this function gets rid of duplicate call requests, run after generating a request
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

//this is the floorThread function, its responsible for generating requests from
//each floor(thread)
void *floorThread(void *argStruct) {
    struct threadArgs *threadArgs = (struct threadArgs *) argStruct;
    int floor = threadArgs->floorNumber;
    int interval = threadArgs->interval;
    struct passengerGroupArray *pendingRequests = threadArgs->pendingRequests;
    while (CURRENTTIME < TOTALTIME) {
        pthread_mutex_lock(&timeMutex);
        if (CURRENTTIME % interval == 0) {
            if (rand() % TOTALFLOORS == 0) {
                pthread_mutex_lock(&mutex);
                struct passengerGroup toAdd = generatePassenger(CURRENTTIME, floor);
                if (dedupe(pendingRequests, toAdd)) {
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_unlock(&timeMutex);
                    continue;
                } else {
                    addPassengerGroup(toAdd, pendingRequests);
                    printf("Time %d: Call received at F%d with destination F%d, %d passengers\n", CURRENTTIME,
                           toAdd.startFloor,
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
}

//the waitFor function is similar to the sleep command but works
//more synchronously with our time
void waitFor(int howLong) {
    pthread_mutex_lock(&timeMutex);
    waiting = true;
    for (int i = 0; i < howLong; i++) {
        CURRENTTIME++;
        sleep(1);
        printf("Waited for %d/%d\n", i + 1, howLong);
    }
    printf("Done waiting, current time is %d\n", CURRENTTIME);
    fprintf(OUTFILE, "Done waiting, current time is %d\n", CURRENTTIME);
    waiting = false;
    pthread_cond_broadcast(&timeWait);
    pthread_mutex_unlock(&timeMutex);
}

//makes elevator goUp by one floor
void goUp(struct elevator *elevator) {
    elevator->direction = true;
    printf("Going up at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Going up at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    waitFor(5);
    elevator->currentFloor++;
    printf("Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
}

//makes elevator goDown by one floor
void goDown(struct elevator *elevator) {
    elevator->direction = false;
    printf("Going down at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Going down at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    waitFor(5);
    elevator->currentFloor--;
    printf("Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
    fprintf(OUTFILE, "Arrived at time %d, current floor: %d\n", CURRENTTIME, elevator->currentFloor);
}

//this function looks through all the requests to see if there are any pending
//or in progress jobs, this function works with the shouldStop function
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

//this function checks if there are any pending requests above the current floor
bool getPendingAbove(struct passengerGroupArray *pendingRequests, int floor) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!pendingRequests->theArray[i].completed &&
            !pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].startFloor > floor) {
            toReturn = true;
        }
    }
    return toReturn;
}

//this function checks if there are any pending requests below the current floor
bool getPendingBelow(struct passengerGroupArray *pendingRequests, int floor) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (!pendingRequests->theArray[i].completed &&
            !pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].startFloor < floor) {
            toReturn = true;
        }
    }
    return toReturn;
}

//this function prints the current passengers on the elevator
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

//this function works with getPending to check if the elevator should stop or not based on
//if there are any pending or in progress requests
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

//this function checks if there are any requests that are in progress going a certain direction
bool anyInProgressGoingDirection(struct passengerGroupArray *pendingRequests, bool direction) {
    bool toReturn = false;
    for (int i = 0; i < pendingRequests->size; i++) {
        if (pendingRequests->theArray[i].inProgress && pendingRequests->theArray[i].direction == direction) {
            toReturn = true;
        }
    }
    return toReturn;
}

//this function checks the direction of the elevator, and returns the direction it should go in
bool whichDirection(struct passengerGroupArray *pendingRequests, struct elevator *elevator) {
    if (elevator->direction == false) {
        if (elevator->currentFloor - 1 <= 0) {
            elevator->direction = true;
            return true;
        }
        if (getPendingBelow(pendingRequests, elevator->currentFloor) ||
            anyInProgressGoingDirection(pendingRequests, false)) {
            printf("continuing in current direction\n");
            return elevator->direction;
        } else if (getPendingAbove(pendingRequests, elevator->currentFloor) ||
                   !anyInProgressGoingDirection(pendingRequests, false)) {
            printf("getpendingabove: %d\n", getPendingAbove(pendingRequests, elevator->currentFloor));
            printf("Elevator changed direction in whichdirection, now going up\n");
            elevator->direction = true;
            return true;
        } else {
            return false;
        }
    } else if (elevator->direction == true) {
        if (elevator->currentFloor + 1 > TOTALFLOORS) {
            elevator->direction = false;
            return false;
        }
        if (getPendingAbove(pendingRequests, elevator->currentFloor) ||
            anyInProgressGoingDirection(pendingRequests, true)) {
            printf("continuing in current direction\n");
            return elevator->direction;
        } else if (getPendingBelow(pendingRequests, elevator->currentFloor) ||
                   !anyInProgressGoingDirection(pendingRequests, true)) {
            printf("getpendingbelow: %d\n", getPendingBelow(pendingRequests, elevator->currentFloor));
            printf("Elevator changed direction in whichdirection, now going down\n");
            elevator->direction = false;
            return false;
        } else {
            return true;
        }
    }
}

//this function checks if we can pickup a passenger group on a floor
bool shouldPickPassengerGroupUp(struct passengerGroup *toCheck, struct elevator *elevator) {
    bool toReturn = false;
    if (toCheck->startFloor == elevator->currentFloor) {
        if (!toCheck->inProgress && !toCheck->completed) {
            if (elevator->numPassengersOnElevator > 0) {
                if (toCheck->direction == elevator->direction) {
                    if (toCheck->numPassengers + elevator->numPassengersOnElevator <= 10) {
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

bool shouldDropPassengerGroupOff(struct passengerGroup *toCheck, struct elevator *elevator) {
    bool toReturn = false;
    if (elevator->currentFloor == toCheck->endFloor &&
        toCheck->inProgress &&
        elevator->numPassengersOnElevator - toCheck->numPassengers >= 0 &&
        !toCheck->completed) {
        toReturn = true;
    }
    return toReturn;
}

//this function calculates and prints the stats after the simulation
void printStats(struct passengerGroupArray *toPrint) {
    // # passengers serviced
    // avg wait time for all serviced passengers
    // avg turnaround time
    // doing these stats for each individual passenger
    int numServiced = 0;
    double avgWaitTime = 0;
    double avgTurnaroundTime = 0;
    for (int i = 0; i < toPrint->size; i++) {
        if (toPrint->theArray[i].completed) {
            numServiced += toPrint->theArray[i].numPassengers;
            avgWaitTime += (toPrint->theArray[i].timePickedUp - toPrint->theArray[i].generatedTime) *
                           toPrint->theArray[i].numPassengers;
            avgTurnaroundTime += (toPrint->theArray[i].timeDroppedOff - toPrint->theArray[i].generatedTime) *
                                 toPrint->theArray[i].numPassengers;
        }
    }
    avgWaitTime /= numServiced;
    avgTurnaroundTime /= numServiced;
    //doing both printf and fprintf to print to a file and console
    printf("\n\nNumber of passengers serviced: %d\n", numServiced);
    printf("Average wait time of all passengers: %g\n", avgWaitTime);
    printf("Average turnaround time of all passengers: %g\n", avgTurnaroundTime);
    fprintf(OUTFILE, "\n\nNumber of passengers serviced: %d\n", numServiced);
    fprintf(OUTFILE, "Average wait time of all passengers: %g\n", avgWaitTime);
    fprintf(OUTFILE, "Average turnaround time of all passengers: %g\n", avgTurnaroundTime);
}

//this is the "main" function, using all the other helper functions
//this schedules what the elevator does
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
            //if we can pick up the passengers
            if (shouldPickPassengerGroupUp(&pendingRequests->theArray[i], elevator)) {
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
            //if we should drop off the passengers
            if (shouldDropPassengerGroupOff(&pendingRequests->theArray[i], elevator)) {
                elevator->numPassengersOnElevator -= pendingRequests->theArray[i].numPassengers;
                pendingRequests->theArray[i].timeDroppedOff = CURRENTTIME;
                pendingRequests->theArray[i].completed = true;
                pendingRequests->theArray[i].inProgress = false;
                printf("Time %d: %d passengers left on floor %d\n", CURRENTTIME,
                       pendingRequests->theArray[i].numPassengers,
                       elevator->currentFloor);
                fprintf(OUTFILE, "Time %d: %d passengers left on floor %d\n", CURRENTTIME,
                        pendingRequests->theArray[i].numPassengers,
                        elevator->currentFloor);
                printf("Current number of passengers: %d\n", elevator->numPassengersOnElevator);
                fprintf(OUTFILE, "Current number of passengers: %d\n", elevator->numPassengersOnElevator);
                passengerOperation = true;
            }
        }
        printf("Current number of passengers: %d\n", elevator->numPassengersOnElevator);
        fprintf(OUTFILE, "Current number of passengers: %d\n", elevator->numPassengersOnElevator);
        //we want to wait 2 seconds for passengers getting on/off
        if (passengerOperation) {
            waitFor(2);
            printf("passenger operation done, time increased by 2\n");
            fprintf(OUTFILE, "passenger operation done, time increased by 2\n");
        }
        //does the elevator have any pending requests?
        if (!shouldStop(pendingRequests)) {
            //check for direction
            elevator->direction = whichDirection(pendingRequests, elevator);
            //going UP
            if (elevator->direction == true) {
                goUp(elevator);
                //going DOWN
            } else if (elevator->direction == false) {
                goDown(elevator);
            }
            //there are no pending or in progress requests and the elevator should stop
        } else {
            printf("Elevator standing still\n");
            fprintf(OUTFILE, "Elevator standing still\n");
            waitFor(1);
        }
        printCurrentPassengers(pendingRequests);
        printf("\n\n");
        fprintf(OUTFILE, "\n\n");
    }
    return NULL;
}

//the timeThread, this synchronises the time across all threads
void *timeThread() {
    while (CURRENTTIME <= TOTALTIME) {
        //if waiting is true, that means the function waitFor is being used and
        //temporarily in control of the time, and the main time thread should
        //wait till waitFor is completed
        while (waiting) {
            //the thread waits till it gets the signal from timeWait
            //once it does, it grabs the timeMutex
            pthread_cond_wait(&timeWait, &timeMutex);
            waiting = false;
        }
        printf("Current time is %d\n", CURRENTTIME);
        fprintf(OUTFILE, "Current time is %d\n", CURRENTTIME);
        sleep(1);
        CURRENTTIME++;
        pthread_mutex_unlock(&timeMutex);
    }
    return NULL;
}

//running :) (making threads and stuff)
void run() {
    struct elevator *elevator;
    struct passengerGroupArray pendingRequests;
    init(&pendingRequests);
    pthread_t *threadArray = malloc(sizeof(pthread_t) * TOTALFLOORS);
    //since we can only pass 1 thing to a thread, we make a struct of "arguments"
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
    printStats(&pendingRequests);
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
