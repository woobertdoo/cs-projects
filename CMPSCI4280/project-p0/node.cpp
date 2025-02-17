#include "node.h"

// takes a pointer to a pointer so that the parent node can be modified in-place
void addNode(node_t** parentPtr, int value, std::string key) {
    if (*parentPtr == nullptr) {
        (*parentPtr) = new node_t;
        (*parentPtr)->value = value;
        (*parentPtr)->keys.push_back(key);
        return;
    }

    if (value == (*parentPtr)->value) {
        (*parentPtr)->keys.push_back(key);
    } else if (value < (*parentPtr)->value) {
        addNode(&(*parentPtr)->leftChild, value, key);
    } else {
        addNode(&(*parentPtr)->rightChild, value, key);
    }
}
