// Standard Library Includes
#include "math.h"
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
    // Find the number of dashes to print by finding the percentage of the distance traveled
    // with respect to the total goal length and drawing 1 dash for every 2%
    int numDashes = min(floor((distanceTraveled / goalLength) * 50), 50.f);
    for (int i = 0; i < numDashes; i++) {
        cout << "-";
    }
    if (distanceTraveled < goalLength) {
        cout << ">";
        int numSpaces = 50 - numDashes - 1;
        if (numSpaces > 0)
            for (int i = 0; i < numSpaces; i++) {
                cout << " ";
            }
    }
    cout << "|";
    // If in the last second the horse moves past the goal line, display the goal line before the arrow
    if (distanceTraveled > goalLength) {
        hasReachedGoal = true;
        racesWon++;
        cout << ">";
    }

    cout << horseName << ", ridden by " << riderName << endl;
}