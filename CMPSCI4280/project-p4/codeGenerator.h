#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "symbolTable.h"
#include <stdio.h>

enum OPERATIONS { Addition, Negation, ReadIn, Loop, Print, Assignment };

inline void fatal(const char* string) {
    fprintf(stderr, "\nFatal: %s\n\n", string);
}

bool openFile(const char*);

/* Utility functions for converting token instances into ASM-friendly strings */
std::string getT2VarName(std::string);
std::string getT3Val(std::string);

// Runs through the parse tree to generate ASM code
bool generateASM(node_t* root, const char* asmFilename);

// Converts all tokens in the parse tree to ASM-friendly format
// Returns false if there is an error converting a token
bool convertInstructions(node_t* root);

// This will be used to write anything inside the parentheses of an "S"
// non-terminal
void writeBlock();

// Used to see what operation we need to do and execute, for B non-terminals
void checkOperation();

/*
 *  Write the statements for negating
 *
 *  Takes the value of `token` and multiplies by -1
 */
void writeNegation();

/*
 * Write the statements for printing
 *
 * Prints the value of `token`
 */
void writePrint();

// Write all variable declarations
void writeVarDecl();

// Used to handle delimited operations to make sure they get stored in a
// temporary variable
void handleDelimiters();

/*
 * Write addition statements
 *
 * Adds `tk1` and `tk2`.
 *
 * Importantly, does not change their values
 */
void writeAddition();

/*
 * Write read-initializer statements
 *
 * Reads in the value to initialize `iden` with
 */
void writeReadIn();

/*
 * Write assignment statements
 *
 * Copies the value of `src` into `dest`.
 */

void writeAssignment();

/*
 * Write loop statements
 *
 * If `testVal` is greater than `caseVal`, do `op` `nIter` times.
 *
 * Importantly, if `nIter` is less than 1, do nothing
 */
void writeLoop();
#endif
