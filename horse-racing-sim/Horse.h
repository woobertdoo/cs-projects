#ifndef HORSE_H
#define HORSE_H

#include <string>
using namespace std;

class Horse {
  private:
    string horseName;
    string riderName;
    int maxRunningDistPerSecond;
    int distanceTraveled;
    int racesWon;

  public:
    Horse(string, string);
    void setHorseName(string n) { horseName = n; }
    void setRiderName(string n) { riderName = n; }
    int getDistanceTraveled() const { return distanceTraveled; }
    int getRacesWon() const { return racesWon; }
    string getHorseName() const { return horseName; }
    string getRiderName() const { return riderName; }
    void runASecond();
    void sendToGate();
    void displayHorse(int);
};

#endif