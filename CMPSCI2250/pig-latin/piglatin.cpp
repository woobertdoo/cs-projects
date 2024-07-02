// This program takes in an english sentence and turns it into pig latin
#include <cctype>
#include <iostream>
#include <string>
using namespace std;

// Each word has an english version and a pig latin translation
struct Word {
    string english;
    string pigLatin;
};

Word *splitSentence(const string, int &);
void convertToPigLatin(Word[], int);
void displayPigLatin(const Word[], int);

int main() {

    int wordCount = 0;
    string message = "";
    cout << "Enter a sentence to translate into pig latin:\n";
    getline(cin, message);

    Word *wordsList = splitSentence(message, wordCount);
    convertToPigLatin(wordsList, wordCount);
    displayPigLatin(wordsList, wordCount);

    return 0;
};

/*
 *   Definition of splitSentence
 *   This function takes in a sentence string and reference parameter size, which represents the word count
 */
Word *splitSentence(const string words, int &size) {
    size = 1;

    // Loop through each character in the string
    for (int i = 0; i < words.length(); i++) {
        // If the current character is a space and the previous character wasn't a space
        // Add 1 to the size. This should keep it from counting consecutive spaces as multiple words
        if (i > 0 && isspace(words[i]) && !isspace(words[i - 1]))
            size++;
    }

    // Create a new array of Words with size being the total number of
    // words counted in the previous loop
    Word *wordsArray = new Word[size];
    // Create a temporary string to hold the characters of the words as we loop through
    string tmpstr = "";
    // A counter variable to add words to the array
    int wordCounter = 0;

    // Loop through each character in the string
    for (int i = 0; i < words.length(); i++) {
        // If the current character is a letter, append it to the temporary string in lowercase
        if (isalpha(words[i])) {
            tmpstr += tolower(words[i]);
        } else if (i > 0 && isspace(words[i]) && !isspace(words[i - 1])) { // When a new space is reached (nonconsecutive)
            wordsArray[wordCounter].english = tmpstr;                      // Take whatever is currently stored in the tmpstr
                                                                           // and put it in the next available spot of the words array
            wordCounter++;                                                 // Increment the wordCounter so the next word goes in
                                                                           // the next empty slot of the array
            tmpstr = "";                                                   // Reset the temporary string
        }
    }

    // When we reach the last character in the string, add the remaining value of tmpstr to the array
    wordsArray[size - 1].english = tmpstr;

    return wordsArray;
}

void convertToPigLatin(Word words[], int size) {
    for (int i = 0; i < size; i++) {
        string english = words[i].english;
        string translation = ""; // Temporary variable to store the translation

        // If the english word starts with a vowel, simply append 'way' to the word
        if (english[0] == 'a' || english[0] == 'e' || english[0] == 'i' || english[0] == 'o' || english[0] == 'u') {
            translation = english + "way";
        } else {
            // For words that start with consonants, start by taking the *second* letter through the last letter for the translation
            for (int c = 1; c < english.length(); c++) {
                translation += english[c];
            }
            translation += english[0]; // After that, append the first letter of the english word to the translation
            translation += "ay";       // Then append 'ay' to finish the translation
        }
        words[i].pigLatin = translation;
    }
}

void displayPigLatin(const Word words[], int size) {
    for (int i = 0; i < size; i++) {
        cout << words[i].pigLatin << " ";
    }
    cout << endl;
}