#ifndef SCANNER_H
#define SCANNER_H

#include "stdio.h"
#include "string.h"
#include "token.h"
#include <vector>

/* DRIVER STATE DEFINITIONS */
const int T1_TOKEN = 1001;
const int T2_TOKEN = 1002;
const int T3_TOKEN = 1003;
const int EOF_TOKEN = 1004;

// Utility function to convert integers from the driver table into tokens
TokenType intToToken(int tk);

// Main Driver Function
Token FSADriver(FILE*);

// Filters input to ignore whitespace/comments, returns all tokens at the end
int filter(FILE*);

// Utility function to get a corresponding column of the driver table from a
// character
int getColumnNum(char);

#endif
