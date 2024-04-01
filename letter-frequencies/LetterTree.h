#ifndef LETTER_TREE_H
#define LETTER_TREE_H

#include "string"
using namespace std;

class LetterTree {
private:
    struct TreeNode {
        string letters;
        int occurences;
        TreeNode *left;
        TreeNode *right;
    };

    TreeNode *root;

    void insert(TreeNode *&, TreeNode *&);
    void destroySubTree(TreeNode *);
    void display(TreeNode *) const;

public:
    LetterTree() { root = nullptr; }
    ~LetterTree() { destroySubTree(root); }

    void insertNode(string);

    void display() const { display(root); }
};

#endif