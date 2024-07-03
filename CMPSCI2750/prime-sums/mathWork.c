#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void help() {
    printf("The arguments this program takes: One filename and a series of positive integers\n");
    printf("It will create and print an array of those integers.\n");
    printf("Then it will print out all pairs of prime numbers in the array that sum to less than "
           "50.\n");
}

int isPrime(int num) {
    int j, upper_check = floor(sqrt(num));
    for (j = 2; j <= upper_check; j++) {
        if (num % j == 0) {
            return 0;
        }
    }
    return 1;
}

void printArray(int *arr, int size, FILE *outputFP) {
    int i;
    for (i = 0; i < size; i++) {
        fprintf(outputFP, "%d ", arr[i]);
    }
    fprintf(outputFP, "\n");
}

int printPrimePairs(int *arr, int size, FILE *outputFP) {
    int i, j, primeCount = 0, *primes;

    for (i = 0; i < size; i++) {
        if (arr[i] == 2 || arr[i] == 3) {
            primeCount++;
        } else if (isPrime(arr[i]) == 1) {
            primeCount++;
        }
    }

    primes = (int *)malloc(primeCount * sizeof(int));
    int primeIdx = 0;
    for (i = 0; i < size; i++) {
        if (arr[i] == 2 || arr[i] == 3) {
            primes[primeIdx] = arr[i];
            primeIdx++;
        } else if (isPrime(arr[i]) == 1) {
            primes[primeIdx] = arr[i];
            primeIdx++;
        }
    }

    fprintf(outputFP, "Pair:");
    int pairs = 0;
    for (i = 0; i < primeCount; i++) {
        for (j = i + 1; j < primeCount; j++) {
            if (primes[i] + primes[j] < 50) {
                pairs++;
                fprintf(outputFP, " %d-%d;", primes[i], primes[j]);
            }
        }
    }
    fprintf(outputFP, "\n");
    free(primes);
    return pairs;
}

int main(int argc, char **argv) {
    int option;
    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
        case 'h':
            help();
            return 1;
        case '?':
            if (isprint(optopt)) {
                fprintf(stderr, "Unknown option -%c\n", optopt);
            } else {
                fprintf(stderr, "Unknown option character '\\x%x'\n", optopt);
            }
            return 1;
        default:
            help();
            return 1;
        }
    }

    int arrSize = argc - 2;
    int *array = (int *)malloc(arrSize * sizeof(int));
    int i;

    for (i = 0; i < arrSize; i++) {
        if (!atoi(argv[i + 2])) {
            fprintf(stderr, "Failed parsing int!\n");
            exit(EXIT_FAILURE);
        }
        array[i] = atoi(argv[i + 2]);
    }

    char *filename = argv[1];
    FILE *outputFP = fopen(filename, "a");
    if (outputFP == NULL) {
        fprintf(stderr, "Bad file read. %s is invalid file\n", filename);
        return EXIT_FAILURE;
    }

    int status;
    pid_t childPid = fork();
    if (childPid == 0) {
        fprintf(outputFP, "Child: %d: ", getpid());
        printArray(array, arrSize, outputFP);
        fprintf(outputFP, "Child: %d: ", getpid());
        int pairs = printPrimePairs(array, arrSize, outputFP);
        free(array);
        if (pairs == 0) {
            exit(EXIT_FAILURE);
        } else {
            exit(EXIT_SUCCESS);
        }
    } else {
        wait(&status);
        char *exit_status = status == 0 ? "EXIT_SUCCESS" : "EXIT_FAILURE";
        fprintf(outputFP, "Parent: %d: %s\n", childPid, exit_status);
    }

    return 0;
}
