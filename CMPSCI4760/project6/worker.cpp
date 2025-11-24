#include "resources.h"
#include <cmath>
#include <cstdlib>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct {
    long mtype;
    pid_t sender;
    int requestOrRelease; // 1 for request, 0 for release
    int resources[NUM_RESOURCE_CLASSES];
    int status; // 1 when worker terminates, 2 if mass release + request
} msgbuffer;

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int BILLION = pow(10, 9);
const int REQUEST_RATIO = 60;

/* Resource Request Codes */
const int REQUEST_AVAILABLE = 0;
const int ERR_RESOURCE_ORDERING = 1;
const int ERR_RESOURCE_FULL = 2;

/* Resource Release Codes */
const int RELEASE_AVAILABLE = 0;
const int ERR_NO_RESOURCE = 1;

// Primitive hashmap. Index refers to resource class ID, value refers to how
// many are currently allocated
int resourcesAllocated[NUM_RESOURCE_CLASSES];

int* clock;
int shm_id;
int msqid;

void initResourceMatrix() {
    for (int i = 0; i < NUM_RESOURCE_CLASSES; i++)
        resourcesAllocated[i] = 0;
}

/*
 * Returns status code based on whether or not a new resource can be requested.
 * REQUEST_AVAILABLE: Resource is valid to be requested.
 * ERR_RESOURCE_ORDERING: Resource can not be requested,
 * as it would violate the ordering rule.
 * ERR_RESOURCE_FULL: Resource can not be requested, as there are not enough in
 * the system.
 * ERR_NO_RESOURCE: Tried to release a resource we currently have none of.
 * */

int canRequestResource(int resourceClass) {
    if (resourcesAllocated[resourceClass] == MAX_PER_RESOURCE_CLASS) {
        return ERR_RESOURCE_FULL;
    }
    for (int i = resourceClass; i < NUM_RESOURCE_CLASSES; i++) {
        if (resourcesAllocated[i] > 0) {
            return ERR_RESOURCE_ORDERING;
        }
    }
    return REQUEST_AVAILABLE;
}

int canReleaseResource(int resourceClass) {
    if (resourcesAllocated[resourceClass] == 0) {
        return ERR_NO_RESOURCE;
    }
    return RELEASE_AVAILABLE;
}

bool resourcesEmpty() {
    for (int i = 0; i < NUM_RESOURCE_CLASSES; i++) {
        if (resourcesAllocated[i] > 0)
            return false;
    }
    return true;
}

void freeAndExit(int sig) {
    // Free shared memory
    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory/n");
        exit(1);
    }

    printf("Worker exiting from SIGTERM\n");

    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Error: Unexpected amount of arguments.\nCorrect "
                        "usage: worker [intervalSec] [intervalNano]\n");
        return EXIT_FAILURE;
    }

    signal(SIGTERM, freeAndExit);

    int intervalSec = atoi(argv[1]);
    int intervalNano = atoi(argv[2]);

    // Print Initialize Statement
    printf("Worker starting, PID:%d, PPID:%d\n", getpid(), getppid());
    printf("Called with:\n");
    printf("Interval: %d seconds, %d nanoseconds\n", intervalSec, intervalNano);

    msgbuffer buf;
    buf.mtype = 1;
    msqid = 0;
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
        fprintf(stderr, "Worker: Error initializing shared memory key\n");
        return EXIT_FAILURE;
    }

    shm_id = shmget(SHM_KEY, BUFF_SZ, 0777);
    if (shm_id <= 0) {
        fprintf(stderr, "Worker: Error initializing shared memory id\n");
        return EXIT_FAILURE;
    }

    clock = (int*)shmat(shm_id, 0, 0);
    if (clock == nullptr) {
        fprintf(stderr, "Worker: Error accessing shared memory\n");
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

    // Track each second passing
    int prevSec = *sec;
    int totalSecs = 0;
    int messagesReceived = 0;
    long lastMessageSentNano = 0;
    bool shouldTerm = false;
    do {

        // Decide time between 0 and 100 ms to send a message
        int randomMessageTime = rand() % 100 * (BILLION / 1000);
        long currentNano = *sec * BILLION + *nano;
        if (currentNano - lastMessageSentNano < randomMessageTime)
            continue;

        buf.mtype = getppid();
        buf.status = 0;
        buf.sender = getpid();
        for (int i = 0; i < NUM_RESOURCE_CLASSES; i++) {
            buf.resources[i] = 0;
        }

        int resourceClass = rand() % NUM_RESOURCE_CLASSES;
        // Worker should request if there are no resources allocated to it
        if (rand() % 100 < REQUEST_RATIO || resourcesEmpty()) {
            buf.requestOrRelease = 1;
            int requestCode = canRequestResource(resourceClass);
            while (requestCode == ERR_RESOURCE_FULL) {
                resourceClass = rand() % NUM_RESOURCE_CLASSES;
                requestCode = canRequestResource(resourceClass);
            }
            if (requestCode == ERR_RESOURCE_ORDERING) {
                // Releasing order-violating resources
                for (int rClass = resourceClass; rClass < NUM_RESOURCE_CLASSES;
                     rClass++) {
                    buf.resources[rClass] = resourcesAllocated[rClass];
                }
                buf.requestOrRelease = 0;
                buf.status = 2;
                if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                    -1) {
                    perror("Failed to send message to parent\n");
                    return EXIT_FAILURE;
                }
                msgbuffer rcvbuf;
                if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(), 0) ==
                    -1) {
                    perror("Failed to receive message from parent\n");
                    return EXIT_FAILURE;
                }

                buf.requestOrRelease = 1;
            }
            buf.status = 0;
            buf.resources[resourceClass] += 1;
            if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                -1) {
                perror("Failed to send message to parent\n");
                return EXIT_FAILURE;
            }
            if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
                perror("Failed to receive message from parent\n");
                return EXIT_FAILURE;
            }

            resourcesAllocated[resourceClass] += 1;

        } else {
            while (canReleaseResource(resourceClass) == ERR_NO_RESOURCE) {
                resourceClass = rand() % NUM_RESOURCE_CLASSES;
            }
            buf.resources[resourceClass] = resourcesAllocated[resourceClass];
            buf.requestOrRelease = 0;
            if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                -1) {
                perror("Failed to send message to parent\n");
                return EXIT_FAILURE;
            }
            if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
                perror("Failed to receive message from parent\n");
                return EXIT_FAILURE;
            }
            resourcesAllocated[resourceClass] = 0;
        }

        lastMessageSentNano = *sec * BILLION + *nano;
        // Check if the program should start termination
        if ((*sec > endSec) || (*sec == endSec && *nano >= endNano)) {
            shouldTerm = true;
            buf.status = 1;
            buf.mtype = getppid();
            buf.sender = getpid();
            for (int rClass = 0; rClass < NUM_RESOURCE_CLASSES; rClass++) {
                buf.resources[rClass] = resourcesAllocated[rClass];
            }
            buf.requestOrRelease = 0;
            if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                -1) {
                perror("Failed to send message to parent\n");
                return EXIT_FAILURE;
            }
            break;
        }
    } while (!shouldTerm);

    shmdt(clock);
    return EXIT_SUCCESS;
}
