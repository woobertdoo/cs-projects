#include <algorithm>
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
const int BILLION = 1000000000;
const int SCHEDULE_QUANTUM_NS = 50000000;
const int SCHEDULE_DECISION_TIME_NS = 100000;
const int CHANGE_QUEUE_TIME_NS = 500000;
const int BLOCK_DELAY_NS = 600000000;
const int IDLE_TIME_NS = 200000000;
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
    pid_t sender;
    int intData;
} msgbuffer;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;       // Time process was created
    int startNano;          // Time process was created
    int serviceTimeSeconds; // Total time process has been scheduled
    int serviceTimeNano;    // Total time process has been scheduled
    int usedTimeSeconds;    // How much time as been used
    int usedTimeNano;       // How much time has been used
    int eventWaitSec;       // When process will run next
    int eventWaitNano;      // When process will run next
    int whenBlockedSec;
    int whenBlockedNano;
    int runtimeNano; // Total time the process will run,
    float queuePriority;
    bool blocked;
} PCB_t;

// Global Variables
options_t options;
PCB_t* processTable;

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
        processTable[i].whenBlockedSec = 0;
        processTable[i].whenBlockedNano = 0;
        processTable[i].queuePriority = 0;
        processTable[i].blocked = 0;
    }
}

void updateProcessUsedTime(int processIndex, int toAddNS) {
    processTable[processIndex].usedTimeNano += toAddNS;
    while (processTable[processIndex].usedTimeNano >= BILLION) {
        processTable[processIndex].usedTimeNano -= BILLION;
        processTable[processIndex].usedTimeSeconds += 1;
    }
}

void addTableEntry(int pid, int burstNano) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = true;
            processTable[i].pid = pid;
            processTable[i].startSeconds = clock[0];
            processTable[i].startNano = clock[1];
            processTable[i].blocked = 0;
            processTable[i].queuePriority = 0;
            processTable[i].runtimeNano = burstNano;
        }
    }
}

void incrementClock(int* currentSec, int* currentNano, int incrNano) {
    *currentNano += incrNano;
    while (*currentNano > BILLION) {
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

int getNextTableIndexToSchedule(std::vector<float>* priorityVector) {
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

        priorityVector->push_back(processTimeRatio);

        if (processTimeRatio < minRatio) {
            minRatio = processTimeRatio;
            nextPIDIndex = i;
        } else if (processTimeRatio == minRatio) {
            // Ties are broken by whichever process has the lower process ID
            nextPIDIndex = processTable[i].pid < processTable[nextPIDIndex].pid
                               ? i
                               : nextPIDIndex;
        }
    }
    // Sort priority ratios from least to greatest
    if (priorityVector->size() > 0) {
        std::sort(priorityVector->begin(), priorityVector->end(),
                  [](float a, float b) { return a < b; });
    }

    return nextPIDIndex;
}

int checkBlockedProcesses(int* clock) {
    int sec = clock[0];
    int nano = clock[1];
    int totalBlockedTime = 0;
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].blocked)
            continue;

        if (sec >= processTable[i].eventWaitSec ||
            (sec == processTable[i].eventWaitSec &&
             nano >= processTable[i].eventWaitNano)) {

            totalBlockedTime += (sec * BILLION + nano) -
                                (processTable[i].whenBlockedSec * BILLION +
                                 processTable[i].whenBlockedNano);
            processTable[i].blocked = false;
            incrementClock(&(clock[0]), &(clock[1]), CHANGE_QUEUE_TIME_NS);
        }
    }
    return totalBlockedTime;
}

int getNumReadyProcesses() {
    int count = 0;
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].blocked && processTable[i].occupied)
            count++;
    }

    return count;
}

void printProcessTable(int currentSec, int currentNano) {
    printf("OSS PID:%d SysclockS:%d SysclockNano:%d\n", getpid(), currentSec,
           currentNano);
    printf("Process Table:\n");
    printf("Ready Queue:\n");
    printf("Entry Occupied PID StartS StartN\n");
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].blocked)
            continue;
        const char* occupiedStatus =
            processTable[i].occupied ? "True" : "False";
        printf("%d   %s       %d   %d    %d\n", i, occupiedStatus,
               processTable[i].pid, processTable[i].startSeconds,
               processTable[i].startNano);
    }
    printf("--------------\n");
    printf("Blocked Queue:\n");
    printf("Entry Occupied PID StartS StartN\n");
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].blocked)
            continue;
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

int getProcessIndexByPID(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].pid == pid)
            return i;
    }
    return -1;
}
void blockProcess(int pid, int curNano) {
    int processIndex = getProcessIndexByPID(pid);
    PCB_t* process = &(processTable[processIndex]);
    process->blocked = true;
    process->eventWaitNano = curNano + BLOCK_DELAY_NS;
    if (process->eventWaitNano > BILLION) {
        process->eventWaitNano -= BILLION;
        process->eventWaitSec += 1;
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
    int secLastChildLaunched = 0;
    int nanoLastChildLaunched = 0;
    int lastSec = *sec;
    int messagesSent = 0;
    int nextChildIndex = -1;
    int idleNano = 0;
    int idleSec = 0;
    int totalBlockedSec = 0;
    int totalBlockedNano = 0;
    std::vector<float> priorityVector;

    while (totalProcessesRan < options.numProcess || !isProcessTableEmpty()) {

        int timeLastChildLaunch =
            secLastChildLaunched * BILLION + nanoLastChildLaunched;
        int curTime = *sec * BILLION + *nano;
        if (numProcessesRunning < options.maxSimultaneous &&
            timeLastChildLaunch >
                curTime + options.childIntervalSec * BILLION) {
            // Bursts will be no smaller than 1 millisecond
            int burstMillis = rand() % int(options.childTimeLimitSec * 1000);
            // Convert milli to nano
            int burstNano = burstMillis * (BILLION / 1000);
            pid_t new_pid = fork();
            if (new_pid == 0) {
                printProcessTable(*sec, *nano);

                char processBurstStr[9];
                sprintf(processBurstStr, "%d", burstNano);

                char* args[] = {(char*)"./worker", processBurstStr, (char*)0};

                execvp(args[0], args);

                // Since the process gets replaced, this will only run if execvp
                // fails for some reason
                fprintf(stderr, "Error executing %s. Terminating.", args[0]);
                return EXIT_FAILURE;
            } else {
                lfprintf(log_file,
                         "OSS: Generating new process with PID %d and putting "
                         "it in ready queue at time %d:%d\n",
                         new_pid, *sec, *nano);
                printf("OSS: Generating new process with PID %d and putting "
                       "it in ready queue at time %d:%d\n",
                       new_pid, *sec, *nano);
                addTableEntry(new_pid, burstNano);
                secLastChildLaunched = *sec;
                nanoLastChildLaunched = *nano;
                totalProcessesRan += 1;
                numProcessesRunning += 1;
            }
        }

        totalBlockedNano += checkBlockedProcesses(clock);
        while (totalBlockedNano > BILLION) {
            totalBlockedNano -= BILLION;
            totalBlockedSec += 1;
        }
        if (getNumReadyProcesses() > 0) {
            nextChildIndex = getNextTableIndexToSchedule(&priorityVector);
            printf("Priority vector size: %ld", priorityVector.size());
            std::string queueString = "[";
            queueString.append(std::to_string(priorityVector[0]));
            if (priorityVector.size() > 1) {
                for (size_t i = 1; i < priorityVector.size(); i++) {
                    queueString.append(", " +
                                       std::to_string(priorityVector.at(i)));
                }
            }
            queueString.append("]");
            lfprintf(log_file, "OSS: Ready Queue Priorities: %s\n",
                     queueString.c_str());
            printf("OSS: Ready Queue Priorities: %s\n", queueString.c_str());
            incrementClock(sec, nano, SCHEDULE_DECISION_TIME_NS);
        } else {
            nextChildIndex = -1;
        }

        if (nextChildIndex >= 0) {

            pid_t nextChildPID = processTable[nextChildIndex].pid;

            buffers[nextChildIndex].mtype = nextChildPID;
            buffers[nextChildIndex].intData = SCHEDULE_QUANTUM_NS;
            buffers[nextChildIndex].sender = getpid();

            lfprintf(log_file,
                     "OSS: Dispatching process with PID %d priority %f from "
                     "ready queue at time %d:%d\n",
                     nextChildPID, priorityVector[0], *sec, *nano);
            printf("OSS: Dispatching process with PID %d priority %f from "
                   "ready queue at time %d:%d\n",
                   nextChildPID, priorityVector[0], *sec, *nano);

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

            int msg = rcvbuf.intData;
            lfprintf(log_file,
                     "OSS: Recieving that process with PID %d ran for %d "
                     "nanoseconds",
                     rcvbuf.sender, msg);
            printf("OSS: Recieving that process with PID %d ran for %d "
                   "nanoseconds",
                   rcvbuf.sender, msg);

            int rcvIndex = getProcessIndexByPID(rcvbuf.sender);
            if (rcvIndex == -1) {
                fprintf(stderr,
                        "could not find process with pid %d in process table\n",
                        rcvbuf.sender);
            }

            if (msg == SCHEDULE_QUANTUM_NS) {
                lfprintf(log_file, "OSS: Using its full quantum\n");
                printf("OSS: Using its full quantum\n");
                incrementClock(sec, nano, SCHEDULE_QUANTUM_NS);
                numProcessesRunning--;
            } else if (msg < 0) {
                lfprintf(log_file,
                         "OSS: Process %d was blocked and did not "
                         "use its full quantum\n",
                         rcvbuf.sender);
                printf("OSS: Process %d was blocked and did not "
                       "use its full quantum\n",
                       rcvbuf.sender);
                blockProcess(rcvbuf.sender, *nano);
                incrementClock(sec, nano, -1 * msg);
            } else if (msg > 0) {
                lfprintf(log_file,
                         "OSS: Process %d terminated and did not "
                         "use its full quantum\n",
                         rcvbuf.sender);
                printf("OSS: Process %d terminated and did not "
                       "use its full quantum\n",
                       rcvbuf.sender);
                incrementClock(sec, nano, msg);
                rmProcess(rcvbuf.sender);
                numProcessesRunning--;
            }
        } else { // No processes to be run/scheduled
            incrementClock(sec, nano, IDLE_TIME_NS);
        }

        long lastTime = lastSec * BILLION + lastNano;
        long now = *sec * BILLION + *nano;

        if (now - lastTime >= BILLION / 2) {
            lastSec = *sec;
            lastNano = *nano;
            printProcessTable(*sec, *nano);
        }
    }

    /* Clean Up Shared Memory */

    shmdt(clock);
    shmctl(shm_id, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
