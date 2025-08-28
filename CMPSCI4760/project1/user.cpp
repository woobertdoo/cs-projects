#include <stdio.h>  // Access to C standard I/O functions like printf
#include <stdlib.h> // Access to EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h> // Access to Unix system calls

int main(int argc, char** argv) {
    printf("User entered with process id %d\n", getpid());
    return EXIT_SUCCESS;
}
