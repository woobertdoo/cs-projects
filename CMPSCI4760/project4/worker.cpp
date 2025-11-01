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
const int BLOCK_RATE = 70;

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Error: Unexpected amount of arguments.\nCorrect "
                        "usage: worker [runtimeNano]\n");
        return EXIT_FAILURE;
    }

    int runtimeNano = atoi(argv[1]);

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

    int timeRanNano = 0;
    int messagesReceived = 0;
    bool shouldTerm = false;
    do {
        if (msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
            perror("Failed to receive message from parent\n");
            return EXIT_FAILURE;
        }

        int quantumNano = buf.intData;
        int usedNano = quantumNano;

        buf.mtype = getppid();
        buf.sender = getpid();

        // Check how much time process would run if it was blocked
        if (rand() % 100 <= BLOCK_RATE) {
            // Generate random number of nanoseconds to use, rounding down to
            // the nearest 1000
            usedNano = rand() % buf.intData;
            buf.intData = usedNano;
        }

        // Check if process should terminate before it uses up its time
        if (timeRanNano + usedNano >= runtimeNano) {
            usedNano = runtimeNano - timeRanNano;
            buf.intData = -1 * usedNano;
            shouldTerm = true;
        }

        timeRanNano += usedNano;
        if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) == -1) {
            fprintf(stderr, "Failed to send message to parent!\n");
        }
    } while (!shouldTerm);

    shmdt(clock);
    return EXIT_SUCCESS;
}
