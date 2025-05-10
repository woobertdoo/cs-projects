#include "symbolTable.h"
#include <climits>
std::vector<Symbol> symbols;

bool generateSymbolTable(node_t* root) {
    if (!processTree(root))
        return false;
    printf("Symbols\n");
    for (Symbol sym : symbols) {
        printf("%s\n", sym.name.c_str());
    }
    return true;
}

bool processTree(node_t* root) {
    // If we run into a t1 token, we check the next token to come up to check it
    // against the symbol table
    bool checkNextTK = false;
    // When this is true, we will attempt to add the token to the symbol table
    // When this is false, we will attempt to fetch the symbol from the symbol
    // table
    bool shouldAddSymbol = false;
    for (node_t* child : root->children) {
        Symbol symbol;
        // If the child token's instance is an empty string, it is a
        // non-terminal node. We then process this subtree With the non-terminal
        // as root
        if (child->token.instance == "") {
            if (!processTree(child))
                return false;
            continue;
        }
        // Otherwise, we need to check the tokens
        // Check T1 Instances to know what type of operation we need to perform
        // Only needs to be done if we are not already trying to check a symbol
        if (!checkNextTK && child->token.type == TKN_T1) {
            // Parentheses aren't followed by a token
            if (child->token.instance == "(" || child->token.instance == ")")
                continue;
            checkNextTK = true;
            // " and # are used to add a symbol to the table
            shouldAddSymbol =
                (child->token.instance == "\"" || child->token.instance == "#");
            // Move to the next child node
            continue;
        }
        // Reset check flag
        checkNextTK = false;
        // Only care about T2s for now
        if (child->token.type == TKN_T2) {
            symbol.name = child->token.instance;
            if (shouldAddSymbol) {
                if (!addSymbol(symbol, child->token.lineNum))
                    return false;
                // Reset add flag
                shouldAddSymbol = false;
                continue;
            }

            if (!fetchSymbol(symbol, child->token.lineNum))
                return false;
        }
    }
    return true;
}

bool addSymbol(Symbol symbol, int lineNum) {
    if (checkSymbol(symbol)) {
        fprintf(stderr,
                "Error: Line %d: Attempt to define already defined symbol %s.",
                lineNum, symbol.name.c_str());
        return false;
    }
    symbols.push_back(symbol);
    return true;
}

bool fetchSymbol(Symbol symbol, int lineNum) {
    if (!checkSymbol(symbol)) {
        fprintf(stderr,
                "Error: Line %d: Attempt to access undefined symbol %s.",
                lineNum, symbol.name.c_str());
        return false;
    }
    return true;
}

Symbol* getSymbol(std::string symName) {
    for (int i = 1; i < symbols.size(); i++) {
        if (symbols[i].name == symName)
            return &symbols[i];
    }

    return nullptr;
}

bool checkSymbol(Symbol symbol) {
    for (Symbol sym : symbols) {
        if (sym.name == symbol.name) {
            return true;
        }
    }
    return false;
}

std::vector<Symbol> fetchSymbolTable() { return symbols; }
