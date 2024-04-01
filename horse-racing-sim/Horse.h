#ifndef HORSE_H
#define HORSE_H

#include <string>
using namespace std;

class Horse {
  private:
    const int displayLength = 50;
    string horseName;
    string riderName;
    int maxRunningDistPerSecond;
    float distanceTraveled;
    int racesWon;
    bool hasReachedGoal = false;

  public:
    Horse(string, string);
    Horse() { horseName = "", riderName = ""; }
    void setHorseName(string n) { horseName = n; }
    void setRiderName(string n) { riderName = n; }
    float getDistanceTraveled() const { return distanceTraveled; }
    int getRacesWon() const { return racesWon; }
    string getHorseName() const { return horseName; }
    string getRiderName() const { return riderName; }
    bool HasReachedGoal() const { return hasReachedGoal; }
    void runASecond();
    void sendToGate();
    void displayHorse(int);
    void displayStats();
};

#endif