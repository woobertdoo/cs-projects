#include "scanner.h"
#include "token.h"
#include <cstdio>

// --------------------WS sc az  09  eof  +
int fsaTable[6][6] = {{0, 5, 1, -1, 1004, 3},
                      {-2, -2, -2, 2, -2, -2},
                      {1003, 1003, 1003, 2, 1003, 1003},
                      {-3, -3, -3, 4, -3, -3},
                      {1002, 1002, 1002, 4, 1002, 1002},
                      {1001, 1001, 1001, 1001, 1001, 1001}};

char nextChar = ' ';
char prevChar = 0;
int lineNum = 1;
int newLineCount = 0;
// Track previous state when moving between tokens
int prevState = 0;

/* start OR1 */
bool newToken = false;
char newTokenChar;
/* end OR1 */

int filter(FILE* inFile) {
    bool insideComment = false;

    /* start OR1 */
    if (newToken) {
        newToken = false;
        nextChar = newTokenChar;
        return getColumnNum(nextChar);
    }
    /* end OR1 */

    while ((nextChar = getc(inFile))) {

        if (insideComment) {
            if (nextChar == '\n')
                lineNum++;
            if (nextChar == '*')
                insideComment = false;
            prevChar = nextChar;
            continue;
        }
        if (nextChar == '*') {
            insideComment = true;
            prevChar = nextChar;
            continue;
        } else if (nextChar == '\n') {
            // let lineNum increment for consecutive newlines, as well as
            // newlines at the end of comments
            if (prevChar == '\n' || (prevChar == '*' && !insideComment)) {
                lineNum++;
            }
            prevChar = nextChar;
            return 0;
        } else if (nextChar == ' ') {
            prevChar = nextChar;
            return 0;
        }
        return getColumnNum(nextChar);
    }
    /* end OR1 */
    return (int)TKN_EOF;
}

Token FSADriver(FILE* inFile) {
    int column = filter(inFile);
    int state = 0;
    std::string S = "";

    int nextState;
    Token tk;

    while (state >= 0 && state <= 10) {
        if (column == -1) {
            tk = Token{TKN_ERR, S, lineNum};
            break;
        }

        nextState = fsaTable[state][column];

        if (nextState < 0) {
            tk = Token{TKN_ERR, S, lineNum};
            break;
        } else if (nextState > 1000) {

            // Increment lineNum after adding token if new line
            if (nextChar == '\n')
                tk = Token{intToToken(nextState), S, lineNum++};
            else
                tk = Token{intToToken(nextState), S, lineNum};
            /* start OR1 */
            if (nextChar != ' ' && nextChar != '\n') {
                newToken = true;
                newTokenChar = nextChar;
            }
            /* end OR1 */
            break;
        } else {
            // Don't add ending new line character
            if (nextChar != ' ' && nextChar != '\n')
                S.push_back(nextChar);
        }
        state = nextState;
        column = filter(inFile);
    }
    return tk;
}
int getColumnNum(char c) {
    if (c >= '!' && c <= ')')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 2;
    if (c >= 'a' && c <= 'z')
        return 2;
    if (c >= '0' && c <= '9')
        return 3;
    if (c == EOF)
        return 4;
    if (c == '+')
        return 5;
    return -1;
}

TokenType intToToken(int tk) {
    switch (tk) {
    case EOF_TOKEN:
        return TokenType::TKN_EOF;
    case T1_TOKEN:
        return TokenType::TKN_T1;
    case T2_TOKEN:
        return TokenType::TKN_T2;
    case T3_TOKEN:
        return TokenType::TKN_T3;
    default:
        fprintf(stderr, "Error: Unknown Token Identifier: %d", tk);
        return (TokenType)NULL;
    }
}
