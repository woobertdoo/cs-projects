#include "resources.h"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>
#include <string>
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
int msqid;
bool verbose = false;

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
    int resources[NUM_RESOURCE_CLASSES];
    int status; // 1 when worker terminates
} msgbuffer;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;
    int startNano;
    int resourceAllocations[NUM_RESOURCE_CLASSES];
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
            "[-t childLifetimeSec] [-i childIntervalSec] [-v]\n",
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
        for (int j = 0; j < NUM_RESOURCE_CLASSES; j++) {
            processTable[i].resourceAllocations[j] = 0;
        }
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
int getProcessIndex(pid_t pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

// returns 0 if acceptable request, 1 if resources are clogged,
// 2 if acceptable release, -1 if resource code is invalid
int processResourceMessage(msgbuffer* buf, FILE* logFile,
                           bool recheck = false) {
    if (buf->requestOrRelease == 1) {
        for (int rClass = 0; rClass < NUM_RESOURCE_CLASSES; rClass++) {
            int rAmt = buf->resources[rClass];
            if (rAmt == 0)
                continue;
            if (!recheck) {
                lfprintf(
                    logFile,
                    "OSS: Detected Process %d requesting %d instances of R%d "
                    "at time %d:%d\n",
                    buf->sender, rAmt, rClass, clock[0], clock[1]);
                printf(
                    "OSS: Detected Process %d requesting %d instances of R%d "
                    "at time %d:%d\n",
                    buf->sender, rAmt, rClass, clock[0], clock[1]);
            }
            if (resources[rClass].numAllocated + rAmt >
                MAX_PER_RESOURCE_CLASS) {
                if (!recheck && verbose) {
                    lfprintf(logFile,
                             "OSS: Not enough instances of R%d available, "
                             "Process %d "
                             "added to blocked queue at time %d:%d\n",
                             rClass, buf->sender, clock[0], clock[1]);
                    printf("OSS: Not enough instances of R%d available, "
                           "Process %d "
                           "added to blocked queue at time %d:%d\n",
                           rClass, buf->sender, clock[0], clock[1]);
                }
                return 1;
            }
        }
        for (int rClass = 0; rClass < NUM_RESOURCE_CLASSES; rClass++) {
            int rAmt = buf->resources[rClass];
            if (rAmt == 0)
                continue;
            resources[rClass].numAllocated += rAmt;
            resources[rClass].numRequested += rAmt;

            int workerIndex = getProcessIndex(buf->sender);
            if (workerIndex == -1) {
                fprintf(stderr, "Can't find process in PID table\n");
                return -1;
            }

            processTable[workerIndex].resourceAllocations[rClass] += rAmt;
            lfprintf(
                logFile,
                "OSS: Granting Process %d request %d instances of R%d at time "
                "%d:%d\n",
                buf->sender, rAmt, rClass, clock[0], clock[1]);
            printf(
                "OSS: Granting Process %d request %d instances of R%d at time "
                "%d:%d\n",
                buf->sender, rAmt, rClass, clock[0], clock[1]);
        }
        return 0;
    } else if (buf->requestOrRelease == 0) {
        std::string releaseStr = "\t Resources released:";
        bool resourcesReleased = false;
        for (int rClass = 0; rClass < NUM_RESOURCE_CLASSES; rClass++) {
            int rAmt = buf->resources[rClass];
            if (rAmt == 0)
                continue;
            resources[rClass].numAllocated -= rAmt;
            int workerIndex = getProcessIndex(buf->sender);
            if (workerIndex == -1) {
                fprintf(stderr, "Can't find process in PID table\n");
                return -1;
            }

            processTable[workerIndex].resourceAllocations[rClass] -= rAmt;
            releaseStr.append(" R");
            releaseStr.append(std::to_string(rClass));
            releaseStr.append(":");
            releaseStr.append(std::to_string(rAmt));
            resourcesReleased = true;
        }
        if (resourcesReleased && verbose) {
            lfprintf(logFile,
                     "OSS: Acknowledge process %d releasing resources at time "
                     "%d:%d\n",
                     buf->sender, clock[0], clock[1]);
            printf("OSS: Acknowledge process %d releasing resources at time "
                   "%d:%d\n",
                   buf->sender, clock[0], clock[1]);
            lfprintf(logFile, "%s\n", releaseStr.c_str());
            printf("%s\n", releaseStr.c_str());
        }
        return 2;
    } else {
        fprintf(stderr, "OSS: Error: Invalid request code %d from %d\n",
                buf->requestOrRelease, buf->sender);
        return -1;
    }
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
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory/n");
        exit(1);
    }

    exit(1);
}

void printProcessTable(int currentSec, int currentNano) {
    printf("OSS PID:%d SysclockS:%d SysclockNano:%d\n", getpid(), currentSec,
           currentNano);
    printf("Process Table:\n");
    printf("Entry Occupied PID StartS StartN\n");
    for (int i = 0; i < options.maxSimultaneous; i++) {
        const char* occupiedStatus =
            processTable[i].occupied ? "True" : "False";
        printf("%d   %s       %d   %d    %d\n", i, occupiedStatus,
               processTable[i].pid, processTable[i].startSeconds,
               processTable[i].startNano);
    }
}

void printResourceTable() {
    printf("\t       R0  R1  R2  R3  R4  R5  R6  R7  R8  R9\n");
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].pid == 0)
            continue;
        std::string resourceRow = "Worker ";
        resourceRow.append(std::to_string(processTable[i].pid));
        for (int j = 0; j < NUM_RESOURCE_CLASSES; j++) {
            resourceRow.append("   ");
            resourceRow.append(
                std::to_string(processTable[i].resourceAllocations[j]));
        }
        printf("%s\n", resourceRow.c_str());
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
            for (int j = 0; j < NUM_RESOURCE_CLASSES; j++) {
                processTable[i].resourceAllocations[j] = 0;
            }
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
    signal(SIGALRM, freeAndExit);
    signal(SIGINT, freeAndExit);
    alarm(5);

    // Set Default Options
    // Default behavior will immediately exit the process
    options.numProcess = 0;
    options.maxSimultaneous = 0;
    options.childLifetimeSec = 0;
    options.childIntervalSec = 1;

    /* Parsing options */

    char opt;
    while ((opt = getopt(argc, argv, "hvn:s:t:i:f:")) != -1) {
        switch (opt) {
        case 'h':
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        case 'v':
            verbose = true;
            break;
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
    int grantedRequests = 0;
    int immediateRequests = 0;
    int numMassReleases = 0;

    while (totalProcessesRan < options.numProcess || !isProcessTableEmpty()) {

        incrementClock(sec, nano);

        for (int i = 0; i < outstandingRequests.size(); i++) {
            msgbuffer buf = outstandingRequests[i];
            int resourceCode = processResourceMessage(&buf, log_file, true);
            if (resourceCode == 0 || resourceCode == 2) {
                grantedRequests++;
                msgbuffer returnbuf = buf;
                returnbuf.mtype = buf.sender;
                returnbuf.sender = getpid();
                if (msgsnd(msqid, &returnbuf, sizeof(msgbuffer) - sizeof(long),
                           0) == -1) {
                    fprintf(stderr,
                            "msgsnd to child with process id %ld failed\n",
                            returnbuf.mtype);
                    return EXIT_FAILURE;
                }
                outstandingRequests.erase(outstandingRequests.begin() + i);
            }
        }
        msgbuffer rcvbuf;

        if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(), IPC_NOWAIT) ==
            -1) {
            if (errno != ENOMSG) {
                fprintf(stderr, "failed to receive message in parent\n");
                return EXIT_FAILURE;
            }
        } else {
            if (rcvbuf.status == 1) {
                processResourceMessage(&rcvbuf, log_file);
                rmProcess(rcvbuf.sender);
                printf("OSS: Worker %d terminating\n", rcvbuf.sender);
                printf("Total processes ran: %d\n", ++totalProcessesRan);
                numProcessesRunning--;
            } else {
                if (rcvbuf.status == 2)
                    numMassReleases++;
                int resourceCode = processResourceMessage(&rcvbuf, log_file);
                if (resourceCode == 0 || resourceCode == 2) {
                    immediateRequests++;
                    grantedRequests++;
                    msgbuffer returnbuf = rcvbuf;
                    returnbuf.mtype = rcvbuf.sender;
                    returnbuf.sender = getpid();
                    if (msgsnd(msqid, &returnbuf,
                               sizeof(msgbuffer) - sizeof(long), 0) == -1) {
                        fprintf(stderr,
                                "msgsnd to child with process id %ld failed\n",
                                returnbuf.mtype);
                        return EXIT_FAILURE;
                    }
                } else if (resourceCode == -1) {
                    return EXIT_FAILURE;
                } else if (resourceCode == 1) {
                    outstandingRequests.push_back(rcvbuf);
                }
            }
        }

        if (grantedRequests % 20 == 0 && grantedRequests > 0) {
            printResourceTable();
        }

        long lastTime = lastSec * BILLION + lastNano;
        long now = *sec * BILLION + *nano;

        if (now - lastTime >= BILLION / 2) {
            lastSec = *sec;
            lastNano = *nano;
            printProcessTable(*sec, *nano);
            printResourceTable();
        }

        if (numProcessesRunning >= options.maxSimultaneous)
            continue;

        if (numProcessesRunning > SYS_MAX_SIMUL_PROCS)
            continue;

        if (totalProcessesRan >= options.numProcess)
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
            numProcessesRunning += 1;
        }
    }

    int totalResourcesRequested = 0;
    for (int rClass = 0; rClass < NUM_RESOURCE_CLASSES; rClass++) {
        totalResourcesRequested += resources[rClass].numRequested;
    }

    float immediateGrantPercent =
        ((float)immediateRequests / (float)grantedRequests) * 100;

    long long totalNano =
        totalProcessesRan * options.childLifetimeSec * BILLION;
    int finalNano = totalNano % BILLION;
    int totalSecs = (totalNano - finalNano) / BILLION;
    printf("OSS PID: %d Terminating\n", getpid());

    printf("-------------\n");
    printf("Total number of resources requested: %d\n",
           totalResourcesRequested);
    printf("Total number of mass releases: %d\n", numMassReleases);
    printf("Percentage of immediate request grantings: %.2f%%\n",
           immediateGrantPercent);

    /* Clean Up Shared Memory */

    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory/n");
        exit(1);
    }

    return EXIT_SUCCESS;
}
