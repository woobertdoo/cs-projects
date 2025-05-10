#include "codeGenerator.h"
#include "symbolTable.h"
#include "token.h"
#include <string.h>
#include <string>

FILE* asmFile;

// Used to track and label which loop we're on
int loopIndex = 0;
int tempVarCount = 0;
std::vector<Symbol> symbolTable;

// A vector that holds all token instances, with identifiers and numbers being
// converted to ASM-friendly format
std::vector<std::string> convertedTokens;

// Current instruction index of the program
int instructionIndex = 0;

bool openFile(const char* fileName) {
    std::string name = fileName;
    name.append(".asm");
    asmFile = fopen(name.c_str(), "w");
    if (asmFile == NULL) {
        fprintf(stderr, "Error creating file %s.asm for writing\n.", fileName);
        return false;
    }
    return true;
}

std::string getT2VarName(std::string t2) {
    if (t2[0] != '+') {
        fatal(
            "Expected a plus sign as first character of t2 token identifier.");
    }

    t2[0] = 'P';
    return t2;
}

std::string getT3Val(std::string t3) {
    int value;

    if (t3[0] < 'A' || t3[0] > 'z') {
        fatal("Expected a letter for first character of t3 token identifier.");
    }
    if (t3[0] > 'Z' && t3[0] < 'a') {
        fatal("Expected a letter for first character of t3 token identifier.");
    }

    // set to 1 if uppercase (positive) or -1 if lowercase (negative)
    int caseVal = (t3[0] >= 'A' && t3[0] <= 'Z') ? 1 : -1;

    // Set letter to '0' so we can convert to int
    t3[0] = '0';
    value = atoi(t3.c_str());

    /* start OR1 */
    return std::to_string(value * caseVal);
    /* end OR1 */
}

bool convertInstructions(node_t* root) {
    for (int i = 0; i < root->children.size(); i++) {
        node_t* child = root->children[i];
        // Empty string in token instance means this is a non-terminal node;
        if (child->token.instance == "") {
            if (!convertInstructions(child))
                return false;
        }

        /* start OR2 */
        switch (child->token.type) {
        case TKN_T1:
            // non-terminal nodes are implicitly T1 type, so check if instance
            // is empty before pushing
            if (child->token.instance != "")
                convertedTokens.push_back(child->token.instance);
            break;
        case TKN_T2: {
            std::string t2Name = getT2VarName(child->token.instance);
            convertedTokens.push_back(t2Name);
            break;
        }
        case TKN_T3: {
            std::string t3Val = getT3Val(child->token.instance);
            convertedTokens.push_back(t3Val);
            break;
        }
        case TKN_ERR:
            fprintf(stderr,
                    "Error converting token '%s' at line %d. Exiting..\n.",
                    child->token.instance.c_str(), child->token.lineNum);
            return false;
        default:
            break;
        }
        /* end OR2 */
    }
    return true;
}

bool generateASM(node_t* root, const char* asmFilename) {
    if (!openFile(asmFilename)) {
        return false;
    }

    // Generate the static symbol table for variable declarations
    if (!generateSymbolTable(root)) {
        return false;
    }
    if (!convertInstructions(root)) {
        fatal("Error generating ASM file\nExiting...");
        return false;
    }

    symbolTable = fetchSymbolTable();

    checkOperation();

    fprintf(asmFile, "STOP\n");

    writeVarDecl();
    return true;
}

void checkOperation() {

    // Now, we check what operation we are doing based on the character
    if (convertedTokens[instructionIndex] == "\"") {
        // If we have a ", we are on an S non-terminal. Increment 3 instructions
        // [" t2 (], and rerun the block code
        instructionIndex += 3;
        writeBlock();
        return;
    } else if (convertedTokens[instructionIndex] == "(") {
        // S production without empty A
        instructionIndex++;
        writeBlock();
        return;
    } else if (convertedTokens[instructionIndex] == "#") {
        instructionIndex++;
        writeReadIn();
        return;
    } else if (convertedTokens[instructionIndex] == "!") {
        instructionIndex++;
        writeNegation();
        return;
    } else if (convertedTokens[instructionIndex] == "$") {
        instructionIndex++;
        writePrint();
        return;
    } else if (convertedTokens[instructionIndex] == "\'") {
        instructionIndex++;
        writeLoop();
        return;
    } else if (convertedTokens[instructionIndex][0] == 'P') {
        // Since the G production rule starts with a t2, we check for that
        // instead of the assignment operator
        writeAssignment();
    }
}

void writeBlock() {
    // Since blocks have 2 B non-terminals, we need to check operations twice
    checkOperation();
    // Go to next operation
    instructionIndex++;
    checkOperation();
    // We skip over the closing parenthesis in S
    instructionIndex++;
}

/*
 *  Write the statements for negating
 *
 *  Takes the value of `token` and multiplies by -1
 */
void writeNegation() {
    // If F is an add operation
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        writeAddition();
    } else { // It's a token of some kind
        fprintf(asmFile, "LOAD %s\n",
                convertedTokens[instructionIndex].c_str());
    }

    fprintf(asmFile, "MULT -1\n");

    // If it's a T2, so we need to update the value
    if (convertedTokens[instructionIndex][0] == 'P') {
        fprintf(asmFile, "STORE %s\n",
                convertedTokens[instructionIndex].c_str());
    }
}

/*
 * Write the statements for printing
 *
 * Prints the value of `token`
 */
void writePrint() {
    // If we need to do addition first, we perform that and store it in a
    // temporary variable to write
    if (convertedTokens[instructionIndex] == "&") {
        tempVarCount++;
        int tempVarNum = tempVarCount;
        instructionIndex++;
        writeAddition();
        fprintf(asmFile, "STORE T%d\n", tempVarNum);
        fprintf(asmFile, "WRITE T%d\n", tempVarNum);
        return;
    } else {
        fprintf(asmFile, "WRITE %s\n",
                convertedTokens[instructionIndex].c_str());
    }
}

// Write all variable declarations
void writeVarDecl() {
    // Write all static symbol declarations
    for (int i = 0; i < symbolTable.size(); i++) {
        std::string convertedName = getT2VarName(symbolTable[i].name);
        fprintf(asmFile, "%s 0\n", convertedName.c_str());
    }
    // Write temporary variable declarations, if any exist
    if (tempVarCount < 1)
        return;
    for (int i = 1; i <= tempVarCount; i++) {
        fprintf(asmFile, "T%d 0\n", i);
    }
}

/*
 * Write addition statements
 *
 * Adds `tk1` and `tk2`.
 *
 * Importantly, does not change their values
 */
void writeAddition() {
    // If the next symbol is another &, we need to do recursive addition
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        writeAddition();
    } else {
        // Load the first value in the deepest addition statement
        fprintf(asmFile, "LOAD %s\n",
                convertedTokens[instructionIndex].c_str());
    }
    // Increment to get the next value
    instructionIndex++;
    fprintf(asmFile, "ADD %s\n", convertedTokens[instructionIndex].c_str());
}

/*
 * Write read-initializer statements
 *
 * Reads in the value to initialize `iden` with
 */
void writeReadIn() {
    fprintf(asmFile, "READ %s\n", convertedTokens[instructionIndex].c_str());
}

/*
 * Write assignment statements
 *
 * Copies the value of `src` into `dest`.
 */

void writeAssignment() {
    // We start by moving the instruction index ahead two spots to get the value
    // to assign
    int t2Index = instructionIndex;
    instructionIndex += 2;
    // Check if it's an addition F rule
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        writeAddition();
    } else {
        // Otherwise load the necessary value into the accumulator
        fprintf(asmFile, "LOAD %s\n",
                convertedTokens[instructionIndex].c_str());
    }

    // Store it in the t2 that was at the beginning of the rule
    fprintf(asmFile, "STORE %s\n", convertedTokens[t2Index].c_str());
}

/*
 * Write loop statements
 *
 * If `testVal` is greater than `caseVal`, do `op` `nIter` times.
 *
 * Importantly, if `nIter` is less than 1, do nothing
 */
void writeLoop() {
    loopIndex++;
    int loopNum = loopIndex;
    std::string testVal;
    std::string caseVal;
    std::string nIter;

    // We need to store some temporary variables if any of the F's in the loop
    // production are addition

    // Test Value
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        tempVarCount++;
        int tempVarNum = tempVarCount;
        writeAddition();
        fprintf(asmFile, "STORE T%d\n", tempVarNum);
        testVal = "T";
        testVal.append(std::to_string(tempVarNum));
    } else {
        testVal = convertedTokens[instructionIndex];
    }

    instructionIndex++;

    // Case Value
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        tempVarCount++;
        int tempVarNum = tempVarCount;
        writeAddition();
        fprintf(asmFile, "STORE T%d\n", tempVarNum);
        caseVal = "T";
        caseVal.append(std::to_string(tempVarNum));
    } else {
        caseVal = convertedTokens[instructionIndex];
    }

    instructionIndex++;

    // Number of iterations, always store it in a temp variable
    if (convertedTokens[instructionIndex] == "&") {
        instructionIndex++;
        writeAddition();
    } else {
        fprintf(asmFile, "LOAD %s\n",
                convertedTokens[instructionIndex].c_str());
    }
    tempVarCount++;
    int tempVarNum = tempVarCount;
    fprintf(asmFile, "STORE T%d\n", tempVarNum);
    nIter = "T";
    nIter.append(std::to_string(tempVarNum));

    // This gives the first operation for the loop itself
    instructionIndex++;

    // Write the loop condition test
    fprintf(asmFile, "LOAD %s\n", testVal.c_str());
    fprintf(asmFile, "SUB %s\n", caseVal.c_str());
    fprintf(asmFile, "BRZNEG EXITLOOP%d\n", loopNum);

    // Write the start of the loop
    fprintf(asmFile, "LOOP%d: LOAD %s\n", loopNum, nIter.c_str());
    // Write break condition test
    fprintf(asmFile, "BRZNEG EXITLOOP%d\n", loopNum);
    // Perform whatever operations are supplied
    checkOperation();

    // Update the loop counter
    fprintf(asmFile, "LOAD %s\n", nIter.c_str());
    fprintf(asmFile, "SUB 1\n");
    fprintf(asmFile, "STORE %s\n", nIter.c_str());

    // Go back to the beginning of the loop
    fprintf(asmFile, "BR LOOP%d\n", loopNum);

    // Write the exit label
    fprintf(asmFile, "EXITLOOP%d: NOOP\n", loopNum);
}
