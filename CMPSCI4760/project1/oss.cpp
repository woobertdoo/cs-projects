#include <cstdlib>
#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <sys/ipc.h>
#include <sys/shm.h>  // Access to shared memory functions
#include <sys/wait.h> // Access to wait() for parent process
#include <unistd.h>   // Access to Unix system calls

const int shm_key = ftok("oss.cpp", 0);
const int buff_sz = sizeof(int);
typedef struct {
    int numProcess;
    int maxSimultaneous;
    int numIter;
} options_t;

void printUsage(const char* app) {
    fprintf(
        stderr,
        "Usage: %s [-h] [-n numProcess] [-s maxSimultaneous] [-t numIter]\n",
        app);
    fprintf(stderr, "IMPORTANT: -h SUPERCEDES ALL OTHER OPTIONS!\n");
    fprintf(stderr,
            "@param numProcess: the total number of child processes to run.\n");
    fprintf(stderr, "@param maxSimultaneous: the maximum amount of child "
                    "processes that can be running at a given time.\n");
    fprintf(stderr, "@param numIter: the number of iterations each child "
                    "process should run through.\n");
}

int main(int argc, char** argv) {
    options_t options;

    // Set Default Options
    // Default behavior will immediately exit the process
    options.numProcess = 0;
    options.maxSimultaneous = 0;
    options.numIter = 0;

    char opt;
    while ((opt = getopt(argc, argv, "hn:s:t:")) != -1) {
        switch (opt) {
        case 'h':
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        case 'n':
            options.numProcess = atoi(optarg);
            break;
        case 's':
            options.maxSimultaneous = atoi(optarg);
            break;
        case 't':
            options.numIter = atoi(optarg);
            break;
        default:
            printf("Invalid option %c\n", optopt);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    // Create shared memory buffer to hold the total number of processes ran and
    // current number of processes running
    int shm_id = shmget(shm_key, buff_sz * 2, 0777 | IPC_CREAT);
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

    *totalProcessesRanPtr = 0;
    *numProcessesRunningPtr = 0;

    while (*totalProcessesRanPtr < options.numProcess) {
        pid_t childPid =
            fork(); // Create child process to get replaced with ./user
        if (childPid == 0) {
            *numProcessesRunningPtr += 1;
            /* printf(
                 "I'm a copy of the parent. My process id is %d.\nMy parent's "
                 "process id is %d.\n", getpid(), getppid()); */
            char numIterString[4];
            sprintf(numIterString, "%d", options.numIter);
            char* args[] = {(char*)"./user", numIterString, (char*)0};
            execvp(args[0], args);

            // Since the process gets replaced, this will only run if execvp
            // fails for some reason
            fprintf(stderr, "Error executing %s. Terminating.", args[0]);
            return EXIT_FAILURE;
        } else {

            if (*numProcessesRunningPtr >= options.maxSimultaneous) {
                wait(0);
            }
            printf("Number of processes running: %d\n",
                   *numProcessesRunningPtr);
            printf("Total processes ran so far: %d\n", *totalProcessesRanPtr);

            // printf("I'm the parent! My process id is %d.\n", getpid());
        }
    }
    return EXIT_SUCCESS;
}
