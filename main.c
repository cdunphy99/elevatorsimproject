#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "dataStructures.h"
#include "randomGeneration.h"

void init() {
    srand(time(0));
    struct passengerGroupArray passengers;
    passengers.size = 0;
    struct passengerGroup test1 = generatePassenger(0);
    struct passengerGroup test2 = generatePassenger(5);
    addPassengerGroup(test1, &passengers);
    addPassengerGroup(test2, &passengers);
    for(int i = 0; i < passengers.size; i++){
        printf("%d", passengers.theArray[i].numPassengers);
    }
}

int main() {
    struct elevator *elevator;
    init();
    return 0;
}
