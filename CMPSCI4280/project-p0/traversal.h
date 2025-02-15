#ifndef TRAVERSAL_H
#define TRAVERSAL_H

#include "node.h"

// Traversal functions return 0 if successful and 1 if error
int traverseLevelOrder(node_t* root, const char* outFileName);
int traversePreOrder(node_t* root, const char* outFileName);
int traversePostOrder(node_t* root, const char* outFileName);

/* Utility function to write the parse tree to a file
 * @param root: a pointer to the root node of the tree
 * @param level: the current level of the tree being printed
 * @param mode: what mode (level order, pre-order, or post-order) to print the
 * tree in valid values: "lev" (level order), "pre" (pre-order), "post" (post
 * order)
 * @param outFile: the output file to write to
 * */
void printParseTree(node_t* root, int level, const char* mode, FILE* outFile);

#endif
