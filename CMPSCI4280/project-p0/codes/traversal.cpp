#include "traversal.h"
#include <stdio.h>
#include <string.h>

// Define traversal mode options
const char* PRE = "pre";
const char* POST = "post";
const char* LEV = "lev";

int traversePreOrder(node_t* root, const char* outFileName) {
    FILE* outFile = fopen(outFileName, "w");
    // fprintf(outFile, "");
    if (outFile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outFileName);
        return 1;
    }

    printParseTree(root, 0, PRE, outFile);

    return 0;
}

int traversePostOrder(node_t* root, const char* outFileName) {

    FILE* outFile = fopen(outFileName, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outFileName);
        return 1;
    }

    printParseTree(root, 0, POST, outFile);

    return 0;
}

int traverseLevelOrder(node_t* root, const char* outFileName) {
    FILE* outFile = fopen(outFileName, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outFileName);
        return 1;
    }

    printParseTree(root, 0, LEV, outFile);

    return 0;
}

void printParseTree(node_t* root, int level, const char* mode, FILE* outFile) {
    if (root == nullptr)
        return;

    std::string nodeKeys;

    // Creating a single string that holds all the node's keys delimited by
    // spaces
    for (int i = 0; i < root->keys.size(); i++) {
        nodeKeys.append(root->keys.at(i).c_str());
        nodeKeys.push_back(' ');
    }

    // Appending null terminator to make sure the string is terminated properly
    nodeKeys.push_back('\0');

    /* start OR5 */
    if (strcmp(mode, PRE) == 0) {
        fprintf(outFile, "%*c%d %d: %s\n", level * 4, ' ', level, root->value,
                nodeKeys.c_str());
        printParseTree(root->leftChild, level + 1, mode, outFile);
        printParseTree(root->rightChild, level + 1, mode, outFile);
    } else if (strcmp(mode, POST) == 0) {
        printParseTree(root->leftChild, level + 1, mode, outFile);
        printParseTree(root->rightChild, level + 1, mode, outFile);
        fprintf(outFile, "%*c%d %d: %s\n", level * 4, ' ', level, root->value,
                nodeKeys.c_str());
    } else if (strcmp(mode, LEV) == 0) {
        printParseTree(root->leftChild, level + 1, mode, outFile);
        fprintf(outFile, "%*c%d %d: %s\n", level * 4, ' ', level, root->value,
                nodeKeys.c_str());
        printParseTree(root->rightChild, level + 1, mode, outFile);
    } else {
        fprintf(stderr, "Invalid traversal mode %s\n", mode);
        return;
    }
    /* end OR5 */
}
