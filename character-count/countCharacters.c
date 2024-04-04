#include "countCharacters.h"

int countCharacters(const char *content, const char target) {
    int targetCount = 0;
    int contentLength = strlen(content);

    for (int i = 0; i < contentLength; i++) {
        if (content[i] == target) {
            targetCount++;
        }
    }
    return targetCount;
}
