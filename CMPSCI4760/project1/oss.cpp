#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h> // Access to Unix system calls

int main(int argc, char** argv) {
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
