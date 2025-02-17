#include "buildTree.h"
#include <cstdio>
#include <string.h>

node_t* buildTree(FILE* inFile) {
    node_t* root = nullptr;

    char token[MAX_TOKEN_LENGTH];
    /* start OR3 */
    // fscanf reads in one string at a time, separated by whitespace
    // we can use this same function call for standard input, because stdin is
    // just a FILE*
    while (fscanf(inFile, "%s", token) == 1) {
        int len = strlen(token);

        for (int i = 0; i < len; i++) {
            if (!isValidChar(token[i])) {
                fprintf(
                    stderr,
                    "Unallowed character found. This program only allows "
                    "alphabetical and numerical characters as well as the "
                    "following special characters:\n! \" # $ %% & ` ( ) * + ");
                return root;
            }
        }

        std::string strToken(token);

        addNode(&root, len, strToken);
    }
    /* end OR3 */

    // this will be a nullptr if there is no data in the file
    return root;
}

bool isValidChar(const char c) {
    /* start OR6 */
    if (c >= 65 && c <= 90) { // A-Z ASCII
        return true;
    } else if (c >= 97 && c <= 122) { // a-z ASCII
        return true;
    } else if (c >= 48 && c <= 57) { // 0-9 ASCII
        return true;
    } else if (c >= 33 && c <= 43) { // Allowed special characters
        return true;
    } else {
        return false;
    }
    /* end OR6 */
}
