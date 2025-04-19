#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "scanner.h"

node_t* parser(FILE* inFile);
node_t* S(int);
node_t* A(int);
node_t* B(int);
node_t* C(int);
node_t* D(int);
node_t* E(int);
node_t* F(int);
node_t* G(int);
void printError();
bool checkChar(char*);

#endif
