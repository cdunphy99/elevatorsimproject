//
// Created by alexi on 10/23/2020.
//
#include "dataStructures.h"
#include <stdlib.h>

struct passengerGroup generatePassenger(int currentTime){
    struct passengerGroup toReturn;
    toReturn.numPassengers = (rand() % 10) + 1;
    toReturn.startFloor = (rand() % 8) + 1;
    toReturn.endFloor = (rand() % 8) + 1;
    toReturn.generatedTime = currentTime;
    while(toReturn.startFloor == toReturn.endFloor){
        toReturn.startFloor = (rand() % 8) + 1;
        toReturn.endFloor = (rand() % 8) + 1;
    }
    return toReturn;
}
