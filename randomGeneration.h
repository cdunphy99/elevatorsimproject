//
// Created by alexi on 10/23/2020.
//

#ifndef ELEVATORSIMPROJECT_RANDOMGENERATION_H
#define ELEVATORSIMPROJECT_RANDOMGENERATION_H
#include "dataStructures.h"
#include <stdlib.h>
struct passengerGroup generatePassenger(int currentTime, int startFloor){
    struct passengerGroup toReturn;
    toReturn.numPassengers = (rand() % 10) + 1;
//    toReturn.startFloor = startFloor;
    toReturn.startFloor = 3;
//    toReturn.endFloor = (rand() % 8) + 1;
    toReturn.endFloor = rand() % 2 + 1;
    toReturn.generatedTime = currentTime;
    toReturn.completed = false;
    while(toReturn.startFloor == toReturn.endFloor){
        toReturn.startFloor = (rand() % 8) + 1;
        toReturn.endFloor = (rand() % 8) + 1;
    }
    return toReturn;
}

#endif //ELEVATORSIMPROJECT_RANDOMGENERATION_H
