#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "node.h"
#include "symbol.h"
#include <stdio.h>
#include <vector>

bool generateSymbolTable(node_t*);

// Processes the entire parse tree to generate symbols, returns false if any
// error occurs.
bool processTree(node_t*);

// Process the tokens of leaf node in the parse tree
bool processLeaf(node_t*);

// Attempts to add a symbol to the symbol table, returning true on success and
// false on error
bool addSymbol(Symbol, int);
// Attempts to fetch a symbol from the symbol table, returning true on success
// and false on error
bool fetchSymbol(Symbol, int);
// Utility function to see if a symbol is in the symbol table
bool checkSymbol(Symbol);

#endif
