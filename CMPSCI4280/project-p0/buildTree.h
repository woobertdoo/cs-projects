#ifndef BUILDTREE_H
#define BUILDTREE_H

#include "node.h"
#include <fstream>

static const int MAX_TOKEN_LENGTH = 100;

node_t* buildTree(FILE*);

// Utility function to check that each character of a token is valid
bool isValidChar(const char c);

#endif
