#include "LetterTree.h"
#include <fstream>
#include <iostream>
using namespace std;

void parseTextToTree(LetterTree &, ifstream &);

int main() {
    LetterTree tree;
    ifstream textFile;
    parseTextToTree(tree, textFile);

    tree.display();
}

/* Reads from the file opened with `data` and gets every sequence of letters ranging from length `1`
 * to a user-determined maximum. It adds each sequence of letters as nodes to `tree`.
 */
void parseTextToTree(LetterTree &tree, ifstream &data) {
    // Get user input for max length of sequence
    int maxLength;
    cout << "What should be the maximum length of a letter sequence? ";
    cin >> maxLength;

    // Create temporary string and char to hold the data
    string tempstr;

    data.open("letters.txt");
    if (data) {
        string word;     // Current word in the file input stream
        string fileText; // String to store the text of the file

        while (data >> word) {
            fileText += word;
        }

        // Loop through each length of character sequences between 1 and maxLength
        for (int currLength = 0; currLength < maxLength; currLength++) {
            int charPos = 0;
            while (charPos < fileText.length() - currLength) {
                tempstr = "";
                for (int i = 0; i <= currLength; i++) {
                    tempstr += fileText[charPos + i];
                }
                tree.insertNode(tempstr);
                charPos++;
            }
        }

        data.close();
    } else {
        cout << "Error: Failed to read file. Make sure it exists!";
    }
}