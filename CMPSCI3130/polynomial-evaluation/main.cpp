#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

#define ull unsigned long long
#define NUM_EVALUATIONS 10

ull hornersAlgorithm(vector<long> data) {
    auto start = chrono::system_clock::now();
    ull sum = 0;
    int n = data.size() - 2;
    long x = data[0];
    // cout << "n = " << n << "; x = " << x << endl;
    //  i = n + 1 to account for 0th degree
    for (int i = n + 1; i > 0; i--) {
        sum *= x;
        sum += data[i] * x;
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    // cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    // cout << "Result: " << sum << endl;
    return sum;
}

ull bruteForce(vector<long> data) {
    auto start = chrono::system_clock::now();
    ull sum = 0;
    int n = data.size() - 2;
    long x = data[0];
    // cout << "n = " << n << "; x = " << x << endl;
    for (int i = 1; i <= n; i++) {
        sum += data[i + 1] * pow(x, i);
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    // cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    // cout << "Result: " << sum << endl;
    return sum;
}

ull repeatedSquares(vector<long> data) {
    auto start = chrono::system_clock::now();
    ull sum = 0;
    int n = data.size() - 2;
    long x = data[0];
    // cout << "n = " << n << "; x = " << x << endl;
    int k = 0;
    do {
        k++;
    } while (pow(2, k) <= n);

    vector<vector<ull>> decomposedSquares;

    for (int p = 0; p < k; p++) {
        vector<ull> square;
        square.push_back(pow(x, pow(2, p)));
        square.push_back(pow(2, p));
        decomposedSquares.push_back(square);
    }

    for (int i = 1; i <= n; i++) {
        int totalDeg = 0;
        ull xDeg = 1;
        for (int j = k - 1; j >= 0; j--) {
            ull pow2 = decomposedSquares[j][1];
            if (totalDeg + decomposedSquares[j][1] > i)
                continue;
            totalDeg += decomposedSquares[j][1];
            xDeg *= decomposedSquares[j][0];
        }
        sum += xDeg * data[i + 1];
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    // cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    // cout << "Result: " << sum << endl;
    return sum;
}

const void generateData(int n, int d, fstream *dataFile) {
    int upperRandBound = pow(10, d) - pow(10, d - 1);
    int randOffset = pow(10, d - 1);
    for (int i = 0; i < NUM_EVALUATIONS; i++) {
        long x = rand() % upperRandBound + randOffset;
        *dataFile << x << " ";
        for (int j = 0; j <= n; j++) {
            int coefficient = rand() % upperRandBound + randOffset;
            *dataFile << coefficient << " ";
        }
        *dataFile << endl;
    }
}

vector<vector<long>> retrieveData(fstream *dataFile) {
    vector<vector<long>> data;
    string dataLine;
    int lineIdx = 0;

    while (getline(*dataFile, dataLine)) {
        vector<long> curLineData;
        string numStr = "";
        for (int i = 0; i < dataLine.length(); i++) {
            int num;
            numStr.push_back(dataLine[i]);
            if (dataLine[i] == ' ') {
                num = stoi(numStr);
                if (num) {
                    curLineData.push_back(num);
                }
                numStr = "";
                continue;
            }
        }
        data.push_back(curLineData);
    }
    return data;
}

const bool compareResults(fstream *resultsFile) {
    vector<ull> horners;
    vector<ull> brute;
    vector<ull> repSquares;
    string dataLine;

    while (getline(*resultsFile, dataLine)) {
        vector<ull> curLineData;
        string numStr = "";
        for (int i = 0; i < dataLine.length(); i++) {
            int num;
            numStr.push_back(dataLine[i]);
            if (dataLine[i] == ' ') {
                num = stoull(numStr);
                if (num) {
                    curLineData.push_back(num);
                }
                numStr = "";
                continue;
            }
        }
        curLineData.push_back(stoull(numStr));
        for (int i = 0; i < curLineData.size(); i++) {
            cout << curLineData[i] << " ";
        }
        cout << endl;
        horners.push_back(curLineData[0]);
        brute.push_back(curLineData[1]);
        repSquares.push_back(curLineData[2]);
    }
    for (int i = 0; i < horners.size(); i++) {
        if (horners[i] == brute[i] && horners[i] == repSquares[i]) {
            cout << "All three algorithms achieve the same value" << endl;
            return true;
        } else if (horners[i] == brute[i]) {
            cout << "Repeated squares algorithm gives a different value: "
                 << repSquares[i] << " vs " << horners[i] << endl;
        } else if (horners[i] == repSquares[i]) {
            cout << "Brute force algorithm gives a different value: "
                 << brute[i] << " vs " << horners[i] << endl;
        } else if (brute[i] == repSquares[i]) {
            cout << "Horner's algorithm gives a different value: " << horners[i]
                 << " vs " << brute[i] << endl;
        } else {
            cout << "All three algorithms give different answers" << endl;
        }
    }
    return false;
}

long polynomialOverflow(int n, long x) {
    vector<long> data;
    data.push_back(1);
    for (int i = 1; i <= n; i++) {
        data.push_back(i);
    }
    auto start = chrono::system_clock::now();
    long sum = 0;
    cout << "n = " << n << "; x = " << x << endl;
    // i = n + 1 to account for 0th degree
    for (int i = n + 1; i > 1; i--) {
        sum *= x;
        sum += data[i] * x;
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    cout << "Result: " << sum << endl;
    return sum;
}

int main() {
    fstream dataFile;
    dataFile.open("polynomial-data.txt", ios::in | ios::out | ios::app);
    if (!dataFile.is_open()) {
        cerr << "Error opening file polynomial-data.txt" << endl;
        return 1;
    }

    // generateData(5, 2, &dataFile);
    vector<vector<long>> data = retrieveData(&dataFile);
    dataFile.close();
    fstream resultsFile;
    resultsFile.open("results.txt", ios::in | ios::out | ios::app);
    if (!resultsFile.is_open()) {
        cerr << "Error opening file results.txt" << endl;
        return 1;
    }
    ull *horners = new ull[NUM_EVALUATIONS];
    ull *brute = new ull[NUM_EVALUATIONS];
    ull *repSquares = new ull[NUM_EVALUATIONS];
    for (int i = 0; i < data.size(); i++) {
        horners[i] = hornersAlgorithm(data[i]);
        brute[i] = bruteForce(data[i]);
        repSquares[i] = repeatedSquares(data[i]);
    }

    for (int i = 0; i < NUM_EVALUATIONS; i++) {
        resultsFile << horners[i] << " ";
        resultsFile << brute[i] << " ";
        resultsFile << repSquares[i] << endl;
    }
    resultsFile.seekg(0);

    compareResults(&resultsFile);
    // polynomialOverflow(30, 200);
    resultsFile.close();
    return 0;
}
