#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int BILLION = pow(10, 9);

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Error: Unexpected amount of arguments.\nCorrect "
                        "usage: worker [intervalSec] [intervalNano]\n");
        return EXIT_FAILURE;
    }

    int intervalSec = atoi(argv[1]);
    int intervalNano = atoi(argv[2]);

    // Print Initialize Statement
    printf("Worker starting, PID:%d, PPID:%d\n", getpid(), getppid());
    printf("Called with:\n");
    printf("Interval: %d seconds, %d nanoseconds\n", intervalSec, intervalNano);

    /* Setting up shared memory */
    if (SHM_KEY <= 0) {
        fprintf(stderr, "Parent: Error initializing shared memory key\n");
        return EXIT_FAILURE;
    }

    int shm_id = shmget(SHM_KEY, BUFF_SZ, 0777);
    if (shm_id <= 0) {
        fprintf(stderr, "Parent: Error initializing shared memory id\n");
        return EXIT_FAILURE;
    }

    int* clock = (int*)shmat(shm_id, 0, 0);
    if (clock == nullptr) {
        fprintf(stderr, "Parent: Error accessing shared memory\n");
        return EXIT_FAILURE;
    }

    int* sec = &(clock[0]);
    int* nano = &(clock[1]);

    // Set end times for worker to watch out for
    int endSec = *sec + intervalSec;
    int endNano = *nano + intervalNano;

    // Loop around if the nanoseconds add up to more than 1 second
    if (endNano > BILLION) {
        endSec += 1;
        endNano -= BILLION;
    }

    // Print Start Statements
    printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
    printf("SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n",
           *sec, *nano, endSec, endNano);
    printf("-- Just Starting --\n");

    // Track each second passing
    int prevSec = *sec;
    int totalSecs = 0;
    while (true) {
        // Check if the program should start termination
        if ((*sec > endSec) || (*sec == endSec && *nano >= endNano))
            break;

        // Check to reprint the table
        if (*sec > prevSec) {
            totalSecs += 1;
            printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
            printf("SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: "
                   "%d\n",
                   *sec, *nano, endSec, endNano);
            printf("-- %d seconds have passed starting --\n", totalSecs);
        }

        prevSec = *sec;
    }

    printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
    printf("SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: "
           "%d\n",
           *sec, *nano, endSec, endNano);
    printf("-- Terminating --\n");
    shmdt(clock);
    return EXIT_SUCCESS;
}
