#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

/* start OR2 */
struct node_t {
    int value;
    node_t* leftChild = nullptr;
    node_t* rightChild = nullptr;
    std::vector<std::string> keys;
};

/* Adds a new node, to the tree.
 * @param parenPtr: A pointer to the parent node (type node_t*)
 * @param value: The value of the node, used to place it in the tree
 * @param key: A string key to add to the node's list of keys
 * */
void addNode(node_t** parentPtr, int value, std::string key);

/* end OR2 */

#endif
