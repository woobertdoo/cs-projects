#include "testScanner.h"
#include "scanner.h"
#include <stdio.h>

void testScanner(const char* fileName) {
    FILE* inFile;
    std::string tokenNames[] = {"t1 token", "t2 token", "t3 token"};
    if (strcmp(fileName, "stdin") == 0) {
        inFile = stdin;
    } else {
        inFile = fopen(fileName, "r");
    }
    if (inFile == nullptr) {
        fprintf(stderr, "Error reading file %s\n", fileName);
    }

    Token tk = FSADriver(inFile);

    while (tk.type != TKN_EOF) {
        if (tk.type == TKN_ERR) {
            printf("SCANNER ERROR: %s at line %d", tk.instance->c_str(),
                   tk.lineNum);
            return;
        }

        printf("%s\t\t%s\t\t%d\n", tokenNames[(int)tk.type].c_str(),
               tk.instance->c_str(), tk.lineNum);
        tk = FSADriver(inFile);
    }

    printf("end of file token\n");
}
