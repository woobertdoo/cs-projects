#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "caseConversion.h"
#include "countCharacters.h"

void help() {
    printf("Takes a file, converts contents to uppercase or lowercase "
           "(if no flag is set, converts to uppercase), and then counts "
           "occurences of a character\n");
    printf("-f <input file> (required)\n");
    printf("-o <output file> (defaults to 'output.txt')\n");
    printf("-c <character to count> (defaults to 'c')\n");
    printf("-u - converts file contents to all uppercase\n");
    printf("-l - converts file contents to all lowercase\n");
}

int main(int argc, char **argv) {
    char target = 'c';
    char *inFilename = NULL;
    char *outFilename = "output.txt";
    int toUpper = 1;
    int helpMenu = 0; // Did they look at the help menu?
    int option;
    while ((option = getopt(argc, argv, "hf:c:o:ul")) != -1) {
        switch (option) {
        case 'h':
            helpMenu = 1;
            help();
            break;
        case 'f':
            inFilename = optarg;
            break;
        case 'o':
            outFilename = optarg;
            break;
        case 'c':
            target = *optarg;
            break;
        case 'u':
            toUpper = 1;
            break;
        case 'l':
            toUpper = 0;
            break;
        case '?':
            switch (optopt) {
            case 'f':
                fprintf(stderr, "-f requires input file as argument\n");
                break;
            case 'o':
                fprintf(stderr, "-o requires output file as argument\n");
                break;
            case 'c':
                fprintf(stderr, "-c requires target character as argument\n");
                break;
            default:
                if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option -%c\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character '\\x%x'\n",
                            optopt);
                }
                break;
            }
            return EXIT_FAILURE;
        default:
            help();
            helpMenu = 1;
            break;
        }
    }

    // If the user just wanted to look at the help menu, don't continue on with
    // the program
    if (helpMenu == 1) {
        return EXIT_SUCCESS;
    }
    if (inFilename == NULL) {
        printf("Input file name required. Use the -f option to set one\n");
        return EXIT_FAILURE;
    }

    const char *conversion = toUpper == 1 ? "uppercase" : "lowercase";
    printf("Input file: %s\n", inFilename);
    printf("Output file: %s\n", outFilename);
    printf("Converting to: %s\n", conversion);

    FILE *inFile, *outFile;
    inFile = fopen(inFilename, "r");
    outFile = fopen(outFilename, "w");

    if (inFile == NULL) {
        printf("Error opening file %s. Make sure it exists and has proper "
               "permissions\n",
               inFilename);
        return EXIT_FAILURE;
    }

    if (outFile == NULL) {
        printf("Error opening file %s. Make sure it exists and has proper "
               "permissions\n",
               outFilename);
        fclose(inFile);
        return EXIT_FAILURE;
    }

    fseek(inFile, 0L, SEEK_END);
    int fileLength = ftell(inFile);
    if (fileLength == -1L) {
        printf("Error reading input file.\n");
        return EXIT_FAILURE;
    }
    rewind(inFile);

    char contents[fileLength];
    for (int i = 0; i < fileLength; i++) {
        contents[i] = fgetc(inFile);
    }

    if (toUpper == 1) {
        toUppercase(contents);
    } else {
        toLowercase(contents);
    }

    int targetCount = countCharacters(contents, target);
    fprintf(outFile, "Number of %c's found: %d", target, targetCount);

    fclose(inFile);
    fclose(outFile);

    return EXIT_SUCCESS;
}
