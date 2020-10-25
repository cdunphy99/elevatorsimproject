//
// Created by alexi on 10/23/2020.
//

#ifndef ELEVATORSIMPROJECT_DATASTRUCTURES_H
#define ELEVATORSIMPROJECT_DATASTRUCTURES_H
#include <stdbool.h>

struct passengerGroup {
    int numPassengers;
    int startFloor;
    int endFloor;
    int timePickedUp;
    int timeDroppedOff;
    int generatedTime;
    // true = up, false = down
    bool direction;
    bool completed;
};

struct passengerGroupArray {
    int size;
    struct passengerGroup *theArray;
};

struct threadArgs {
    int floorNumber;
    int interval;
    int currentTime;
    struct passengerGroupArray *pendingRequests;
};

void addPassengerGroup(struct passengerGroup toAdd, struct passengerGroupArray *passengers) {
    if(passengers->size == 0){
        passengers->theArray = malloc(sizeof(struct passengerGroup));
        passengers->theArray[0] = toAdd;
        passengers->size++;
        return;
    }
    struct passengerGroup *newArray = malloc(sizeof(struct passengerGroup) * (++passengers->size));
    for(int i = 0; i < passengers->size - 1; i++){
        newArray[i] = passengers->theArray[i];
    }
    newArray[passengers->size - 1] = toAdd;
    //free(passengers->theArray);
    passengers->theArray = newArray;
}

void removePassengerGroup(int toRemoveIndex, struct passengerGroupArray *passengers){
    if(passengers->size == 0){
        return;
    }
    struct passengerGroup *newArray = malloc(sizeof(struct passengerGroup) * (--passengers->size));
    for(int i = 0; i < passengers->size; i++){
        if(i == toRemoveIndex){
            continue;
        }
        else{
            newArray[i] = passengers->theArray[i];
        }
    }
    //free(passengers->theArray);
    passengers->theArray = newArray;
}


struct elevator {
    int currentFloor;
    // true = up, false = down
    bool direction;
    int numPassengersOnElevator;
    int *path;
    // 0 = stopped, 1 = stopping, 2 = going up, 3 = going down
    int currentState;
};

#endif //ELEVATORSIMPROJECT_DATASTRUCTURES_H
