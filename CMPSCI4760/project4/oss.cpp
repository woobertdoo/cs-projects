#include <cmath>
#include <cstdlib>
#include <signal.h>
#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>  // Access to shared memory functions
#include <sys/wait.h> // Access to wait() for parent process
#include <unistd.h>   // Access to Unix system calls

const int SHM_KEY = ftok("oss.cpp", 0);
const int BUFF_SZ = sizeof(int) * 2;
const int NANO_INCR = 10000;
const int BILLION = 1000000000;
const int SCHEDULE_QUANTUM_NS = 10000;
int shm_id;
int* clock;

typedef struct {
    int numProcess;
    int maxSimultaneous;
    float childTimeLimitSec;
    float childIntervalSec;
    char logFile[100];
} options_t;

typedef struct {
    long mtype;
    int intData;
} msgbuffer;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;       // Time process was created
    int startNano;          // Time process was created
    int serviceTimeSeconds; // Total time process has been scheduled
    int serviceTimeNano;    // Total time process has been scheduled
    int usedTimeSeconds;
    int usedTimeNano;
    int eventWaitSec;  // When process will run next
    int eventWaitNano; // When process will run next
    bool blocked;
} PCB_t;

// Global Variables
options_t options;
PCB_t* processTable;

void printUsage(const char* app) {
    fprintf(stderr,
            "Usage: %s [-h] [-n numProcess] [-s maxSimultaneous] "
            "[-t childTimeLimitSec] [-i childIntervalSec]\n",
            app);
    fprintf(stderr, "IMPORTANT: -h SUPERCEDES ALL OTHER OPTIONS!\n");
    fprintf(stderr,
            "@param numProcess: the total number of child processes to run.\n");
    fprintf(stderr, "@param maxSimultaneous: the maximum amount of child "
                    "processes that can be running at a given time.\n");
    fprintf(stderr, "@param childTimeLimitSec: the amount of simulated time "
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
        processTable[i].serviceTimeNano = 0;
        processTable[i].serviceTimeSeconds = 0;
        processTable[i].eventWaitNano = 0;
        processTable[i].eventWaitSec = 0;
        processTable[i].blocked = 0;
    }
}

void addTableEntry(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = true;
            processTable[i].pid = pid;
            processTable[i].startSeconds = clock[0];
            processTable[i].startNano = clock[1];
            processTable[i].blocked = 0;
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

pid_t getNextProcessToSchedule(int* clock[]) {
    float minRatio = MAXFLOAT;
    int nextPIDIndex = -1;
    for (int i = 0; i < options.maxSimultaneous; i++) {
        // Don't try to schedule blocked processes
        // or empty entries in process table
        if (processTable[i].blocked || !processTable[i].occupied)
            continue;
        float totalServiceTime = processTable[i].serviceTimeSeconds * BILLION +
                                 processTable[i].serviceTimeNano;
        float totalUsedTime = processTable[i].usedTimeSeconds * BILLION +
                              processTable[i].usedTimeNano;
        float processTimeRatio;

        if (totalServiceTime == 0) {
            processTimeRatio = 0;
        } else {
            processTimeRatio = totalUsedTime / totalServiceTime;
        }
    }

    return processTable[nextPIDIndex].pid;
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
            processTable[i].blocked = 0;
        }
    }
}

void printSysStart() {
    printf("OSS starting, PID: %d PPID: %d\n", getpid(), getppid());
    printf("Called with\n");
    printf("-n %d\n-s %d\n-t %f\n-i %f\n", options.numProcess,
           options.maxSimultaneous, options.childTimeLimitSec,
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
    options.childTimeLimitSec = 0;
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
            options.childTimeLimitSec = atof(optarg);
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

            if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(), 0) == -1) {
                fprintf(stderr, "failed to receive message in parent\n");
                return EXIT_FAILURE;
            }

            fprintf(
                log_file,
                "OSS: Received message from worker %d PID %d at time %d:%d\n",
                nextChildIndex, nextChildPID, *sec, *nano);
            printf(
                "OSS: Received message from worker %d PID %d at time %d:%d\n",
                nextChildIndex, nextChildPID, *sec, *nano);

            processTable[nextChildIndex].messagesSent += 1;

            if (rcvbuf.intData == 0) {
                fprintf(log_file,
                        "OSS: Worker %d PID %d has decided to terminate\n",
                        nextChildIndex, nextChildPID);
                printf("OSS: Worker %d PID %d has decided to terminate\n",
                       nextChildIndex, nextChildPID);
                wait(0);
                numProcessesRunning--;
                rmProcess(nextChildPID);

                if (totalProcessesRan >= options.numProcess)
                    break;
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

        pid_t new_pid = fork();
        if (new_pid == 0) {
            printProcessTable(*sec, *nano);
            int lifetimeSeconds = std::floor(options.childTimeLimitSec);
            int lifetimeNanoSeconds =
                float(options.childTimeLimitSec - lifetimeSeconds) * BILLION;

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
        totalProcessesRan * options.childTimeLimitSec * BILLION;
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
