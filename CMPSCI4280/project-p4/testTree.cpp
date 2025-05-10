#include "testTree.h"
#include <stdio.h>

void printParseTree(node_t* root) {

    printf("%*c%s\n", root->level * 4, ' ', root->label);

    if (root->children.size() == 0)
        return;

    for (auto child : root->children) {
        printParseTree(child);
    }
}
