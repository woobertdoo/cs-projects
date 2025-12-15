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
const int NANO_INCR = 100000;
const int BILLION = 1000000000;
const int DISK_RW_TIME = 14 * (BILLION / 1000);
const int SYS_MAX_SIMUL_PROCS = 18;
const int FRAME_TABLE_SIZE = 64;
const int PAGE_SIZE = 1024; // In Bytes
const int PROCESS_PAGES = 16;
const int PAGE_NONFAULT_INCR = 100; // Increment when there is no page fault
int shm_id;
int* ossclock;
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
    int address;
    int dirty;  // dirty bit, 0 for read 1 for write
    int status; // 1 when worker terminates
} msgbuffer;

typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;
    int startNano;
    //  Value of each page index is the frame it is stored in,
    //  or -1 if it has not been loaded
    int pages[PROCESS_PAGES];
    long long blockedNano;
    // If process is waiting for a page to be loaded into frame
    bool deviceQueued;
} PCB_t;

typedef struct {
    bool dirtyBit;
    pid_t process;
    int page;
    bool occupied;
    int frameNum;
} frame_t;

// Global Variables
options_t options;
PCB_t* processTable;
frame_t frameTable[FRAME_TABLE_SIZE];
std::vector<msgbuffer> outstandingRequests;
std::vector<frame_t> frameQueue;

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
        processTable[i].deviceQueued = false;
        processTable[i].blockedNano = 0;
        for (int j = 0; j < PROCESS_PAGES; j++) {
            processTable[i].pages[j] = -1;
        }
    }
}

void initiateFrameTable() {
    for (int i = 0; i < FRAME_TABLE_SIZE; i++) {
        frameTable[i].dirtyBit = 0;
        frameTable[i].occupied = false;
        frameTable[i].process = -1;
        frameTable[i].page = -1;
        frameTable[i].frameNum = i;
    }
}

void addTableEntry(int pid) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (!processTable[i].occupied) {
            processTable[i].occupied = true;
            processTable[i].pid = pid;
            processTable[i].startSeconds = ossclock[0];
            processTable[i].startNano = ossclock[1];
            processTable[i].deviceQueued = false;
            processTable[i].blockedNano = 0;
            for (int j = 0; j < PROCESS_PAGES; j++) {
                processTable[i].pages[j] = -1;
            }
            return;
        }
    }
}

void removeFrame(int frameNum) {
    frameTable[frameNum].occupied = false;
    frameTable[frameNum].process = -1;
    frameTable[frameNum].dirtyBit = 0;
    frameTable[frameNum].page = -1;
}

// Returns the frame number popped
int popFrameQueue() {
    if (frameQueue.size() == 0) {
        printf("Could not pop frame queue, it's empty!\n");
        return -1;
    }
    frame_t poppedFrame = frameQueue.at(0);
    int frameNum = poppedFrame.frameNum;
    removeFrame(frameNum);
    frameQueue.erase(frameQueue.begin());

    return frameNum;
}

int getAddressPage(int address) { return address / PAGE_SIZE; }

void incrementossclock(int* currentSec, int* currentNano, int incrNano) {
    *currentNano += incrNano;
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
// Returns the frame number the page is assigned to
int addPageToFrameTable(int pageNum, pid_t process) {
    for (int i = 0; i < FRAME_TABLE_SIZE; i++) {
        frame_t frame;
        if (frameTable[i].occupied == false) {
            frame.process = process;
            frame.page = pageNum;
            frame.occupied = true;
            frame.dirtyBit = 0;
            frame.frameNum = i;
            frameQueue.push_back(frame);
            frameTable[i] = frame;
            int processIndex = getProcessIndex(process);
            processTable[processIndex].pages[pageNum] = i;
            return i;
        }
    }

    // If we still havent found a frame to put the page in
    // We need to swap a frame out
    int emptyFrameNum = popFrameQueue();
    frame_t frame;
    frame.process = process;
    frame.page = pageNum;
    frame.occupied = true;
    frame.dirtyBit = 0;
    frame.frameNum = emptyFrameNum;
    frameQueue.push_back(frame);
    frameTable[emptyFrameNum] = frame;
    int processIndex = getProcessIndex(process);
    processTable[processIndex].pages[pageNum] = emptyFrameNum;

    return emptyFrameNum;
}

bool allProcessesDeviceQueued() {
    if (isProcessTableEmpty())
        return false;
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].occupied && !processTable[i].deviceQueued) {
            return false;
        }
    }
    return true;
}

void freeAndExit(int sig) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        if (processTable[i].occupied) {
            kill(processTable[i].pid, SIGTERM);
        }
    }

    // Free shared memory
    shmdt(ossclock);
    shmctl(shm_id, IPC_RMID, NULL);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory/n");
        exit(1);
    }

    exit(1);
}

void printProcessTable(int currentSec, int currentNano) {
    printf("OSS PID:%d SysossclockS:%d SysossclockNano:%d\n", getpid(),
           currentSec, currentNano);
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

void printFrameTable(FILE* logFile) {
    int columnWidth = 10;
    lfprintf(logFile, "%-*s | %-*s | %-*s | %-*s | %-*s\n", columnWidth,
             "Frame", columnWidth, "Occupied", columnWidth, "DirtyBit",
             columnWidth, "Process", columnWidth, "Page");
    printf("%-*s | %-*s | %-*s | %-*s | %-*s\n", columnWidth, "Frame",
           columnWidth, "Occupied", columnWidth, "DirtyBit", columnWidth,
           "Process", columnWidth, "Page");
    for (int i = 0; i < FRAME_TABLE_SIZE; i++) {
        frame_t frame = frameTable[i];
        const char* occupiedStatus = frame.occupied ? "Yes" : "No";
        lfprintf(logFile, "%-*d | %-*s | %-*d | %-*d | %-*d\n", columnWidth, i,
                 columnWidth, occupiedStatus, columnWidth, frame.dirtyBit,
                 columnWidth, frame.process, columnWidth, frame.page);
        printf("%-*d | %-*s | %-*d | %-*d | %-*d\n", columnWidth, i,
               columnWidth, occupiedStatus, columnWidth, frame.dirtyBit,
               columnWidth, frame.process, columnWidth, frame.page);
    }
}

void printProcessPages(FILE* logFile) {
    for (int i = 0; i < options.maxSimultaneous; i++) {
        PCB_t process = processTable[i];
        std::string pageArray = "[";
        for (int j = 0; j < PROCESS_PAGES - 1; j++) {
            pageArray.append(std::to_string(process.pages[j]));
            pageArray.append(", ");
        }
        pageArray.append(std::to_string(process.pages[9]));
        pageArray.append("]");

        lfprintf(logFile, "P%d: %s\n", process.pid, pageArray.c_str());
        printf("P%d: %s\n", process.pid, pageArray.c_str());
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
            processTable[i].deviceQueued = false;
            processTable[i].blockedNano = 0;
            for (int j = 0; j < PROCESS_PAGES; j++) {
                processTable[i].pages[j] = -1;
            }
        }
    }
}

// Returns the buffer that was fulfilled
msgbuffer fulfillHeadRequest(FILE* logFile) {
    msgbuffer headReq = outstandingRequests.at(0);
    if (outstandingRequests.size() > 0) {
        outstandingRequests.erase(outstandingRequests.begin());
    }
    int pageNum = getAddressPage(headReq.address);

    int frameNum = addPageToFrameTable(pageNum, headReq.sender);
    frameTable[frameNum].dirtyBit = headReq.dirty;

    printf("OSS: Page %d for Process %d put into frame %d\n", pageNum,
           headReq.sender, frameNum);

    return headReq;
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
    //    signal(SIGALRM, freeAndExit);
    signal(SIGINT, freeAndExit);
    //   alarm(5);

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

    ossclock = (int*)shmat(shm_id, 0, 0);
    if (ossclock == nullptr) {
        fprintf(stderr, "Parent: Error accessing shared memory\n");
        return EXIT_FAILURE;
    }

    int* sec = &(ossclock[0]);
    int* nano = &(ossclock[1]);
    *sec = *nano = 0;

    processTable = new PCB_t[options.maxSimultaneous];
    initiateProcessTable();
    int totalProcessesRan = 0;
    int numProcessesRunning = 0;

    int lastNano = *nano;
    int lastSec = *sec;
    int pageFaultCount;

    while (totalProcessesRan < options.numProcess || !isProcessTableEmpty()) {

        incrementossclock(sec, nano, NANO_INCR);

        msgbuffer rcvbuf;

        if (outstandingRequests.size() > 0 && allProcessesDeviceQueued()) {
            pid_t headProcess = outstandingRequests.at(0).sender;
            long long curNano = *sec * BILLION + *nano;
            long long processUnblockNano =
                processTable[getProcessIndex(headProcess)].blockedNano +
                DISK_RW_TIME;
            // Need to increment the clock to the time that the head request
            // would have been processed
            if (processUnblockNano > curNano)
                incrementossclock(sec, nano, processUnblockNano - curNano);
            msgbuffer headReq = fulfillHeadRequest(log_file);

            msgbuffer buf = headReq;
            buf.sender = getpid();
            buf.mtype = headReq.sender;

            if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                -1) {
                perror("Failed to send message to parent\n");
                return EXIT_FAILURE;
            }
        } else if (outstandingRequests.size() > 0) {
            pid_t headProcess = outstandingRequests.at(0).sender;
            long long curTime = *sec * BILLION + *nano;
            if (processTable[getProcessIndex(headProcess)].blockedNano +
                    DISK_RW_TIME >=
                curTime) {
                msgbuffer headReq = fulfillHeadRequest(log_file);
                msgbuffer buf = headReq;
                buf.sender = getpid();
                buf.mtype = headReq.sender;

                if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                    -1) {
                    perror("Failed to send message to parent\n");
                    return EXIT_FAILURE;
                }
            }
        }

        if (msgrcv(msqid, &rcvbuf, sizeof(msgbuffer), getpid(), IPC_NOWAIT) ==
            -1) {
            if (errno != ENOMSG) {
                fprintf(stderr, "failed to receive message in parent\n");
                return EXIT_FAILURE;
            }
        } else if (rcvbuf.status == 1) {
            printf("OSS: Detected process %d terminating\n", rcvbuf.sender);
            totalProcessesRan++;
            rmProcess(rcvbuf.sender);
            numProcessesRunning--;
        } else {
            int reqAddress = rcvbuf.address;
            int page = getAddressPage(reqAddress);
            int processIndex = getProcessIndex(rcvbuf.sender);

            if (processTable[processIndex].pages[page] == -1) {
                pageFaultCount++;
                processTable[processIndex].deviceQueued = true;
                processTable[processIndex].blockedNano = *sec * BILLION + *nano;
                outstandingRequests.push_back(rcvbuf);
                lfprintf(log_file, "Address %d is not in a frame, pagefault\n",
                         rcvbuf.address);
                printf("Address %d is not in a frame, pagefault\n",
                       rcvbuf.address);
            } else {
                int frameNum = processTable[processIndex].pages[page];
                frameTable[frameNum].dirtyBit = rcvbuf.dirty;
                msgbuffer buf;
                buf.mtype = rcvbuf.sender;
                buf.sender = getpid();
                buf.dirty = rcvbuf.dirty;
                buf.address = rcvbuf.address;
                buf.status = 0;

                if (rcvbuf.dirty == 0) {
                    printf("OSS: Address %d in frame %d, giving data to "
                           "process %d at time %d:%d\n",
                           reqAddress, frameNum, rcvbuf.sender, *sec, *nano);
                } else if (rcvbuf.dirty == 1) {
                    printf("OSS: Address %d in frame %d, writing data to frame "
                           "at time %d:%d\n",
                           reqAddress, frameNum, *sec, *nano);
                }

                if (msgsnd(msqid, &buf, sizeof(msgbuffer) - sizeof(long), 0) ==
                    -1) {
                    perror("Failed to send message to parent\n");
                    return EXIT_FAILURE;
                }
                incrementossclock(sec, nano, PAGE_NONFAULT_INCR);
            }
        }

        long lastTime = lastSec * BILLION + lastNano;
        long now = *sec * BILLION + *nano;

        if (now - lastTime >= BILLION / 2) {
            lastSec = *sec;
            lastNano = *nano;
            printFrameTable(log_file);
            printProcessPages(log_file);
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

    long long totalNano =
        totalProcessesRan * options.childLifetimeSec * BILLION;
    int finalNano = totalNano % BILLION;
    int totalSecs = (totalNano - finalNano) / BILLION;
    printf("OSS PID: %d Terminating\n", getpid());

    printf("-------------\n");

    /* Clean Up Shared Memory */

    shmdt(ossclock);
    shmctl(shm_id, IPC_RMID, NULL);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to remove shared memory/n");
        exit(1);
    }

    return EXIT_SUCCESS;
}
