#include "resources.h"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>  // Access to shared memory functions
#include <sys/wait.h> // Access to wait() for parent process
#include <unistd.h>   // Access to Unix system calls
#include <vector>

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int NANO_INCR = 10000;
const int BILLION = 1000000000;
const int SYS_MAX_SIMUL_PROCS = 18;
int shm_id;
int* clock;

typedef struct {
    int numProcess;
    int maxSimultaneous;
    float childLifetimeSec;
    float childIntervalSec;
    char logFile[100];
} options_t;

typedef struct {
    long mtype;
    pid_t sender;
    int requestOrRelease; // 1 for request, 0 for release
    int resourceClass;
    int resourceAmount;
    int status; // 1 when worker terminates
} msgbuffer;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;
    int startNano;
    int messagesSent;
} PCB_t;

// Global Variables
options_t options;
PCB_t* processTable;
std::vector<msgbuffer> outstandingRequests;

int lfprintf(FILE* stream, const char* format, ...) {
    static int lineCount = 0;
    lineCount++;

    if (lineCount > 10000)
        return 1;

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);

    return 0;
}
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
        processTable[i].messagesSent = 0;
    }
}

void addTableEntry(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = true;
            processTable[i].pid = pid;
            processTable[i].startSeconds = clock[0];
            processTable[i].startNano = clock[1];
            processTable[i].messagesSent = 0;
            return;
        }
    }
}

void incrementClock(int* currentSec, int* currentNano,
                    int numProcessesRunning) {
    *currentNano += 250000000 / numProcessesRunning;
    if (*currentNano > BILLION) {
        *currentNano -= BILLION;
        *currentSec += 1;
    }
}

int processResourceMessage(msgbuffer* buf, FILE* logFile) {
    if (buf->requestOrRelease == 1) {
        int rClass = buf->resourceClass;
        int rAmt = buf->resourceAmount;
        lfprintf(logFile,
                 "OSS: Detected Process %d requesting %d instances of R%d "
                 "at time %d:%d\n",
                 buf->sender, rAmt, rClass, clock[0], clock[1]);
        printf("OSS: Detected Process %d requesting %d instances of R%d "
               "at time %d:%d\n",
               buf->sender, rAmt, rClass, clock[0], clock[1]);
        if (resources[rClass].numAllocated + rAmt > MAX_PER_RESOURCE_CLASS) {
            outstandingRequests.push_back(*buf);
            lfprintf(logFile,
                     "OSS: Not enough instances of R%d available, Process %d "
                     "added to blocked queue at time %d:%d\n",
                     rClass, buf->sender, clock[0], clock[1]);
            printf("OSS: Not enough instances of R%d available, Process %d "
                   "added to blocked queue at time %d:%d\n",
                   rClass, buf->sender, clock[0], clock[1]);
            return 1;
        }
        lfprintf(logFile,
                 "OSS: Granting Process %d request %d instances of R%d at time "
                 "%d:%d\n",
                 buf->sender, rAmt, rClass, clock[0], clock[1]);
        printf("OSS: Granting Process %d request %d instances of R%d at time "
               "%d:%d\n",
               buf->sender, rAmt, rClass, clock[0], clock[1]);
        return 0;
    } else if (buf->requestOrRelease == 0) {
        int rClass = buf->resourceClass;
        int rAmt = buf->resourceAmount;
        resources[rClass].numAllocated -= rAmt;
        lfprintf(
            logFile,
            "OSS: Acknowledge process %d releasing resources at time %d:%d\n",
            buf->sender, clock[0], clock[1]);
        lfprintf(logFile, "\t Resources released: R%d:%d\n", rClass, rAmt);
        return 0;
    } else {
        fprintf(stderr, "OSS: Error: Invalid request code from %d\n",
                buf->sender);
        return -1;
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

void freeAndExit(int sig) {
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
            processTable[i].messagesSent = 0;
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
    signal(SIGINT, freeAndExit);
    alarm(60);

    // Set Default Options
    // Default behavior will immediately exit the process
    options.numProcess = 0;
    options.maxSimultaneous = 0;
    options.childLifetimeSec = 0;
    options.childIntervalSec = 1;

    /* Parsing options */

    char opt;
    while ((opt = getopt(argc, argv, "hn:s:t:i:f:")) != -1) {
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
            options.childLifetimeSec = atof(optarg);
            break;
        case 'i':
            options.childIntervalSec = atof(optarg);
            break;
        case 'f':
            strcpy(options.logFile, optarg);
            break;
        default:
            printf("Invalid option %c\n", optopt);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    printSysStart();

    FILE* log_file;
    log_file = fopen(options.logFile, "w");

    if (log_file == NULL) {
        fprintf(stderr, "Error: Could not write to log file!\n");
        return EXIT_FAILURE;
    }

    /* Setting up message queue */
    msgbuffer buffers[options.maxSimultaneous];
    int msqid;
    key_t key;
    system("touch msgq.txt");

    if ((key = ftok("msgq.txt", 1)) == -1) {
        perror("ftok");
        return EXIT_FAILURE;
    }

    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget in parent");
        return EXIT_FAILURE;
    }

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
    int nextChildIndex = -1;
    int messagesSent = 0;

    while (totalProcessesRan < options.numProcess || !isProcessTableEmpty()) {

        if (numProcessesRunning > 0) {
            incrementClock(sec, nano, numProcessesRunning);
            nextChildIndex = (nextChildIndex + 1) % numProcessesRunning;
            // keep incrementing index until a running process is encountered
            while (!processTable[nextChildIndex].occupied) {
                nextChildIndex = (nextChildIndex + 1) % numProcessesRunning;
            }
        }

        if (nextChildIndex >= 0) {

            pid_t nextChildPID = processTable[nextChildIndex].pid;

            buffers[nextChildIndex].mtype = nextChildPID;
            buffers[nextChildIndex].intData = nextChildPID;

            fprintf(log_file,
                    "OSS: Sending message to worker %d PID %d at time %d:%d\n",
                    nextChildIndex, nextChildPID, *sec, *nano);
            printf("OSS: Sending message to worker %d PID %d at time %d:%d\n",
                   nextChildIndex, nextChildPID, *sec, *nano);

            if (msgsnd(msqid, &buffers[nextChildIndex],
                       sizeof(msgbuffer) - sizeof(long), 0) == -1) {
                fprintf(stderr, "msgsnd to child with process id %d failed\n",
                        nextChildPID);
                return EXIT_FAILURE;
            }
            messagesSent++;

            msgbuffer rcvbuf;

            if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(),
                       IPC_NOWAIT) == -1) {
                if (errno != ENOMSG) {
                    fprintf(stderr, "failed to receive message in parent\n");
                    return EXIT_FAILURE;
                }
            } else {
            }
        }

        long lastTime = lastSec * BILLION + lastNano;
        long now = *sec * BILLION + *nano;

        if (now - lastTime >= BILLION / 2) {
            lastSec = *sec;
            lastNano = *nano;
            printProcessTable(*sec, *nano);
        }

        if (numProcessesRunning >= options.maxSimultaneous)
            continue;

        if (numProcessesRunning > SYS_MAX_SIMUL_PROCS)
            continue;

        pid_t new_pid = fork();
        if (new_pid == 0) {
            printProcessTable(*sec, *nano);
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
            printf("New PID: %d\n", new_pid);
            addTableEntry(new_pid);
            totalProcessesRan += 1;
            numProcessesRunning += 1;
        }
    }

    long long totalNano =
        totalProcessesRan * options.childLifetimeSec * BILLION;
    int finalNano = totalNano % BILLION;
    int totalSecs = (totalNano - finalNano) / BILLION;
    printf("OSS PID: %d Terminating\n", getpid());
    printf("%d workers were launched and terminated\n", totalProcessesRan);
    printf("OSS sent a total of %d messages\n", messagesSent);

    /* Clean Up Shared Memory */

    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
