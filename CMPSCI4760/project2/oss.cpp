#include <cmath>
#include <cstdlib>
#include <signal.h>
#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <sys/ipc.h>
#include <sys/shm.h>  // Access to shared memory functions
#include <sys/wait.h> // Access to wait() for parent process
#include <unistd.h>   // Access to Unix system calls

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int NANO_INCR = 10000;
const int BILLION = 1000000000;
int shm_id;
int* clock;

typedef struct {
    int numProcess;
    int maxSimultaneous;
    float childLifetimeSec;
    float childIntervalSec;
} options_t;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;
    int startNano;
} PCB_t;

// Global Variables
options_t options;
PCB_t* processTable;

void printUsage(const char* app) {
    fprintf(stderr,
            "Usage: %s [-h] [-n numProcess] [-s maxSimultaneous] "
            "[-t childLifetimeSec] [-i childIntervalSec]\n",
            app);
    fprintf(stderr, "IMPORTANT: -h SUPERCEDES ALL OTHER OPTIONS!\n");
    fprintf(stderr,
            "@param numProcess: the total number of child processes to run.\n");
    fprintf(stderr, "@param maxSimultaneous: the maximum amount of child "
                    "processes that can be running at a given time.\n");
    fprintf(stderr, "@param childLifetimeSec: the amount of simulated time "
                    "each child should run for"
                    "\n");
    fprintf(stderr, "@param childIntevalSec: the minimum amount of simulated "
                    "time between each child opening\n");
}

bool isProcessTableEmpty() {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].occupied == true)
            return false;
    }
    return true;
}

void initiateProcessTable() {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        processTable[i].occupied = false;
        processTable[i].pid = 0;
        processTable[i].startNano = 0;
        processTable[i].startSeconds = 0;
    }
}

void addTableEntry(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = true;
            processTable[i].pid = pid;
            processTable[i].startSeconds = clock[0];
            processTable[i].startNano = clock[1];
            return;
        }
    }
}

void incrementClock(int* currentSec, int* currentNano) {
    *currentNano += NANO_INCR;
    if (*currentNano > BILLION) {
        *currentNano -= BILLION;
        *currentSec += 1;
    }
}

void handleTimeout(int sig) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].occupied) {
            kill(processTable[i].pid, SIGTERM);
        }
    }

    // Free shared memory
    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);

    exit(1);
}

void printProcessTable(int currentSec, int currentNano) {
    printf("OSS PID:%d SysclockS:%d SysclockNano:%d\n", getpid(), currentSec,
           currentNano);
    printf("Process Tabe:\n");
    printf("Entry Occupied PID StartS StartN\n");
    for (int i = 0; i < options.maxSimultaneous; i++) {
        const char* occupiedStatus =
            processTable[i].occupied ? "True" : "False";
        printf("%d   %s       %d   %d    %d\n", i, occupiedStatus,
               processTable[i].pid, processTable[i].startSeconds,
               processTable[i].startNano);
    }
}

// Remove process from table via pid
void rmProcess(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].pid == pid) {
            processTable[i].pid = 0;
            processTable[i].startNano = 0;
            processTable[i].startSeconds = 0;
            processTable[i].occupied = false;
        }
    }
}

void printSysStart() {
    printf("OSS starting, PID: %d PPID: %d\n", getpid(), getppid());
    printf("Called with\n");
    printf("-n %d\n-s %d\n-t %f\n-i %f\n", options.numProcess,
           options.maxSimultaneous, options.childLifetimeSec,
           options.childIntervalSec);
}

int main(int argc, char** argv) {

    // Set up signal handler
    signal(SIGALRM, handleTimeout);
    alarm(60);

    // Set Default Options
    // Default behavior will immediately exit the process
    options.numProcess = 0;
    options.maxSimultaneous = 0;
    options.childLifetimeSec = 0;
    options.childIntervalSec = 1;

    /* Parsing options */

    char opt;
    while ((opt = getopt(argc, argv, "hn:s:t:i:")) != -1) {
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
            options.childLifetimeSec = atoi(optarg);
            break;
        case 'i':
            options.childIntervalSec = atoi(optarg);
            break;
        default:
            printf("Invalid option %c\n", optopt);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    printSysStart();

    /* Setting up shared memory */
    if (SHM_KEY <= 0) {
        fprintf(stderr, "Parent: Error initializing shared memory key\n");
        return EXIT_FAILURE;
    }

    shm_id = shmget(SHM_KEY, BUFF_SZ, 0777 | IPC_CREAT);
    if (shm_id <= 0) {
        fprintf(stderr, "Parent: Error initializing shared memory id\n");
        return EXIT_FAILURE;
    }

    clock = (int*)shmat(shm_id, 0, 0);
    if (clock == nullptr) {
        fprintf(stderr, "Parent: Error accessing shared memory\n");
        return EXIT_FAILURE;
    }

    int* sec = &(clock[0]);
    int* nano = &(clock[1]);
    *sec = *nano = 0;

    processTable = new PCB_t[options.maxSimultaneous];
    initiateProcessTable();
    int totalProcessesRan = 0;
    int numProcessesRunning = 0;

    int lastNano = *nano;
    int lastSec = *sec;
    while (totalProcessesRan < options.numProcess || !isProcessTableEmpty()) {

        incrementClock(sec, nano);

        long lastTime = lastSec * BILLION + lastNano;
        long now = *sec * BILLION + *nano;

        if (now - lastTime >= BILLION / 2) {
            lastSec = *sec;
            lastNano = *nano;
            printProcessTable(*sec, *nano);
        }

        int status;
        int pid = waitpid(-1, &status, WNOHANG);

        if (numProcessesRunning >= options.maxSimultaneous) {
            if (pid > 0) {
                rmProcess(pid);
                numProcessesRunning -= 1;
            } else {
                continue;
            }
        }
        pid_t new_pid = fork();
        if (new_pid == 0) {
            addTableEntry(getpid());
            int lifetimeSeconds = std::floor(options.childLifetimeSec);
            int lifetimeNanoSeconds =
                float(options.childLifetimeSec - lifetimeSeconds) * BILLION;

            char lifetimeSecStr[4];
            char lifetimeNanoStr[9];
            sprintf(lifetimeSecStr, "%d", lifetimeSeconds);
            sprintf(lifetimeNanoStr, "%d", lifetimeNanoSeconds);

            char* args[] = {(char*)"./worker", lifetimeSecStr, lifetimeNanoStr,
                            (char*)0};
            execvp(args[0], args);

            // Since the process gets replaced, this will only run if execvp
            // fails for some reason
            fprintf(stderr, "Error executing %s. Terminating.", args[0]);
            return EXIT_FAILURE;
        } else {
            totalProcessesRan += 1;
            numProcessesRunning += 1;
        }
    }

    while (numProcessesRunning > 0) {
        incrementClock(sec, nano);
        int status;
        int pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            rmProcess(pid);
            numProcessesRunning -= 1;
        }
    }

    int totalSecs = totalProcessesRan * std::floor(options.childLifetimeSec);
    printf("OSS PID: %d Terminating\n", getpid());
    printf("%d workers were launched and terminated\n", totalProcessesRan);
    printf("");

    /* Clean Up Shared Memory */

    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
