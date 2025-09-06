#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h> // Access to Unix system calls

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
    fprintf(stderr, "If a value is not given for any of -n,-s, or -t, the "
                    "program will immediately exit.\n");
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

    pid_t childPid = fork(); // Create child process to get replaced with ./user
    if (childPid == 0) {
        printf("I'm a copy of the parent. My process id is %d.\nMy parent's "
               "process id is %d.\n",
               getpid(), getppid());
        char* args[] = {(char*)"./user", (char*)0};
        execvp(args[0], args);

        // Since the process gets replaced, this will only run if execvp fails
        // for some reason
        fprintf(stderr, "Error executing %s. Terminating.", args[0]);
        return EXIT_FAILURE;
    } else {
        printf("I'm the parent! My process id is %d.\n", getpid());
    }
    return EXIT_SUCCESS;
}
