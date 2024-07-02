// Standard Library Includes
#include <iostream>

// Custom Class Includes
#include "Horse.h"
using namespace std;

void getNumHorses(int &);          // Function to prompt the user for how many horses will be racing
void getHorseInfo(Horse *[], int); // Function to populate the horse array with each horses information

void race(Horse *[], int, bool &);
void finishRace(Horse *, bool &);
void resetHorses(Horse *[], int);

int main() {
    int numHorses = 0;
    getNumHorses(numHorses);
    Horse **horses = new Horse *[numHorses];
    getHorseInfo(horses, numHorses);
    bool raceOver = false;
    bool shouldExit = false;

    while (!shouldExit) {
        race(horses, numHorses, raceOver);
        char raceAgain;
        if (raceOver) {
            for (int i = 0; i < numHorses; i++) {
                horses[i]->displayStats();
                horses[i]->sendToGate();
            }
            cout << "Would you like to race again with the same horses?(y/n) ";
            cin >> raceAgain;
            raceOver = false;
            if (raceAgain == 'n' || raceAgain == 'N')
                shouldExit = true;
        }
    }

    delete[] horses;
    return 0;
}

void getNumHorses(int &numHorses) {
    cout << "How many horses will be racing? ";
    cin >> numHorses;

    // Make sure the number of horses is a positive number greater than 1
    while (numHorses < 2) {
        cout << "You need to have at least two horses to race.\n";
        cout << "How many horses will be racing? ";
        cin >> numHorses;
    }
}
/* Populates the horse array with information for each horse */
void getHorseInfo(Horse *horses[], int numHorses) {
    for (int i = 0; i < numHorses; i++) {
        string horseName = "", riderName = ""; // Create dummy variables to temporarly store the names
                                               // of the horse and rider
        Horse *tempHorse = nullptr;            // Create a dummy variable to store a single Horse pointer to populate
                                               // an index of the array
        cout << "Horse #" << i + 1 << "\n";    // Let the user know which horse they are working on
        if (i == 0)
            cin.ignore();
        cout << "Enter the horse's name: ";
        getline(cin, horseName);
        while (horseName == "") {
            cout << "You must enter a name to continue: ";
            cin >> horseName;
        }
        cout << "Enter the rider's name: ";

        getline(cin, riderName);
        while (riderName == "") {
            cout << "You must enter a name to continue: ";
            cin >> riderName;
        }

        tempHorse = new Horse(horseName, riderName); // Run the constructor of the temporary Horse pointer with the
                                                     // input of the user
        horses[i] = tempHorse;                       // Set the current index of the Horse array to the
                                                     // temporary Horse pointer
    }
}

void race(Horse *horses[], int numHorses, bool &raceOver) {
    Horse *winner;
    int goalLength;
    cout << "How long should the race be (meters)? ";
    cin >> goalLength;
    while (goalLength < 100) {
        cout << "The race should be at least 100 meters for a fun time!\n";
        cout << "How long should the race be (meters)? ";
        cin >> goalLength;
    }
    while (!raceOver) {
        char raceAgain;
        for (int i = 0; i < numHorses; i++) {
            horses[i]->runASecond();
            horses[i]->displayHorse(goalLength);

            if (horses[i]->HasReachedGoal()) {
                raceOver = true;
                winner = horses[i];
            }
        }

        if (!raceOver) {
            cout << "Would you like to continue the race?(y/n) ";
            cin >> raceAgain;
            if (raceAgain == 'n' || raceAgain == 'N')
                raceOver = true;
        }
    }

    finishRace(winner, raceOver);
}

void finishRace(Horse *winner, bool &raceOver) {
    cout << "The winner is " << winner->getHorseName() << ", ridden by " << winner->getRiderName() << "!\n";
}

void resetHorses(Horse *horses[], int numHorses) {
    for (int i = 0; i < numHorses; i++) {
        horses[i]->sendToGate();
    }
}