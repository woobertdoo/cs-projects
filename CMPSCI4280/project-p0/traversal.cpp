#include "traversal.h"
#include <queue>
#include <stdio.h>
#include <string.h>
#include <vector>

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
        /* start OR7 */
        std::queue<node_t*> q;
        std::vector<std::vector<node_t*>> res;

        // Enqueue Root
        q.push(root);
        int currLevel = 0;

        while (!q.empty()) {
            int len = q.size();
            res.push_back({});

            for (int i = 0; i < len; i++) {

                // Add front of queue and remove it from queue
                node_t* node = q.front();
                q.pop();

                res[currLevel].push_back(node);

                // Enqueue left child
                if (node->leftChild != nullptr)
                    q.push(node->leftChild);

                // Enqueue right child
                if (node->rightChild != nullptr)
                    q.push(node->rightChild);
            }
            currLevel++;
        }

        for (int i = 0; i < res.size(); i++) {
            for (int j = 0; j < res[i].size(); j++) {
                std::string nodeKeys;

                for (int tkn = 0; tkn < res[i][j]->keys.size(); tkn++) {
                    nodeKeys.append(res[i][j]->keys[tkn].c_str());
                    nodeKeys.push_back(' ');
                }

                // Appending null terminator to make sure the string is
                // terminated properly
                nodeKeys.push_back('\0');
                fprintf(outFile, "%*c%d %d: %s\n", i * 4, ' ', i,
                        res[i][j]->value, nodeKeys.c_str());
            }
        }
        /* end OR7 */
    } else {
        fprintf(stderr, "Invalid traversal mode %s\n", mode);
        return;
    }
    /* end OR5 */
}
