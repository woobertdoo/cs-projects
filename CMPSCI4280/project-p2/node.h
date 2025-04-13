#ifndef NODE_H
#define NODE_H

#include "token.h"
#include <string>
#include <vector>

/* start OR2 */
struct node_t {
    std::vector<node_t*> children;
    char* label;
    int level;
};

/* end OR2 */

void addToken(node_t**, Token);
void addChild(node_t**, node_t*, const char* label);

#endif
