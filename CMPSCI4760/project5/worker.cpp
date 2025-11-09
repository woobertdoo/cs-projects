#include "resources.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct {
    long mtype;
    int intData;
} msgbuffer;

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int BILLION = pow(10, 9);

// Primitive hashmap. Index refers to resource class ID, subindex returns how
// many are currently allocated
int resourcesAllocated[NUM_RESOURCE_CLASSES][1];

void initResourceMatrix() {
    for (int i = 0; i < NUM_RESOURCE_CLASSES; i++)
        resourcesAllocated[i][0] = 0;
}

/*
 * Returns status code based on whether or not a new resource can be requested.
 * 0: Resource is valid to be requested.
 * 1: Resource can not be requested, as it would violate the ordering rule
 * 2: Resource can not be requested, as there are not enough in the system
 * */
int canRequestResource(int resourceClass) {
    if (resourceClass < 9) {
        for (int i = resourceClass + 1; i < NUM_RESOURCE_CLASSES; i++) {
            if (resourcesAllocated[NUM_RESOURCE_CLASSES][0] > 0)
                return 1;
        }
    }
    if (resourcesAllocated[resourceClass][0] == MAX_PER_RESOURCE_CLASS)
        return 2;
    return 0;
}

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

    msgbuffer buf;
    buf.mtype = 1;
    int msqid = 0;
    key_t key;

    if ((key = ftok("msgq.txt", 1)) == -1) {
        perror("ftok");
        return EXIT_FAILURE;
    }

    if ((msqid = msgget(key, 0644)) == -1) {
        perror("msgget in child");
        return EXIT_FAILURE;
    }

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
    int messagesReceived = 0;
    bool shouldTerm = false;
    do {
        if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
            perror("Failed to receive message from parent\n");
            return EXIT_FAILURE;
        }

        printf("Recieved message from parent\n");

        // Check if the program should start termination
        if ((*sec > endSec) || (*sec == endSec && *nano >= endNano))
            shouldTerm = true;

        printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
        printf("SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: "
               "%d\n",
               *sec, *nano, endSec, endNano);
        printf("-- %d messages received from oss\n\n", ++messagesReceived);

        buf.mtype = getppid();
        buf.intData = shouldTerm ? 0 : 1;

        if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) == -1) {
            perror("Failed to send message to parent\n");
            return EXIT_FAILURE;
        }
    } while (!shouldTerm);

    printf("WORKER PID:%d PPID:%d\n", getpid(), getppid());
    printf("SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: "
           "%d\n",
           *sec, *nano, endSec, endNano);
    printf("-- Terminating --\n");

    shmdt(clock);
    return EXIT_SUCCESS;
}
