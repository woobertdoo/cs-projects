#include <iostream>
#include <random>
using namespace std;

void initializeArray(int[], int);
int *shuffleArray(const int[], int);
void printArray(const int[], int);
int *createOddArray(const int[], int, int &);
int *createEvenArray(const int[], int, int &);
void arrayWar(int *, int, int *, int);
void sortArray(int *, int);

int main() {
    int SIZE;
    int *array = nullptr;
    int *shuffledArray = nullptr;

    cout << "Enter a number between 1 and 52, inclusive: ";

    cin >> SIZE;

    while (SIZE < 1 || SIZE > 52) {
        cout << "Invalid input. Please enter a number between 1 and 52: ";
        cin >> SIZE;
    }

    array = new int[SIZE];

    initializeArray(array, SIZE);
    cout << "Original Array: ";
    printArray(array, SIZE);
    shuffledArray = shuffleArray(array, SIZE);
    cout << "Shuffled Array: ";
    printArray(shuffledArray, SIZE);

    cout << "Now we'll split it into odds and evens\n";

    int oddCount = 0, evenCount = 0;
    int *oddsArray = createOddArray(shuffledArray, SIZE, oddCount);
    int *evensArray = createEvenArray(shuffledArray, SIZE, evenCount);

    cout << "The array of all the odd numbers is: ";
    printArray(oddsArray, oddCount);
    cout << "The array of all the even numbers is: ";
    printArray(evensArray, evenCount);

    cout << "Now we'll make the odds array and evens array duke it out!\n";
    arrayWar(oddsArray, oddCount, evensArray, evenCount);

    delete[] array;
    array = nullptr;
    delete[] oddsArray;
    oddsArray = nullptr;
    delete[] evensArray;
    evensArray = nullptr;
    return 0;
}

void initializeArray(int array[], int size) {
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
}

int *shuffleArray(const int array[], int size) {
    int *shuffled = new int[size];
    int temp;
    srand(time(0));
    // Copy the parameter array into the shuffled array.
    for (int i = 0; i < size; i++)
        shuffled[i] = array[i];

    // Swap elements randomly in the shuffled array.
    for (int i = size - 1; i >= 0; i--) {
        int j = rand() % (i + 1);
        temp = shuffled[i];
        shuffled[i] = shuffled[j];
        shuffled[j] = temp;
    }

    return shuffled;
}

void printArray(const int array[], int size) {
    cout << "{";
    for (int i = 0; i < size; i++) {
        if (i < size - 1) {
            cout << array[i] << ", ";
        } else {
            cout << array[i] << "}" << endl;
        }
    }
}

int *createOddArray(const int array[], int size, int &oddCount) {
    for (int i = 0; i < size; i++) {
        // If the value at array[i] is odd, increment the odd number counter
        if (array[i] % 2 == 1)
            oddCount++;
    }

    // Create an array with a size equal to the number of odd numbers
    int *oddArray = new int[oddCount];
    int j = 0; // Counter to increment through the oddsArray
    for (int i = 0; i < size; i++) {
        if (array[i] % 2 == 1) {
            oddArray[j] = array[i];
            j++;
        }
    }

    return oddArray;
}

int *createEvenArray(const int array[], int size, int &evenCount) {
    for (int i = 0; i < size; i++) {
        if (array[i] % 2 == 0)
            evenCount++;
    }

    int *evenArray = new int[evenCount];
    int j = 0; // Counter to increment through the evensArray
    for (int i = 0; i < size; i++) {
        if (array[i] % 2 == 0) {
            evenArray[j] = array[i];
            j++;
        }
    }

    return evenArray;
}

void arrayWar(int *arrayOne, int sizeOne, int *arrayTwo, int sizeTwo) {
    int *finalArray = nullptr;
    int finalArraySize = 0;

    if (sizeOne > sizeTwo) {
        finalArraySize = sizeOne;
        finalArray = new int[finalArraySize];
        // When both arrays share an index, see which value is larger and add it
        // to the final array
        for (int i = 0; i < sizeTwo; i++) {
            if (arrayOne[i] > arrayTwo[i]) {
                finalArray[i] = arrayOne[i];
            } else {
                finalArray[i] = arrayTwo[i];
            }
        }

        // Once one array runs out of indices, the remaining indices of the
        // final array get populated with what's left in the larger array
        for (int i = sizeTwo; i < finalArraySize; i++) {
            finalArray[i] = arrayOne[i];
        }
    } else if (sizeTwo > sizeOne) {
        finalArraySize = sizeTwo;
        finalArray = new int[finalArraySize];
        for (int i = 0; i < sizeOne; i++) {
            if (arrayTwo[i] > arrayOne[i]) {
                finalArray[i] = arrayTwo[i];
            } else {
                finalArray[i] = arrayOne[i];
            }
        }

        for (int i = sizeOne; i < finalArraySize; i++) {
            finalArray[i] = arrayTwo[i];
        }
    } else {
        finalArraySize = sizeOne;
        finalArray = new int[finalArraySize];
        for (int i = 0; i < finalArraySize; i++) {
            if (arrayTwo[i] > arrayOne[i]) {
                finalArray[i] = arrayTwo[i];
            } else {
                finalArray[i] = arrayOne[i];
            }
        }
    }

    cout << "The results of the array war are: ";
    printArray(finalArray, finalArraySize);

    sortArray(finalArray, finalArraySize);
    cout << "Once sorted, the final array becomes: ";
    printArray(finalArray, finalArraySize);
}

void sortArray(int *array, int size) {
    int minIndex, minValue;
    for (int start = 0; start < size - 1; start++) {
        minIndex = start;
        minValue = array[start];
        for (int i = start + 1; i < size; i++) {
            if (array[i] < minValue) {
                minValue = array[i];
                minIndex = i;
            }
        }
        swap(array[minIndex], array[start]);
    }
}