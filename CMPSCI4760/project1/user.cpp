#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <sys/shm.h>
#include <unistd.h> // Access to Unix system calls

const int shm_key = ftok("oss.cpp", 0);

int main(int argc, char** argv) {
    // Get number of iterations passed in via execvp
    const int numIter = atoi(argv[1]);

    // Attach to shared memory buffer from parent
    // The buffer holds number of processes ran and
    // current number of processes running
    int shm_id = shmget(shm_key, sizeof(int) * 2, 0777);
    if (shm_id <= 0) {
        fprintf(stderr, "Error getting shared memory\n");
        return EXIT_FAILURE;
    }

    int* shm_ptr = (int*)shmat(shm_id, 0, 0);
    if (shm_ptr == nullptr) {
        fprintf(stderr, "Error attaching to shared memory\n");
        return EXIT_FAILURE;
    }

    int* totalProcessesRanPtr = shm_ptr;
    int* numProcessesRunningPtr = shm_ptr + 1;

    for (int i = 0; i < numIter; i++) {
        printf("USER PID:%d PPID:%d Iteration:%d before sleeping\n", getpid(),
               getppid(), i);
        sleep(1);
        printf("USER PID:%d PPID:%d Iteration:%d after sleeping\n", getpid(),
               getppid(), i);
    }
    *totalProcessesRanPtr += 1;
    *numProcessesRunningPtr -= 1;

    return EXIT_SUCCESS;
}
