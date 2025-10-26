#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

typedef struct {
    long mtype;
    pid_t sender;
    int intData;
} msgbuffer;

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int BILLION = pow(10, 9);

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Error: Unexpected amount of arguments.\nCorrect "
                        "usage: worker [intervalSec] [intervalNano]\n");
        return EXIT_FAILURE;
    }

    int upperLimitSec = atoi(argv[1]);
    int upperLimitNano = upperLimitSec * BILLION + atoi(argv[2]);

    // Generate random number of nanoseconds to use, rounding down to the
    // nearest 1000
    int usedNano = std::floor(rand() / 1000) * 1000;
    int usedSec = std::floor(usedNano / BILLION);
    usedNano -= usedSec * BILLION;

    // Print Initialize Statement
    printf("Worker starting, PID:%d, PPID:%d\n", getpid(), getppid());
    printf("Called with:\n");
    printf("Interval: %d seconds, %d nanoseconds\n", usedSec, usedNano);

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
    int endSec = *sec + usedSec;
    int endNano = *nano + usedNano;

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
