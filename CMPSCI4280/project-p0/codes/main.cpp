#include "buildTree.h"
#include "traversal.h"
#include <cstdlib>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    FILE* inFile = nullptr;
    std::string inFileName;
    node_t* root = nullptr;

    if (argc == 1) {
        printf("reading std\n");
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

    if (inFile == nullptr) {
        root = buildTree(stdin);
    } else {
        root = buildTree(inFile);
    }
    if (root == nullptr) {
        fprintf(stderr, "Error: No data found in file\n");
        return EXIT_FAILURE;
    }

    /* start OR4 */
    // printf("Root Node\nValue: %i, First Key: %s", root->value,
    //        root->keys.at(0).c_str());
    /* end OR4 */

    std::string outFileName = inFileName;
    outFileName.append(".preorder");
    traversePreOrder(root, outFileName.c_str());
    outFileName = inFileName;
    outFileName.append(".postorder");
    traversePostOrder(root, outFileName.c_str());
    outFileName = inFileName;
    outFileName.append(".levelorder");
    traverseLevelOrder(root, outFileName.c_str());
    return EXIT_SUCCESS;
}
