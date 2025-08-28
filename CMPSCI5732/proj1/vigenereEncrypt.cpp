#include <cctype>
#include <iostream>
#include <string>

const int aAscii = (int)'a';

std::string encryptMessage(std::string msg, std::string key) {
    // String to hold the encryption as pre-process the message
    std::string encryption;

    for (char c : msg) {
        if (std::isalpha(c))
            // Add all alphabetic characters from the message to encryption
            encryption.push_back(std::tolower(c));
    }

    for (int i = 0; i < encryption.length(); i++) {
        // The offset of the current character in the key we're using
        // subtract aAscii to get 0-25
        int curOffset = (int)key[i % key.length()] - aAscii;

        // Subtract aAscii so we have our characters at 0-25, then add the key
        // offset and modulus 26 before adding aAscii back to get proper ascii
        // code
        int cryptChar = (int)encryption[i] - aAscii;
        cryptChar += curOffset;
        cryptChar %= 26;
        cryptChar += aAscii;
        encryption[i] = (char)(cryptChar);
    }

    return encryption;
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
int main() {
    std::string msg, key;

    std::cout << "Please insert your message: ";
    std::getline(std::cin, msg);
    std::cout << "\nPlease insert your key: ";
    std::cin >> key;

    std::string encryption = encryptMessage(msg, key);

    std::cout << "\nEncrypted message: " << encryption << std::endl;
    std::cout << "\nDecrypted message: " << decryptMessage(encryption, key);

    return 0;
}
