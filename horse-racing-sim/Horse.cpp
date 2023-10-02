// Standard Library Includes
#include <iostream>
#include <random>

// Custom Class Includes
#include "Horse.h"
using namespace std;

Horse::Horse(string name, string rider) {
    srand(time(0));                             // Make a new random seed every time the function is called
    horseName = name;                           // Set the name of the horse
    riderName = rider;                          // Set the name of the rider
    distanceTraveled = 0;                       // Set the distance ran of the horse to 0 when it is created
    maxRunningDistPerSecond = rand() % 100 + 1; // Randomize the horse's max running speed
}

/* This function has the horse run a random amount of distance between 0 and its maximum running speed */
void Horse::runASecond() {
    int distanceMovedThisSecond = rand() % maxRunningDistPerSecond; // Have the horse run a random distance up to its maxRunningDist
    distanceTraveled += distanceMovedThisSecond;
}
/* This function sends the horses back to the gate at the beginning of each race.
    Use in the event that the user wants to simulate another race with the same set of horses. */
void Horse::sendToGate() {
    distanceTraveled = 0;
}

void Horse::displayHorse(int goalLength) {
    int numDashes = distanceTraveled % goalLength;
    for (int i = 0; i < numDashes; i++) {
        cout << "-";
    }
    cout << ">";
}