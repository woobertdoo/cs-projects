#include <iostream>
using namespace std;
class DelimiterStack {
private:
    struct DelimiterNode {
        char character;
        int lineNumber;
        int charCount;
        DelimiterNode *next;
    };
    // Define stack top
    DelimiterNode *top;

public:
    class MismatchedDelimiter {};
    class MissingClosingDelimiter {};
    class MissingOpeningDelimter {};

    // Constructor
    DelimiterStack() { top = nullptr; }
    // Destructor
    ~DelimiterStack();

    // Stack Operations
    void push(char, int, int);
    void popDelimiter(char &, int &, int &); // reference variables because three variables cannot
                                             // be returned through return statement
    bool isEmpty() const;
};

const string OPENING_DELIMITERS = "{[("; // String of acceptable opening delimiters
const string CLOSING_DELIMITERS = "}])"; // String of acceptable closing delimiters

int main() {
    // variable declarations and initializations
    DelimiterStack delimeterStack;
    string currentLine = "";
    char poppedNodeChar; // character of a popped node
    int poppedNodeLine;  // line number of a popped node
    int poppedNodeCol;   // column number of a popped node

    int lineCount = 0; // What line number the user is currently on
    do {
        // What character on that line is currently being processed
        // parse the input line ignore non delimiters
        getline(cin, currentLine);
        lineCount++;

        for (int i = 0; i < currentLine.length(); i++) {
            char currentChar = currentLine[i];
            try {
                // if an opening delimiter, push it to the stack
                if (OPENING_DELIMITERS.find(currentChar) != string::npos) {
                    delimeterStack.push(currentChar, lineCount, i + 1);
                }
                // if a closing delimiter, pop out the last node, which should be the matching
                // opening delimiter
                if (CLOSING_DELIMITERS.find(currentChar) != string::npos) {
                    // If the stack is empty, there are no more opening delimiters to be matched
                    if (delimeterStack.isEmpty()) {
                        throw DelimiterStack::MissingOpeningDelimter();
                    }
                    delimeterStack.popDelimiter(poppedNodeChar, poppedNodeLine, poppedNodeCol);
                    /* If the delimiters do not have the same index in their respective lists,
                     *  then they do not match, and the user should be alerted.*/
                    if (CLOSING_DELIMITERS.find(currentChar) !=
                        OPENING_DELIMITERS.find(poppedNodeChar)) {
                        throw DelimiterStack::MismatchedDelimiter();
                    }
                }

            } catch (DelimiterStack::MismatchedDelimiter) {
                // Let the user know where the mismatched delimiter is
                cout << "Left delimiter " << poppedNodeChar << " at line " << poppedNodeLine
                     << ", column " << poppedNodeCol << " does not match right delimiter "
                     << currentChar << " at line " << lineCount << ", column " << i + 1 << ".\n";
                // Push the opening delimiter back onto the stack so the user can fix their error
                delimeterStack.push(poppedNodeChar, poppedNodeLine, poppedNodeCol);

            } catch (DelimiterStack::MissingOpeningDelimter) {
                cout << "Right delimiter " << currentChar << " at line " << lineCount << ", column "
                     << i + 1 << " has no matching left delimiter.\n";
            }
            // reset the current opening delimiter so it doesn't try to compare unfinished
            // code blocks
        }
    } while (currentLine != "DONE");

    try {
        // If the user has finished inputing, but there are still delimiters on the stack
        // then there are opening delimiters with no matching closing delimiter
        if (!delimeterStack.isEmpty()) {
            throw DelimiterStack::MissingClosingDelimiter();
        }
    } catch (DelimiterStack::MissingClosingDelimiter) {
        while (!delimeterStack.isEmpty()) {
            delimeterStack.popDelimiter(poppedNodeChar, poppedNodeLine, poppedNodeCol);
            cout << "Left delimiter " << poppedNodeChar << " at line " << poppedNodeLine
                 << ", column " << poppedNodeCol << " has no matching closing delimiter.\n";
        }
    }

    return 0;
}

DelimiterStack::~DelimiterStack() {
    DelimiterNode *node = nullptr, *nextNode = nullptr;
    node = top;
    while (node != nullptr) {
        nextNode = node->next;
        delete node;
        node = nextNode;
    }
}

void DelimiterStack::push(char delimiter, int lineNum, int colNum) {
    DelimiterNode *newNode = nullptr;

    newNode = new DelimiterNode;
    newNode->character = delimiter;
    newNode->lineNumber = lineNum;
    newNode->charCount = colNum;

    if (isEmpty()) {
        top = newNode;
        newNode->next = nullptr;
    } else {
        newNode->next = top;
        top = newNode;
    }
}

void DelimiterStack::popDelimiter(char &delimiter, int &lineNum, int &colNum) {
    DelimiterNode *temp = nullptr;

    if (isEmpty()) {
        // If there are no closing delimiters left, set the popped character to a whitespace
        // character for condition checking purposes.
        delimiter = ' ';
    } else {
        delimiter = top->character;
        lineNum = top->lineNumber;
        colNum = top->charCount;
        temp = top->next;
        delete top;
        top = temp;
    }
}

bool DelimiterStack::isEmpty() const {
    if (top == nullptr)
        return true;
    else
        return false;
}