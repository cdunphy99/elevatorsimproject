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
