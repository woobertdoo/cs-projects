#include "parser.h"
#include "scanner.h"
#include "token.h"
#include <string.h>

FILE* parseFile;
Token tk;

node_t* parser(FILE* inFile) {
    parseFile = inFile;
    tk = FSADriver(parseFile);
    node_t* root = S(0);
    if (tk.type == TokenType::TKN_EOF) {
        printf("Parse Successful!\n");
        return root;
    } else {
        printError();
        return nullptr;
    }
}

/*
 * FIRST(G) = t2
 * FIRST(F) = t2 | t3 | &
 * FIRST(E) = `
 * FIRST(D) = $
 * FIRST(C) = # | !
 * FIRST(A) = " | 0
 * FIRST(S) = "0(
 * FIRST(B) = "0( | #! | $ | ` | t2
 */

node_t* S(int level) {
    node_t* rootS = new node_t;
    rootS->level = level;
    rootS->label = (char*)"S";
    node_t* nodeA = A(level + 1);
    if (nodeA == nullptr)
        return nullptr;
    addChild(&rootS, nodeA, "A");
    if (!checkChar((char*)"(")) {
        printError();
        return nullptr;
    }
    addToken(&rootS, tk);
    tk = FSADriver(parseFile);
    node_t* nodeB = B(level + 1);
    if (nodeB == nullptr)
        return nullptr;
    addChild(&rootS, nodeB, "B");
    node_t* nodeB2 = B(level + 1);
    if (nodeB2 == nullptr)
        return nullptr;
    addChild(&rootS, nodeB2, "B");
    if (!checkChar((char*)")")) {
        printError();
        return nullptr;
    }
    addToken(&rootS, tk);
    tk = FSADriver(parseFile);
    return rootS;
}

node_t* A(int level) {
    node_t* rootA = new node_t;
    rootA->level = level;
    if (checkChar((char*)"\"")) {
        addToken(&rootA, tk);
        tk = FSADriver(parseFile);
        if (tk.type == TKN_T2) {
            addToken(&rootA, tk);
            tk = FSADriver(parseFile);
            return rootA;
        }
        printError();
        return nullptr;
    }
    node_t* emptyNode = new node_t;
    emptyNode->level = level + 1;
    addChild(&rootA, emptyNode, "empty");
    return rootA;
}

node_t* B(int level) {
    node_t* rootB = new node_t;
    rootB->level = level;
    if (tk.type == TKN_T2) {
        node_t* nodeG = G(level + 1);
        if (nodeG == nullptr)
            return nullptr;
        addChild(&rootB, nodeG, "G");
        return rootB;
    } else if (tk.type == TKN_T1) {
        if (checkChar((char*)"'")) {
            node_t* nodeE = E(level + 1);
            if (nodeE == nullptr)
                return nullptr;
            addChild(&rootB, nodeE, "E");
            return rootB;
        }
        if (checkChar((char*)"#") || checkChar((char*)"!")) {
            node_t* nodeC = C(level + 1);
            if (nodeC == nullptr)
                return nullptr;
            addChild(&rootB, nodeC, "C");
            return rootB;
        }
        if (checkChar((char*)"$")) {
            node_t* nodeD = D(level + 1);
            if (nodeD == nullptr)
                return nullptr;
            addChild(&rootB, nodeD, "D");
            return rootB;
        }
        if (checkChar((char*)"\"") || checkChar((char*)"(")) {
            node_t* nodeS = S(level + 1);
            if (nodeS == nullptr)
                return nullptr;
            addChild(&rootB, nodeS, "S");
            return rootB;
        }
        printError();
        return nullptr;
    } else {
        printError();
        return nullptr;
    }
}

node_t* C(int level) {
    node_t* rootC = new node_t;
    rootC->level = level;
    if (checkChar((char*)"#")) {
        addToken(&rootC, tk);
        tk = FSADriver(parseFile);
        if (tk.type == TKN_T2) {
            addToken(&rootC, tk);
            tk = FSADriver(parseFile);
            return rootC;
        } else {
            printError();
            return nullptr;
        }
    } else if (checkChar((char*)"!")) {
        addToken(&rootC, tk);
        tk = FSADriver(parseFile);
        node_t* nodeF = F(level + 1);
        if (nodeF == nullptr)
            return nullptr;
        addChild(&rootC, nodeF, "F");
        return rootC;
    } else {
        printError();
        return nullptr;
    }
}

node_t* D(int level) {
    node_t* rootD = new node_t;
    rootD->level = level;
    addToken(&rootD, tk);
    tk = FSADriver(parseFile);
    node_t* nodeF = F(level + 1);
    if (nodeF == nullptr)
        return nullptr;
    addChild(&rootD, nodeF, "F");
    return rootD;
}

node_t* E(int level) {
    node_t* rootE = new node_t;
    rootE->level = level;
    addToken(&rootE, tk);
    tk = FSADriver(parseFile);
    node_t* nodeF = F(level + 1);
    if (nodeF == nullptr)
        return nullptr;
    addChild(&rootE, nodeF, "F");
    node_t* nodeF2 = F(level + 1);
    if (nodeF2 == nullptr)
        return nullptr;
    addChild(&rootE, nodeF2, "F");
    node_t* nodeF3 = F(level + 1);
    if (nodeF3 == nullptr)
        return nullptr;
    addChild(&rootE, nodeF3, "F");
    node_t* nodeB = B(level + 1);
    if (nodeB == nullptr)
        return nullptr;
    addChild(&rootE, nodeB, "B");
    return rootE;
}

node_t* F(int level) {
    node_t* rootF = new node_t;
    rootF->level = level;
    if (tk.type == TKN_T2) {
        addToken(&rootF, tk);
        tk = FSADriver(parseFile);
        return rootF;
    } else if (tk.type == TKN_T3) {
        addToken(&rootF, tk);
        tk = FSADriver(parseFile);
        return rootF;
    } else if (checkChar((char*)"&")) {
        addToken(&rootF, tk);
        tk = FSADriver(parseFile);
        node_t* nodeF = F(level + 1);
        if (nodeF == nullptr)
            return nullptr;
        addChild(&rootF, nodeF, "F");
        node_t* nodeF2 = F(level + 1);
        if (nodeF2 == nullptr)
            return nullptr;
        addChild(&rootF, nodeF2, "F");
        return rootF;
    } else {
        printError();
        return nullptr;
    }
}

node_t* G(int level) {
    node_t* rootG = new node_t;
    rootG->level = level;
    addToken(&rootG, tk);
    tk = FSADriver(parseFile);
    if (!checkChar((char*)"%")) {
        printError();
        return nullptr;
    }
    addToken(&rootG, tk);
    tk = FSADriver(parseFile);
    node_t* nodeF = F(level + 1);
    if (nodeF == nullptr)
        return nullptr;
    addChild(&rootG, nodeF, "F");
    return rootG;
}

void printError() {
    printf("Parse Error: %s %d\n", tk.instance.c_str(), tk.lineNum);
}

bool checkChar(char* check) {
    // strcmp returns 0 if true, so need to negate for proper behavior
    return !strcmp(tk.instance.c_str(), check);
}
