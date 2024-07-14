#include "BigInt.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

#define ull unsigned long long
#define NUM_EVALUATIONS 10

BigInt hornersAlgorithm(vector<BigInt> data) {
    cout << "** HORNERS **\n";
    auto start = chrono::system_clock::now();
    BigInt sum = 0;
    int n = data.size() - 2;
    BigInt x = data[0];
    cout << "n = " << n << "; x = " << x << endl;
    //  i = n + 1 to account for 0th degree
    for (int i = n + 1; i > 1; i--) {
        sum += data[i];
        sum *= x;
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    cout << "Result: " << sum << endl;
    return sum;
}

BigInt bruteForce(vector<BigInt> data) {
    cout << "** BRUTE FORCE **\n";
    auto start = chrono::system_clock::now();
    BigInt sum = 0;
    int n = data.size() - 2;
    BigInt x = data[0];
    cout << "n = " << n << "; x = " << x << endl;
    for (int i = 1; i <= n; i++) {
        sum += data[i + 1] * pow(x, i);
        // cout << sum << endl;
    }
    sum += data[1];
    auto end = chrono::system_clock::now();
    long nanosecondsElapsed = chrono::nanoseconds(end - start).count();
    cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    cout << "Result: " << sum << endl;
    return sum;
}

BigInt repeatedSquares(vector<BigInt> data) {
    cout << "** REPEATED SQUARES **\n";
    auto start = chrono::system_clock::now();
    BigInt sum = 0;
    int n = data.size() - 2;
    BigInt x = data[0];
    cout << "n = " << n << "; x = " << x << endl;
    int k = 0;
    do {
        k++;
    } while (pow(2, k) <= n);

    vector<vector<BigInt>> decomposedSquares;

    for (int p = 0; p < k; p++) {
        vector<BigInt> square;
        square.push_back(pow(x, pow(2, p)));
        square.push_back(pow(2, p));
        decomposedSquares.push_back(square);
    }

    for (int i = 1; i <= n; i++) {
        BigInt totalDeg = 0;
        BigInt xDeg = 1;
        for (int j = k - 1; j >= 0; j--) {
            BigInt pow2 = decomposedSquares[j][1];
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
    cout << "Time elapsed: " << nanosecondsElapsed << "ns" << endl;
    cout << "Result: " << sum << endl;
    return sum;
}

const void generateData(fstream* dataFile) {
    int n, d;
    cout << "Enter degree of polynomial: ";
    cin >> n;
    cout << "\nEnter number of digits per operand: ";
    cin >> d;
    cout << endl;

    for (int i = 0; i < NUM_EVALUATIONS; i++) {
        BigInt x = big_random(d);
        *dataFile << x << " ";
        for (int j = 0; j <= n; j++) {
            BigInt coefficient = big_random(d);
            *dataFile << coefficient << " ";
        }
        *dataFile << endl;
    }
}

vector<vector<BigInt>> retrieveData(fstream* dataFile) {
    vector<vector<BigInt>> data;
    string dataLine;
    int lineIdx = 0;

    while (getline(*dataFile, dataLine)) {
        vector<BigInt> curLineData;
        string numStr = "";
        for (int i = 0; i < dataLine.length(); i++) {
            if (dataLine[i] == ' ' || dataLine[i] == '\n') {
                try {
                    BigInt num = *new BigInt(numStr);
                    curLineData.push_back(num);
                } catch (exception e) {
                    cout << "Error parsing number: " << e.what() << endl;
                }
                numStr = "";
                continue;
            }
            numStr.push_back(dataLine[i]);
        }
        data.push_back(curLineData);
    }
    return data;
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
    // polynomialOverflow(30, 200);
    fstream dataFile;
    dataFile.open("polynomial-data.txt", ios::in);
    if (!dataFile.is_open()) {
        cerr << "Error opening file polynomial-data.txt" << endl;
        return 1;
    }

    // generateData(&dataFile);
    vector<vector<BigInt>> data = retrieveData(&dataFile);
    dataFile.close();
    // BigInt* horners = new BigInt[NUM_EVALUATIONS];
    // BigInt* brute = new BigInt[NUM_EVALUATIONS];
    BigInt* repSquares = new BigInt[NUM_EVALUATIONS];
    for (int i = 0; i < data.size(); i++) {
        // horners[i] = hornersAlgorithm(data[i]);
        // brute[i] = bruteForce(data[i]);
        repSquares[i] = repeatedSquares(data[i]);
    }
    return 0;
}
