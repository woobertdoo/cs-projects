#include "buildTree.h"
#include <cstdio>
#include <string.h>

node_t* buildTree(FILE* inFile) {
    node_t* root = nullptr;

    char* token;
    /* start OR3 */
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

    return root;
}

node_t* buildTreeStdIn() {
    node_t* root = nullptr;

    char* token;
    /* start OR3 */
    while (fscanf(stdin, "%s ", token) == 1) {
        printf("%s", token);
        /* int len = strlen(token);

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

        addNode(&root, len, strToken); */
    }
    /* end OR3 */

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
}
