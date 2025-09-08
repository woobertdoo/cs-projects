#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <sys/shm.h>
#include <unistd.h> // Access to Unix system calls

const int shm_key = ftok("oss.cpp", 0);

int main(int argc, char** argv) {
    // Get number of iterations passed in via execvp
    const int numIter = atoi(argv[1]);

    for (int i = 0; i < numIter; i++) {
        printf("USER PID:%d PPID:%d Iteration:%d before sleeping\n", getpid(),
               getppid(), i);
        sleep(1);
        printf("USER PID:%d PPID:%d Iteration:%d after sleeping\n", getpid(),
               getppid(), i);
    }

    return EXIT_SUCCESS;
}
