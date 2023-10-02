// Standard Library Includes
#include <iostream>

// Custom Class Includes
#include "Horse.h"
using namespace std;

void getNumHorses(int &);          // Function to prompt the user for how many horses will be racing
void getHorseInfo(Horse *[], int); // Function to populate the horse array with each horses information

void race(Horse *);

int main() {
    int numHorses = 0;
    getNumHorses(numHorses);
    Horse *horses[numHorses];
    getHorseInfo(horses, numHorses);
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
        cout << "Horse #" << i + 1 << endl;    // Let the user know which horse they are working on
        cout << "Enter the horse's name: ";
        cin >> horseName;
        cout << "Enter the rider's name: ";
        cin >> riderName;

        tempHorse = new Horse(horseName, riderName); // Run the constructor of the temporary Horse pointer with the
                                                     // input of the user
        horses[i] = tempHorse;                       // Set the current index of the Horse array to the
                                                     // temporary Horse pointer
    }
}

void race(Horse *horses) {
}