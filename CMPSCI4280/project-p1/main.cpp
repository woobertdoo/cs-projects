#include "testScanner.h"
#include <cstdlib>
#include <stdio.h>
#include <string>

int main(int argc, char** argv) {
    std::string inFileName;

    if (argc == 1) {
        inFileName = "stdin";
    } else if (argc == 2) {
        inFileName = argv[1];
    } else {
        fprintf(stderr, "Fatal: Improper usage\n");
        fprintf(stderr, "Usage: P0 [filename]\n");
        return EXIT_FAILURE;
    }

    testScanner(inFileName.c_str());

    return EXIT_SUCCESS;
}
