#include "parser.h"
#include "testTree.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    FILE* inFile;
    std::string inFileName;
    node_t* root = nullptr;

    if (argc == 1) {
        inFile = stdin;
        inFileName = "out";
    } else if (argc == 2) {
        inFileName = argv[1];
        inFile = fopen(argv[1], "r");
        if (inFile == NULL) {
            fprintf(stderr, "Error loading file %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Fatal: Improper usage\n");
        fprintf(stderr, "Usage: P0 [filename]\n");
        return EXIT_FAILURE;
    }

    root = parser(inFile);
    if (root == nullptr)
        return EXIT_FAILURE;
    printParseTree(root);
    return EXIT_SUCCESS;
}
