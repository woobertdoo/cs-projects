#include <cctype>
#include <iostream>
#include <string.h>
#include <vector>

const int aAscii = (int)'a';

bool getContinue() {
    char choice;
    std::cout << "Continue? (y/n)\n";
    std::cin >> choice;
    return std::tolower(choice) == 'y';
}

std::vector<std::string> getKeys(int keyLength) {
    std::string validChars = "abcdefghijklmnopqrstuvwxyz";
    std::vector<std::string> possibleKeys;

    // have list of keys start at aaaa...
    std::vector<int> indices(keyLength, 0);

    while (true) {
        std::string current;
        for (int i : indices) {
            current += validChars[i];
        }
        possibleKeys.push_back(current);

        int pos = keyLength - 1;
        while (pos >= 0) {
            if (++indices[pos] < 26)
                break;
            else {
                indices[pos] = 0;
                pos--;
            }
        }

        if (pos < 0)
            break;
    }

    return possibleKeys;
}
std::string decryptMessage(std::string cipherText, std::string key) {
    // String to hold the encryption as pre-process the message
    std::string message;

    for (int i = 0; i < cipherText.length(); i++) {
        // The offset of the current character in the key we're using
        // subtract aAscii to get 0-25
        int curOffset = (int)key[i % key.length()] - aAscii;

        // Subtract aAscii so we have our characters at 0-25, then add the key
        // offset and modulus 26 before adding aAscii back to get proper ascii
        // code

        /*
         * dogd
         * dhzd
         *
         * aZta
         *
         */

        int cryptChar = (int)cipherText[i] - aAscii;
        cryptChar -= curOffset;

        // negative modulus doesn't wrap around for some reason?
        if (cryptChar < 0)
            cryptChar += 26;

        cryptChar %= 26;
        cryptChar += aAscii;
        message.push_back((char)cryptChar);
    }

    return message;
}

void findKeys(std::string cipherText, std::string confirmedText,
              int maxLength) {
    std::string confirmedLower;
    for (char c : confirmedText) {
        if (std::isalpha(c))
            confirmedLower.push_back(std::tolower(c));
    }
    int curKeyLength = 1;
    while (curKeyLength <= maxLength) {
        for (std::string key : getKeys(curKeyLength)) {
            if (decryptMessage(cipherText, key).find(confirmedLower) !=
                std::string::npos) {
                std::cout << "Possible Key Found: " << key << std::endl;
                if (!getContinue())
                    return;
            }
        }
        std::cout << "All keys of length " << curKeyLength
                  << " have been exhausted.\n";
        if (!getContinue())
            return;
        curKeyLength++;
    }
}

int main() {

    std::string cipherText;
    std::string confirmedText;
    int maxKeyLength;

    std::cout << "Please enter your encrypted text:\n";
    std::getline(std::cin, cipherText);

    std::cout << "\nPlease enter text confirmed to be in message:\n";
    std::getline(std::cin, confirmedText);

    std::cout << "\nPlease enter maximum key length to check:\n";
    std::cin >> maxKeyLength;

    findKeys(cipherText, confirmedText, maxKeyLength);

    return 0;
}
