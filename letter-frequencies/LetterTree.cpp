#include "LetterTree.h"
#include <iostream>
using namespace std;

/* Private method for inserting a new node into the tree.
 *   Searches the tree for the proper insertion point, then inserts `newNode` as a leaf in a subtree
 * of `nodePtr`.
 *  If the letters stored in `newNode` are found during the traversal, `newNode` is
 * discarded and the occurence counter for the original node is incremented.
 */
void LetterTree::insert(TreeNode *&nodePtr, TreeNode *&newNode) {
    if (nodePtr == nullptr) {
        nodePtr = newNode; // If there are no nodes in the tree, insert newNode as the root
    } else if (newNode->letters < nodePtr->letters) {
        insert(nodePtr->left, newNode); // If the letters in newNode appear before the current
                                        // node's letters alphabetically, search the left branch
    } else if (newNode->letters > nodePtr->letters) {
        insert(nodePtr->right, newNode); // If the letters in newNode appear after the current
                                         // node's letters alphabetically, search the right branch
    } else {
        nodePtr->occurences++; // If the letter combination in newNode already have a node in the
                               // tree, increment the occurences counter.
    }
}

/* Public method for inserting a new node into a binary tree. */
void LetterTree::insertNode(string letters) {
    TreeNode *newNode = nullptr;

    // Create a new node and store the letter distribution in it
    // with a starting occurence count of 1
    newNode = new TreeNode;
    newNode->letters = letters;
    newNode->occurences = 1;
    newNode->left = newNode->right = nullptr;

    insert(root, newNode);
}

/* Displays the occurences of each letter combination recursively, using an inorder display approach
 */
void LetterTree::display(TreeNode *nodePtr) const {
    if (nodePtr) {
        display(nodePtr->left);
        cout << nodePtr->letters << ": " << nodePtr->occurences << endl;
        display(nodePtr->right);
    }
}

/* Deletes all the nodes from a subtree */
void LetterTree::destroySubTree(TreeNode *nodePtr) {
    if (nodePtr) {
        if (nodePtr->left)
            destroySubTree(nodePtr->left);
        if (nodePtr->right)
            destroySubTree(nodePtr->right);
        delete nodePtr;
    }
}