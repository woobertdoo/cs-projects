#include "node.h"
#include <string.h>

std::string tokenNames[] = {"t1", "t2", "t3"};

void addToken(node_t** parentPtr, Token tk) {
    std::string label = "";
    label.append(tokenNames[(int)tk.type]);
    label.append(" ");
    label.append(tk.instance);
    node_t* child = new node_t;
    addChild(parentPtr, child, label.c_str());
}

void addChild(node_t** parentPtr, node_t* child, const char* label) {
    child->label = (char*)malloc(sizeof(label));
    strcpy(child->label, label);
    child->level = (*parentPtr)->level + 1;
    (*parentPtr)->children.push_back(child);
}
